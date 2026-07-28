! Copyright (C) 2023-2026 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
! TS1 + miem NOx box model, real-fixture variant
!
! This is the Fortran counterpart of
! python/musica/examples/miem_nox_box_model_real_fixture.py, and a variant of
! fortran/examples/miem_nox_box_model.F90 driven from the real committed miem
! fixture (configs/miem/x1.163842_2024_nox_subset.nc, unmodified CAMS-GLOB-ANT
! anthropogenic NOx, 12 monthly snapshots x 4097 cells) instead of a synthetic
! single-cell/single-snapshot inventory.
!
! Differences from miem_nox_box_model.F90:
!  - the miem Emissions module is built with n_cells=4097 (must match the
!    real file); one specific cell (1603 in this 1-based indexing, i.e.
!    Python's 0-based 1602 -- the single highest-emitting cell in the
!    fixture, a stand-in for "busy urban" since the fixture carries no
!    lat/lon) is read from its flux output. This is a different dimension
!    space from the single-cell MICM/TUV-x box model itself, which still
!    only ever has one well-mixed grid cell.
!  - the species map does a genuine one-to-many split (nox_anth_sum -> NO
!    0.9 + NO2 0.1, matching emissions_model_end_to_end_nox.cpp and the
!    plain API test in fortran/test/integration/test_emissions_api.F90),
!    instead of a 1:1 NO-only map -- so this drives two EMISSION reactions
!    (NO and NO2), requiring its own spliced ts1.json copy.
!  - temporal interpolation is "linear" across the real monthly brackets
!    (rather than "none" against a single static snapshot), so this
!    actually exercises OfflineEmissionSource's bracket/interpolation path.
!  - sim date is fixed to 2024-07-15 (mid-July 2024), the only calendar
!    year this fixture has data for -- LoadBrackets hard-errors on any
!    timestamp outside [2024-01-01, 2024-12-01].
!
program miem_nox_box_model_real_fixture
  use iso_fortran_env, only: real64
  use musica_util, only: dk => musica_dk, error_t, string_t, mappings_t, assert
  use musica_string, only: config_string_t => string_t
  use musica_config, only: config_t
  use musica_tuvx, only: tuvx_t
  use musica_micm, only: micm_t, solver_stats_t, RosenbrockStandardOrder
  use musica_state, only: state_t
  use musica_emissions, only: mechanism_t, emissions_t
  use tuvx_ts1_setup, only: get_tuvx_ts1_calculator, config_file_path

  implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_GT( a, b ) call assert( a > b, __FILE__, __LINE__ )
