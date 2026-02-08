! Copyright (C) 2023-2026 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
! Coupled TUV-x + MICM 1D Column Model
!
! Runs Chapman stratospheric O3 photochemistry in a 120-cell vertical column
! (0-120 km, 1 km resolution) with diurnal photolysis rate updates from
! TUV-x v5.4.
!
! The model simulates a 24-hour period at Boulder, CO on the summer solstice.
! Each grid cell is an independent box (no transport). TUV-x computes
! photolysis rate constants as a function of solar zenith angle, and MICM
! integrates the Chapman mechanism chemistry.
!
program column_model
  use iso_fortran_env, only: real64
  use musica_util, only: dk => musica_dk, error_t, string_t, mappings_t
  use musica_tuvx, only: tuvx_t
  use musica_micm, only: micm_t, solver_stats_t, RosenbrockStandardOrder
  use musica_state, only: state_t
  use tuvx_v54_setup, only: get_tuvx_calculator
  use column_model_setup

  implicit none

  integer, parameter :: DAY_OF_YEAR = 172  ! June 21
  real(dk), parameter :: DT_PHOTO = 900.0_dk  ! 15 minutes in seconds
  real(dk), parameter :: SIM_LENGTH = 86400.0_dk  ! 24 hours in seconds
  real(dk), parameter :: START_UTC = 6.0_dk  ! midnight MDT = 06:00 UTC

  type(tuvx_t), pointer :: tuvx
  type(micm_t), pointer :: micm
  type(state_t), pointer :: state
  type(error_t) :: error
  type(string_t) :: solver_state
  type(solver_stats_t) :: solver_stats
  type(mappings_t), pointer :: photo_mappings

  ! Indices
  integer :: jo2_tuvx_idx, jo3a_tuvx_idx, jo3b_tuvx_idx
  integer :: jo2_micm_idx, jo3a_micm_idx, jo3b_micm_idx
  integer :: O2_idx, O_idx, O1D_idx, O3_idx, N2_idx
  integer :: num_photo, num_heat
  integer :: cell_stride, var_stride

  ! Arrays
  real(dk) :: height_mid(NUM_CELLS)
  real(dk) :: temperature(NUM_CELLS), pressure(NUM_CELLS)
  real(dk) :: o3_init(NUM_CELLS), o2_init(NUM_CELLS), n2_init(NUM_CELLS)
  real(dk), allocatable :: photo_rates(:,:), heating_rates(:,:)
  real(dk) :: jo3a_vals(NUM_CELLS), jo3b_vals(NUM_CELLS), jo2_vals(NUM_CELLS)

  ! Time stepping
  integer :: n_steps, step, i
  real(dk) :: hour_utc, sza, local_hr, current_sec
  real(dk) :: elapsed, remaining, pct
  logical :: is_day

  ! CSV output
  integer :: csv_unit

  write(*,*) "======================================"
  write(*,*) "Coupled TUV-x + MICM Column Model"
  write(*,*) "======================================"
  write(*,*)

  ! --- Set up TUV-x v5.4 ---
  write(*,*) "Setting up TUV-x v5.4..."
  tuvx => get_tuvx_calculator(error)
  if (.not. error%is_success()) then
    write(*,*) "Error creating TUV-x: ", error%message()
    stop 1
  end if

  ! Get TUV-x reaction indices
  photo_mappings => tuvx%get_photolysis_rate_constants_ordering(error)
  if (.not. error%is_success()) stop 1
  num_photo = photo_mappings%size()

  jo2_tuvx_idx = photo_mappings%index("O2+hv->O+O", error)
  if (.not. error%is_success()) then
    write(*,*) "Error finding O2+hv->O+O: ", error%message()
    stop 1
  end if
  jo3a_tuvx_idx = photo_mappings%index("O3+hv->O2+O(3P)", error)
  if (.not. error%is_success()) stop 1
  jo3b_tuvx_idx = photo_mappings%index("O3+hv->O2+O(1D)", error)
  if (.not. error%is_success()) stop 1

  ! Get heating rate count for allocation
  block
    type(mappings_t), pointer :: heat_mappings
    heat_mappings => tuvx%get_heating_rates_ordering(error)
    if (.not. error%is_success()) stop 1
    num_heat = heat_mappings%size()
    deallocate(heat_mappings)
  end block

  ! Allocate TUV-x output arrays
  allocate(photo_rates(NUM_CELLS + 1, num_photo))
  allocate(heating_rates(NUM_CELLS + 1, num_heat))

  ! Get initial atmospheric profiles
  write(*,*) "Extracting atmospheric profiles..."
  call get_initial_profiles(tuvx, height_mid, temperature, pressure, &
                            o3_init, o2_init, n2_init, error)
  if (.not. error%is_success()) then
    write(*,*) "Error getting profiles: ", error%message()
    stop 1
  end if

  ! --- Set up MICM Chapman solver (v1 config) ---
  write(*,*) "Setting up MICM Chapman solver..."
  micm => micm_t("../v1/chapman/config.json", RosenbrockStandardOrder, error)
  if (.not. error%is_success()) then
    write(*,*) "Error creating MICM: ", error%message()
    stop 1
  end if

  state => micm%get_state(NUM_CELLS, error)
  if (.not. error%is_success()) then
    write(*,*) "Error creating state: ", error%message()
    stop 1
  end if

  ! Get MICM species indices
  O2_idx = state%species_ordering%index("O2", error)
  if (.not. error%is_success()) stop 1
  O_idx = state%species_ordering%index("O", error)
  if (.not. error%is_success()) stop 1
  O1D_idx = state%species_ordering%index("O1D", error)
  if (.not. error%is_success()) stop 1
  O3_idx = state%species_ordering%index("O3", error)
  if (.not. error%is_success()) stop 1
  N2_idx = state%species_ordering%index("N2", error)
  if (.not. error%is_success()) stop 1

  ! Get MICM photolysis rate parameter indices
  jo2_micm_idx = state%rate_parameters_ordering%index("PHOTO.jo2_b", error)
  if (.not. error%is_success()) stop 1
  jo3a_micm_idx = state%rate_parameters_ordering%index("PHOTO.jo3_a", error)
  if (.not. error%is_success()) stop 1
  jo3b_micm_idx = state%rate_parameters_ordering%index("PHOTO.jo3_b", error)
  if (.not. error%is_success()) stop 1

  ! Set environmental conditions and initial concentrations
  cell_stride = state%species_strides%grid_cell
  var_stride = state%species_strides%variable
  do i = 1, NUM_CELLS
    state%conditions(i)%temperature = temperature(i)
    state%conditions(i)%pressure = pressure(i)
    state%conditions(i)%air_density = pressure(i) / (R_GAS * temperature(i))

    state%concentrations(1 + (i-1)*cell_stride + (O2_idx-1)*var_stride)  = o2_init(i)
    state%concentrations(1 + (i-1)*cell_stride + (O_idx-1)*var_stride)   = 0.0_dk
    state%concentrations(1 + (i-1)*cell_stride + (O1D_idx-1)*var_stride) = 0.0_dk
    state%concentrations(1 + (i-1)*cell_stride + (O3_idx-1)*var_stride)  = o3_init(i)
    state%concentrations(1 + (i-1)*cell_stride + (N2_idx-1)*var_stride)  = n2_init(i)
  end do

  ! Initialize photolysis rates to zero (starting at midnight)
  jo3a_vals = 0.0_dk
  jo3b_vals = 0.0_dk
  jo2_vals = 0.0_dk
  call set_photolysis_rates(state, photo_rates, &
      jo2_tuvx_idx, jo3a_tuvx_idx, jo3b_tuvx_idx, &
      jo2_micm_idx, jo3a_micm_idx, jo3b_micm_idx, &
      .false., jo3a_vals, jo3b_vals, jo2_vals)

  ! --- Open CSV output ---
  open(newunit=csv_unit, file='column_model_fortran.csv', &
      status='replace', action='write')
  write(csv_unit,'(A)') 'time_hr,height_km,O3,O,O1D,jO3a,jO3b,jO2'

  ! Write initial state (t = 0)
  do i = 1, NUM_CELLS
    write(csv_unit,'(F8.3,",",F6.1,6(",",ES16.8E3))') &
        0.0_dk, height_mid(i), &
        state%concentrations(1 + (i-1)*cell_stride + (O3_idx-1)*var_stride), &
        state%concentrations(1 + (i-1)*cell_stride + (O_idx-1)*var_stride), &
        state%concentrations(1 + (i-1)*cell_stride + (O1D_idx-1)*var_stride), &
        jo3a_vals(i), jo3b_vals(i), jo2_vals(i)
  end do

  ! --- Time loop ---
  n_steps = nint(SIM_LENGTH / DT_PHOTO)
  write(*,'(A,I0,A,F5.1,A)') " Running ", n_steps, " steps of ", &
      DT_PHOTO / 60.0_dk, " min..."
  write(*,*)

  do step = 1, n_steps
    current_sec = real(step - 1, dk) * DT_PHOTO
    hour_utc = START_UTC + current_sec / 3600.0_dk
    sza = compute_sza(hour_utc, DAY_OF_YEAR)
    is_day = (sza < PI / 2.0_dk)

    ! Update photolysis rates
    if (is_day) then
      photo_rates = 0.0_dk
      heating_rates = 0.0_dk
      call tuvx%run(sza, 1.0_dk, photo_rates, heating_rates, error)
      if (.not. error%is_success()) then
        write(*,*) "TUV-x error at step ", step, ": ", error%message()
        stop 1
      end if
    end if

    call set_photolysis_rates(state, photo_rates, &
        jo2_tuvx_idx, jo3a_tuvx_idx, jo3b_tuvx_idx, &
        jo2_micm_idx, jo3a_micm_idx, jo3b_micm_idx, &
        is_day, jo3a_vals, jo3b_vals, jo2_vals)

    ! Integrate chemistry
    elapsed = 0.0_dk
    do while (elapsed < DT_PHOTO)
      remaining = DT_PHOTO - elapsed
      call micm%solve(remaining, state, solver_state, solver_stats, error)
      if (.not. error%is_success()) then
        write(*,*) "MICM error at step ", step, ": ", error%message()
        stop 1
      end if
      elapsed = elapsed + solver_stats%final_time()
      if (solver_state%get_char_array() /= "Converged") then
        write(*,*) "  Warning: solver ", solver_state%get_char_array(), &
            " at step ", step
      end if
    end do

    ! Write state to CSV
    local_hr = (current_sec + DT_PHOTO) / 3600.0_dk
    do i = 1, NUM_CELLS
      write(csv_unit,'(F8.3,",",F6.1,6(",",ES16.8E3))') &
          local_hr, height_mid(i), &
          state%concentrations(1 + (i-1)*cell_stride + (O3_idx-1)*var_stride), &
          state%concentrations(1 + (i-1)*cell_stride + (O_idx-1)*var_stride), &
          state%concentrations(1 + (i-1)*cell_stride + (O1D_idx-1)*var_stride), &
          jo3a_vals(i), jo3b_vals(i), jo2_vals(i)
    end do

    ! Progress
    pct = real(step, dk) / real(n_steps, dk) * 100.0_dk
    if (mod(step, n_steps / 10) == 0) then
      write(*,'(F6.1,A,F6.1,A,F6.1,A)') pct, "%  local time ", &
          local_hr, " hr  SZA = ", sza / DEG2RAD, " deg"
    end if
  end do

  close(csv_unit)
  write(*,*)
  write(*,*) "Results written to column_model_fortran.csv"

  ! Clean up
  deallocate(photo_rates, heating_rates)
  deallocate(photo_mappings)
  deallocate(state)
  deallocate(micm)
  deallocate(tuvx)

  write(*,*) "Done."

end program column_model
