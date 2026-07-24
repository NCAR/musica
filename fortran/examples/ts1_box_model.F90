! Copyright (C) 2023-2026 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
! TS1 box model driven by TUV-x photolysis rates
!
! This is the Fortran counterpart of python/musica/examples/ts1_box_model.py.
! It runs the TS1 (MZ327_TS1.2) gas-phase chemistry mechanism in a vertical
! column of independent box-model cells (1-9 km), with photolysis rate
! constants supplied by TUV-x using the TS1/TSMLT configuration.
!
! TUV-x reports rates for a subset of photolysis reactions; the remaining
! reactions are mapped from those using the "__CAM options -> aliasing ->
! pairs" section of the TUV-x configuration file (each pair names a target
! MICM reaction, a source TUV-x reaction, and an optional scale factor). This
! mirrors exactly what the Python example does.
!
program ts1_box_model
  use iso_fortran_env, only: real64
  use musica_util, only: dk => musica_dk, error_t, string_t, mappings_t
  use musica_string, only: config_string_t => string_t
  use musica_config, only: config_t
  use musica_tuvx, only: tuvx_t
  use musica_micm, only: micm_t, solver_stats_t, RosenbrockStandardOrder
  use musica_state, only: state_t
  use tuvx_ts1_setup, only: get_tuvx_ts1_calculator, config_file_path

  implicit none

  ! Vertical column: skip the ground cell (index 0 in TUV-x) and use the
  ! 1-9 km edge rates, matching the Python example (start=1, end=10).
  integer, parameter :: NUM_CELLS = 9
  integer, parameter :: FIRST_LAYER = 2   ! TUV-x layer index for 1 km edge
  integer, parameter :: NUM_LAYERS = 121  ! 120-section height grid -> 121 edges

  real(dk), parameter :: SZA = 0.0_dk               ! radians (overhead sun)
  real(dk), parameter :: EARTH_SUN_DISTANCE = 1.0_dk ! AU
  real(dk), parameter :: TIME_STEP = 30.0_dk         ! seconds
  real(dk), parameter :: SIM_LENGTH = 0.1_dk * 24.0_dk * 60.0_dk * 60.0_dk

  ! US Standard Atmosphere 1976 troposphere constants (valid 0-11 km)
  real(dk), parameter :: T0 = 288.15_dk              ! K
  real(dk), parameter :: P0 = 101325.0_dk            ! Pa
  real(dk), parameter :: LAPSE = 6.5_dk              ! K km-1
  real(dk), parameter :: USSA_EXP = 5.25588_dk       ! g*M/(R*L)
  real(dk), parameter :: R_GAS = 8.31446261815324_dk ! J mol-1 K-1

  type(tuvx_t), pointer :: tuvx
  type(micm_t), pointer :: micm
  type(state_t), pointer :: state
  type(error_t) :: error
  type(string_t) :: solver_state
  type(solver_stats_t) :: solver_stats
  type(mappings_t), pointer :: photo_mappings, heat_mappings

  integer :: num_photo, num_heat, i
  real(dk), allocatable :: photo_rates(:,:), heating_rates(:,:)

  real(dk) :: z_km, temperature, pressure, current_time, elapsed, remaining
  real(dk) :: last_pct, pct
  integer :: csv_unit
  integer :: idx_bepomuc, idx_c6h5ooh, idx_br, idx_cl, idx_o3

  write(*,*) "======================================"
  write(*,*) "TS1 box model driven by TUV-x"
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

  ! TUV-x reports rates on the height-grid edges. The height grid has 120
  ! sections (0-120 km), giving 121 edges; the run fills the full column.
  allocate(photo_rates(NUM_LAYERS, num_photo))
  allocate(heating_rates(NUM_LAYERS, num_heat))
  photo_rates = 0.0_dk
  heating_rates = 0.0_dk

  write(*,'(A,I0,A)') " Running TUV-x for ", num_photo, " photolysis reactions..."
  call tuvx%run(SZA, EARTH_SUN_DISTANCE, photo_rates, heating_rates, error)
  call check(error, "running TUV-x")

  ! --- Set up MICM for the TS1 mechanism ---
  write(*,*) "Setting up MICM (TS1)..."
  micm => micm_t("../v1/ts1/ts1.json", RosenbrockStandardOrder, error)
  call check(error, "creating MICM")

  state => micm%get_state(NUM_CELLS, error)
  call check(error, "creating state")

  ! --- Apply initial conditions from the CSV file ---
  write(*,*) "Applying initial conditions..."
  call apply_initial_conditions(state, "../v1/ts1/initial_conditions.csv")

  ! --- Set environmental conditions from US Standard Atmosphere 1976 ---
  do i = 1, NUM_CELLS
    z_km = real(i, dk)                       ! 1 km ... 9 km
    temperature = T0 - LAPSE * z_km
    pressure = P0 * (temperature / T0) ** USSA_EXP
    state%conditions(i)%temperature = temperature
    state%conditions(i)%pressure = pressure
    state%conditions(i)%air_density = pressure / (R_GAS * temperature)
  end do

  ! --- Map TUV-x photolysis rates onto the MICM state ---
  write(*,*) "Mapping TUV-x photolysis rates (CAM aliasing)..."
  call apply_photolysis_aliasing(state, photo_mappings, photo_rates)

  ! --- Species of interest for output ---
  idx_bepomuc = species_index(state, "BEPOMUC")
  idx_c6h5ooh = species_index(state, "C6H5OOH")
  idx_br      = species_index(state, "BR")
  idx_cl      = species_index(state, "CL")
  idx_o3      = species_index(state, "O3")

  ! --- Open CSV output ---
  open(newunit=csv_unit, file='ts1_box_model_fortran.csv', &
      status='replace', action='write')
  write(csv_unit,'(A)') 'time_s,grid_cell,BEPOMUC,C6H5OOH,BR,CL,O3'
  call write_output(csv_unit, state, 0.0_dk, &
      idx_bepomuc, idx_c6h5ooh, idx_br, idx_cl, idx_o3)

  ! --- Time loop (box model) ---
  write(*,'(A,F6.0,A,F4.0,A)') " Integrating for ", SIM_LENGTH, &
      " s in ", TIME_STEP, " s steps..."
  write(*,*)

  current_time = 0.0_dk
  last_pct = -5.0_dk
  do while (current_time < SIM_LENGTH)
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

    call write_output(csv_unit, state, current_time, &
        idx_bepomuc, idx_c6h5ooh, idx_br, idx_cl, idx_o3)

    pct = current_time / SIM_LENGTH * 100.0_dk
    if (pct - last_pct >= 5.0_dk) then
      last_pct = pct
      write(*,'(A,F6.1,A)') " Simulation progress: ", pct, "%"
    end if
  end do

  close(csv_unit)
  write(*,*)
  write(*,*) "Results written to ts1_box_model_fortran.csv"

  ! --- Clean up ---
  deallocate(photo_rates, heating_rates)
  deallocate(photo_mappings)
  deallocate(heat_mappings)
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

  !> Write the concentrations of the tracked species for every cell
  subroutine write_output(unit, state, time_s, ib, ic, ibr, icl, io3)
    integer, intent(in) :: unit
    type(state_t), intent(in) :: state
    real(dk), intent(in) :: time_s
    integer, intent(in) :: ib, ic, ibr, icl, io3
    integer :: i, cs, vs
    cs = state%species_strides%grid_cell
    vs = state%species_strides%variable
    do i = 1, NUM_CELLS
      write(unit,'(F10.1,",",I0,5(",",ES16.8E3))') time_s, i, &
          conc(state, i, ib, cs, vs), conc(state, i, ic, cs, vs), &
          conc(state, i, ibr, cs, vs), conc(state, i, icl, cs, vs), &
          conc(state, i, io3, cs, vs)
    end do
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
  !> parameters to every grid cell. Rows are keyed by a prefix:
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
    do i = 1, NUM_CELLS
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
    do i = 1, NUM_CELLS
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
    integer :: n, p, tuvx_col, micm_idx, i, cs, vs
    integer :: n_mapped

    call config%from_file(config_file_path())
    call config%get("__CAM options", cam_options, "ts1_box_model")
    call cam_options%get("aliasing", aliasing, "ts1_box_model")
    call aliasing%get("pairs", pairs, "ts1_box_model")

    cs = state%rate_parameters_strides%grid_cell
    vs = state%rate_parameters_strides%variable
    n = size(pairs)
    n_mapped = 0

    do p = 1, n
      call pairs(p)%get("to", to_name, "ts1_box_model")
      call pairs(p)%get("from", from_name, "ts1_box_model")
      call pairs(p)%get("scale by", scale, "ts1_box_model", default=1.0_dk)

      ! TUV-x output column for the source reaction
      tuvx_col = photo_mappings%index(trim(from_name%val_), local_error)
      if (.not. local_error%is_success()) cycle

      ! MICM rate parameter for the target reaction
      micm_idx = state%rate_parameters_ordering%index( &
          "PHOTO." // trim(to_name%val_), local_error)
      if (.not. local_error%is_success()) cycle

      do i = 1, NUM_CELLS
        state%rate_parameters(1 + (i - 1) * cs + (micm_idx - 1) * vs) = &
            photo_rates(FIRST_LAYER + (i - 1), tuvx_col) * scale
      end do
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

end program ts1_box_model