#define ASSERT_LT( a, b ) call assert( a < b, __FILE__, __LINE__ )

  ! MICM/TUV-x still operate on a single well-mixed box, matching the
  ! Python example's grid_cell_index=1 (skip the ground cell, use the 1 km
  ! edge -- same convention as miem_nox_box_model.F90/ts1_box_model.F90).
  integer, parameter :: NUM_MICM_CELLS = 1
  integer, parameter :: FIRST_LAYER = 2   ! TUV-x layer index for 1 km edge
  integer, parameter :: NUM_LAYERS = 121  ! 120-section height grid -> 121 edges

  ! miem's emissions grid is the real fixture's own dimension (must match
  ! the file); EMISSIONS_CELL_INDEX (1-based) is Python's 0-based 1602, the
  ! single highest-emitting cell across all 12 months in that fixture.
  integer, parameter :: NUM_EMISSIONS_CELLS = 4097
  integer, parameter :: EMISSIONS_CELL_INDEX = 1603

  real(dk), parameter :: SZA = 0.0_dk               ! radians (overhead sun)
  real(dk), parameter :: EARTH_SUN_DISTANCE = 1.0_dk ! AU
  real(dk), parameter :: TIME_STEP = 30.0_dk         ! seconds
  real(dk), parameter :: SIM_LENGTH = 3.0_dk * 3600.0_dk  ! 3 hours

  ! 2024-07-15 12:30:00 UTC (06:30 MDT, one hour before 07:30 local, matching
  ! the Python example) -- the only calendar year this fixture covers.
  real(dk), parameter :: EPOCH_START = 1721046600.0_dk

  real(dk), parameter :: NO_MOLECULAR_WEIGHT = 0.030006_dk    ! kg mol-1
  real(dk), parameter :: NO2_MOLECULAR_WEIGHT = 0.0460055_dk  ! kg mol-1
  real(dk), parameter :: BOX_HEIGHT_M = 100.0_dk

  ! US Standard Atmosphere 1976 troposphere constants (valid 0-11 km)
  real(dk), parameter :: T0 = 288.15_dk              ! K
  real(dk), parameter :: P0 = 101325.0_dk            ! Pa
  real(dk), parameter :: LAPSE = 6.5_dk              ! K km-1
  real(dk), parameter :: USSA_EXP = 5.25588_dk       ! g*M/(R*L)
  real(dk), parameter :: R_GAS = 8.31446261815324_dk ! J mol-1 K-1

  type(tuvx_t), pointer :: tuvx
  type(micm_t), pointer :: micm
  type(state_t), pointer :: state
  type(mechanism_t), pointer :: emissions_mechanism
  type(emissions_t), pointer :: emissions
  type(error_t) :: error
  type(string_t) :: solver_state
  type(solver_stats_t) :: solver_stats
  type(mappings_t), pointer :: photo_mappings, heat_mappings

  integer :: num_photo, num_heat
  real(dk), allocatable :: photo_rates(:,:), heating_rates(:,:)

  real(dk) :: z_km, temperature, pressure, current_time, elapsed, remaining
  real(dk) :: epoch_time, no_surface_flux, no2_surface_flux
  real(dk) :: no_emis_rate, no2_emis_rate
  real(dk) :: initial_o3, final_o3, initial_no2, final_no2
  real(dk) :: last_pct, pct
  integer :: csv_unit
  integer :: idx_no, idx_no2, idx_o3

  write(*,*) "======================================"
  write(*,*) "TS1 + miem NOx box model (real fixture)"
  write(*,*) "======================================"
  write(*,*)

  ! --- Set up TUV-x for the TS1/TSMLT photolysis configuration ---
  write(*,*) "Setting up TUV-x (TS1/TSMLT)..."
  tuvx => get_tuvx_ts1_calculator(error)
  call check(error, "creating TUV-x")

  photo_mappings => tuvx%get_photolysis_rate_constants_ordering(error)
  call check(error, "getting photolysis ordering")
  num_photo = photo_mappings%size()

  heat_mappings => tuvx%get_heating_rates_ordering(error)
  call check(error, "getting heating rate ordering")
  num_heat = heat_mappings%size()

  allocate(photo_rates(NUM_LAYERS, num_photo))
  allocate(heating_rates(NUM_LAYERS, num_heat))
  photo_rates = 0.0_dk
  heating_rates = 0.0_dk

  write(*,'(A,I0,A)') " Running TUV-x for ", num_photo, " photolysis reactions..."
  call tuvx%run(SZA, EARTH_SUN_DISTANCE, photo_rates, heating_rates, error)
  call check(error, "running TUV-x")

  ! --- Set up MICM for the TS1 mechanism plus spliced-in NO/NO2 EMISSION reactions ---
  write(*,*) "Setting up MICM (TS1 + EMIS.NO + EMIS.NO2)..."
  micm => micm_t("../v1/ts1/ts1_with_nox_emission.json", RosenbrockStandardOrder, error)
  call check(error, "creating MICM")

  state => micm%get_state(NUM_MICM_CELLS, error)
  call check(error, "creating state")

  ! --- Apply initial conditions from the CSV file ---
  write(*,*) "Applying initial conditions..."
  call apply_initial_conditions(state, "../v1/ts1/initial_conditions.csv")

  ! --- Set environmental conditions from US Standard Atmosphere 1976 (1 km) ---
  z_km = real(FIRST_LAYER - 1, dk)
  temperature = T0 - LAPSE * z_km
  pressure = P0 * (temperature / T0) ** USSA_EXP
  state%conditions(1)%temperature = temperature
  state%conditions(1)%pressure = pressure
  state%conditions(1)%air_density = pressure / (R_GAS * temperature)

  ! --- Map TUV-x photolysis rates onto the MICM state ---
  write(*,*) "Mapping TUV-x photolysis rates (CAM aliasing)..."
  call apply_photolysis_aliasing(state, photo_mappings, photo_rates)

  ! --- Set up miem against the real fixture, split 0.9 NO / 0.1 NO2 ---
  write(*,*) "Setting up miem (real fixture, cell ", EMISSIONS_CELL_INDEX, ")..."
  emissions_mechanism => mechanism_t("../miem/nox_real_fixture_emissions_config.yaml", error)
  call check(error, "reading miem mechanism")
  emissions => emissions_t(emissions_mechanism, NUM_EMISSIONS_CELLS, 1, error)
  call check(error, "creating emissions model")
  ASSERT_GT( emissions%number_of_species, 1 )

  ! --- Species of interest for output ---
  idx_no  = species_index(state, "NO")
  idx_no2 = species_index(state, "NO2")
  idx_o3  = species_index(state, "O3")
  ASSERT_GT( idx_no, 0 )
  ASSERT_GT( idx_no2, 0 )
  ASSERT_GT( idx_o3, 0 )

  initial_o3 = conc(state, 1, idx_o3, state%species_strides%grid_cell, state%species_strides%variable)
  initial_no2 = conc(state, 1, idx_no2, state%species_strides%grid_cell, state%species_strides%variable)

  ! --- Open CSV output ---
  open(newunit=csv_unit, file='miem_nox_box_model_real_fixture_fortran.csv', &
      status='replace', action='write')
  write(csv_unit,'(A)') 'time_s,NO,NO2,O3,no_surface_flux,no2_surface_flux'
  call write_output(csv_unit, state, 0.0_dk, idx_no, idx_no2, idx_o3, 0.0_dk, 0.0_dk)

  ! --- Time loop (box model) ---
  write(*,'(A,F6.0,A,F4.0,A)') " Integrating for ", SIM_LENGTH, &
      " s in ", TIME_STEP, " s steps..."
  write(*,*)

  current_time = 0.0_dk
  epoch_time = EPOCH_START
  last_pct = -5.0_dk
  do while (current_time < SIM_LENGTH)
    call emissions%run(epoch_time, TIME_STEP, error)
    call check(error, "miem run")
    no_surface_flux  = emissions%flux(EMISSIONS_CELL_INDEX, "NO", error)
    call check(error, "reading NO flux")
    no2_surface_flux = emissions%flux(EMISSIONS_CELL_INDEX, "NO2", error)
    call check(error, "reading NO2 flux")

    no_emis_rate  = no_surface_flux  / (BOX_HEIGHT_M * NO_MOLECULAR_WEIGHT)   ! mol m-3 s-1
    no2_emis_rate = no2_surface_flux / (BOX_HEIGHT_M * NO2_MOLECULAR_WEIGHT)  ! mol m-3 s-1
    call set_rate_parameter(state, "EMIS.NO", no_emis_rate)
    call set_rate_parameter(state, "EMIS.NO2", no2_emis_rate)

    elapsed = 0.0_dk
    do while (elapsed < TIME_STEP)
      remaining = TIME_STEP - elapsed
      call micm%solve(remaining, state, solver_state, solver_stats, error)
      call check(error, "MICM solve")
      elapsed = elapsed + real(solver_stats%final_time(), dk)
      current_time = current_time + real(solver_stats%final_time(), dk)
      if (solver_state%get_char_array() /= "Converged") then
        write(*,*) "  Warning: solver ", solver_state%get_char_array(), &
            " at t = ", current_time
      end if
    end do
    epoch_time = EPOCH_START + current_time

    call write_output(csv_unit, state, current_time, idx_no, idx_no2, idx_o3, &
        no_surface_flux, no2_surface_flux)

    pct = current_time / SIM_LENGTH * 100.0_dk
    if (pct - last_pct >= 5.0_dk) then
      last_pct = pct
      write(*,'(A,F6.1,A)') " Simulation progress: ", pct, "%"
    end if
  end do

  close(csv_unit)
  write(*,*)
  write(*,*) "Results written to miem_nox_box_model_real_fixture_fortran.csv"

  ! --- Check the expected NOx photochemistry signature ---
  final_o3 = conc(state, 1, idx_o3, state%species_strides%grid_cell, state%species_strides%variable)
  final_no2 = conc(state, 1, idx_no2, state%species_strides%grid_cell, state%species_strides%variable)
  write(*,*) "Initial O3: ", initial_o3, " Final O3: ", final_o3
  write(*,*) "Initial NO2: ", initial_no2, " Final NO2: ", final_no2
  ASSERT_LT( final_o3, initial_o3 )    ! O3 is net titrated by NO + O3 -> NO2 + O2
  ASSERT_GT( final_no2, initial_no2 )  ! NO2 grows from the NO/NO2 emission source
  ASSERT_GT( no_surface_flux, 0.0_dk )  ! miem produced a real, nonzero NO flux
  ASSERT_GT( no2_surface_flux, 0.0_dk ) ! miem produced a real, nonzero NO2 flux
  ! The species map splits one inventory quantity 0.9/0.1, so the two
  ! fluxes must retain that exact 9:1 ratio (same check as
  ! emissions_model_end_to_end_nox.cpp and test_emissions_api.F90).
  ASSERT( abs(no_surface_flux - 9.0_dk * no2_surface_flux) < 1.0e-12_dk )

  ! --- Clean up ---
  deallocate(photo_rates, heating_rates)
  deallocate(photo_mappings)
  deallocate(heat_mappings)
  deallocate(emissions)
  deallocate(emissions_mechanism)
  deallocate(state)
  deallocate(micm)
  deallocate(tuvx)

  write(*,*) "Done."

contains

  !> Stop with a message if an error was reported
  subroutine check(error, context)
    type(error_t), intent(in) :: error
    character(len=*), intent(in) :: context
    if (.not. error%is_success()) then
      write(*,*) "Error while ", trim(context), ": ", error%message()
      stop 1
    end if
  end subroutine check

  !> Return the 1-based species index, or -1 if the species is not present
  function species_index(state, name) result(idx)
    type(state_t), intent(in) :: state
    character(len=*), intent(in) :: name
    integer :: idx
    type(error_t) :: local_error
    idx = state%species_ordering%index(name, local_error)
    if (.not. local_error%is_success()) idx = -1
  end function species_index

  !> Write the concentrations of the tracked species and the miem fluxes
  subroutine write_output(unit, state, time_s, ino, ino2, io3, no_flux, no2_flux)
    integer, intent(in) :: unit
    type(state_t), intent(in) :: state
    real(dk), intent(in) :: time_s, no_flux, no2_flux
    integer, intent(in) :: ino, ino2, io3
    integer :: cs, vs
    cs = state%species_strides%grid_cell
    vs = state%species_strides%variable
    write(unit,'(F10.1,3(",",ES16.8E3),2(",",ES16.8E3))') time_s, &
        conc(state, 1, ino, cs, vs), conc(state, 1, ino2, cs, vs), &
        conc(state, 1, io3, cs, vs), no_flux, no2_flux
  end subroutine write_output

  !> Look up a concentration for cell/species, returning 0 for absent species
  function conc(state, cell, idx, cs, vs) result(value)
    type(state_t), intent(in) :: state
    integer, intent(in) :: cell, idx, cs, vs
    real(dk) :: value
    if (idx < 1) then
      value = 0.0_dk
    else
      value = state%concentrations(1 + (cell - 1) * cs + (idx - 1) * vs)
    end if
  end function conc

  !> Read the initial conditions CSV and apply concentrations and rate
  !> parameters. Rows are keyed by a prefix:
  !>   CONC.<species>            -> initial concentration (mol m-3)
  !>   USER.<name> / PHOTO.<name>-> user-defined / photolysis rate parameter
  !>   SURF.<name>               -> effective radius (col 1) and particle
  !>                                number concentration (col 2)
  !>   ENV.*                     -> ignored (set from US Std Atmosphere above)
  subroutine apply_initial_conditions(state, filepath)
    type(state_t), intent(inout) :: state
    character(len=*), intent(in) :: filepath

    integer :: unit_num, ios
    character(len=256) :: line
    character(len=256) :: key
    real(dk) :: value1, value2
    integer :: nfields

    open(newunit=unit_num, file=filepath, status='old', action='read', &
        iostat=ios)
    if (ios /= 0) then
      write(*,*) "ERROR: Could not open ", trim(filepath)
      stop 1
    end if

    do
      read(unit_num, '(A)', iostat=ios) line
      if (ios /= 0) exit
      line = adjustl(line)
      if (len_trim(line) == 0 .or. line(1:1) == '#') cycle

      call parse_csv_row(line, key, value1, value2, nfields)
      if (nfields < 2) cycle

      if (starts_with(key, "CONC.")) then
        call set_concentration(state, trim(key(6:)), value1)
      else if (starts_with(key, "USER.") .or. starts_with(key, "PHOTO.")) then
        call set_rate_parameter(state, trim(key), value1)
      else if (starts_with(key, "SURF.")) then
        call set_rate_parameter(state, &
            trim(key) // ".effective radius [m]", value1)
        call set_rate_parameter(state, &
            trim(key) // ".particle number concentration [# m-3]", value2)
      end if
      ! ENV.* rows are handled separately via the US Standard Atmosphere
    end do

    close(unit_num)
  end subroutine apply_initial_conditions

  !> Set a species concentration for every grid cell (no-op if absent)
  subroutine set_concentration(state, name, value)
    type(state_t), intent(inout) :: state
    character(len=*), intent(in) :: name
    real(dk), intent(in) :: value
    type(error_t) :: local_error
    integer :: idx, i, cs, vs
    idx = state%species_ordering%index(name, local_error)
    if (.not. local_error%is_success()) then
      write(*,*) "  Warning: species not found: ", trim(name)
      return
    end if
    cs = state%species_strides%grid_cell
    vs = state%species_strides%variable
    do i = 1, NUM_MICM_CELLS
      state%concentrations(1 + (i - 1) * cs + (idx - 1) * vs) = value
    end do
  end subroutine set_concentration

  !> Set a rate parameter for every grid cell (no-op if absent)
  subroutine set_rate_parameter(state, name, value)
    type(state_t), intent(inout) :: state
    character(len=*), intent(in) :: name
    real(dk), intent(in) :: value
    type(error_t) :: local_error
    integer :: idx, i, cs, vs
    idx = state%rate_parameters_ordering%index(name, local_error)
    if (.not. local_error%is_success()) then
      write(*,*) "  Warning: rate parameter not found: ", trim(name)
      return
    end if
    cs = state%rate_parameters_strides%grid_cell
    vs = state%rate_parameters_strides%variable
    do i = 1, NUM_MICM_CELLS
      state%rate_parameters(1 + (i - 1) * cs + (idx - 1) * vs) = value
    end do
  end subroutine set_rate_parameter

  !> Map TUV-x photolysis rates onto MICM PHOTO.* rate parameters using the
  !> "__CAM options -> aliasing -> pairs" section of the TUV-x config file.
  subroutine apply_photolysis_aliasing(state, photo_mappings, photo_rates)
    type(state_t), intent(inout) :: state
    type(mappings_t), intent(in) :: photo_mappings
    real(dk), intent(in) :: photo_rates(:,:)

    type(config_t) :: config, cam_options, aliasing
    type(config_t), allocatable :: pairs(:)
    type(config_string_t) :: to_name, from_name
    real(dk) :: scale
    type(error_t) :: local_error
    integer :: n, p, tuvx_col, micm_idx, vs
    integer :: n_mapped

    call config%from_file(config_file_path())
    call config%get("__CAM options", cam_options, "miem_nox_box_model_real_fixture")
    call cam_options%get("aliasing", aliasing, "miem_nox_box_model_real_fixture")
    call aliasing%get("pairs", pairs, "miem_nox_box_model_real_fixture")

    vs = state%rate_parameters_strides%variable
    n = size(pairs)
    n_mapped = 0

    do p = 1, n
      call pairs(p)%get("to", to_name, "miem_nox_box_model_real_fixture")
      call pairs(p)%get("from", from_name, "miem_nox_box_model_real_fixture")
      call pairs(p)%get("scale by", scale, "miem_nox_box_model_real_fixture", default=1.0_dk)

      tuvx_col = photo_mappings%index(trim(from_name%val_), local_error)
      if (.not. local_error%is_success()) cycle

      micm_idx = state%rate_parameters_ordering%index( &
          "PHOTO." // trim(to_name%val_), local_error)
      if (.not. local_error%is_success()) cycle

      state%rate_parameters(1 + (micm_idx - 1) * vs) = &
          photo_rates(FIRST_LAYER, tuvx_col) * scale
      n_mapped = n_mapped + 1
    end do

    write(*,'(A,I0,A,I0,A)') "  Mapped ", n_mapped, " of ", n, &
        " photolysis reactions from TUV-x"
  end subroutine apply_photolysis_aliasing

  !> Parse a "key,value1[,value2]" CSV row
  subroutine parse_csv_row(line, key, value1, value2, nfields)
    character(len=*), intent(in) :: line
    character(len=*), intent(out) :: key
    real(dk), intent(out) :: value1, value2
    integer, intent(out) :: nfields

    integer :: c1, c2, ios
    character(len=64) :: f2, f3

    value1 = 0.0_dk
    value2 = 0.0_dk
    nfields = 0

    c1 = index(line, ',')
    if (c1 == 0) then
      key = adjustl(line)
      nfields = 1
      return
    end if
    key = adjustl(line(1:c1 - 1))
    nfields = 1

    c2 = index(line(c1 + 1:), ',')
    if (c2 == 0) then
      f2 = adjustl(line(c1 + 1:))
      read(f2, *, iostat=ios) value1
      if (ios == 0) nfields = 2
    else
      f2 = adjustl(line(c1 + 1:c1 + c2 - 1))
      f3 = adjustl(line(c1 + c2 + 1:))
      read(f2, *, iostat=ios) value1
      if (ios == 0) nfields = 2
      read(f3, *, iostat=ios) value2
      if (ios == 0) nfields = 3
    end if
  end subroutine parse_csv_row

  !> True if `string` begins with `prefix`
  logical function starts_with(string, prefix)
    character(len=*), intent(in) :: string, prefix
    starts_with = .false.
    if (len_trim(string) >= len(prefix)) then
      starts_with = (string(1:len(prefix)) == prefix)
    end if
  end function starts_with

end program miem_nox_box_model_real_fixture
