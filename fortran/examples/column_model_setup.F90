! Copyright (C) 2023-2026 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
! Column model setup module
!
! Provides SZA calculation, initial condition extraction from TUV-x profiles,
! and photolysis rate mapping between TUV-x and MICM.
!
module column_model_setup
  use musica_util, only: dk => musica_dk, error_t, mappings_t
  use musica_tuvx, only: tuvx_t
  use musica_tuvx_grid, only: grid_t
  use musica_tuvx_grid_map, only: grid_map_t
  use musica_tuvx_profile, only: profile_t
  use musica_tuvx_profile_map, only: profile_map_t
  use musica_micm, only: micm_t
  use musica_state, only: state_t

  implicit none
  private
  public :: compute_sza, get_initial_profiles, set_photolysis_rates

  real(dk), parameter, public :: PI = 3.14159265358979323846_dk
  real(dk), parameter, public :: DEG2RAD = PI / 180.0_dk
  real(dk), parameter, public :: AVOGADRO = 6.02214076e23_dk
  real(dk), parameter, public :: BOLTZMANN = 1.380649e-23_dk
  real(dk), parameter, public :: R_GAS = 8.31446261815324_dk

  integer, parameter, public :: NUM_CELLS = 120

  ! Boulder, CO
  real(dk), parameter, public :: LATITUDE  = 40.015_dk
  real(dk), parameter, public :: LONGITUDE = -105.27_dk

contains

  ! Compute solar zenith angle using Spencer (1971) declination formula
  function compute_sza(hour_utc, day_of_year) result(sza)
    real(dk), intent(in) :: hour_utc
    integer, intent(in) :: day_of_year
    real(dk) :: sza

    real(dk) :: lat, gamma, dec, solar_hour, ha, cos_sza

    lat = LATITUDE * DEG2RAD
    gamma = 2.0_dk * PI * real(day_of_year - 1, dk) / 365.0_dk
    dec = 0.006918_dk &
        - 0.399912_dk * cos(gamma) + 0.070257_dk * sin(gamma) &
        - 0.006758_dk * cos(2.0_dk * gamma) + 0.000907_dk * sin(2.0_dk * gamma) &
        - 0.002697_dk * cos(3.0_dk * gamma) + 0.00148_dk * sin(3.0_dk * gamma)
    solar_hour = hour_utc + LONGITUDE / 15.0_dk
    ha = (solar_hour - 12.0_dk) * 15.0_dk * DEG2RAD
    cos_sza = sin(lat) * sin(dec) + cos(lat) * cos(dec) * cos(ha)
    cos_sza = max(-1.0_dk, min(1.0_dk, cos_sza))
    sza = acos(cos_sza)
  end function compute_sza

  ! Extract initial atmospheric profiles from TUV-x
  subroutine get_initial_profiles(tuvx, height_mid, temperature, pressure, &
                                   o3_mol_m3, o2_mol_m3, n2_mol_m3, error)
    type(tuvx_t), intent(inout) :: tuvx
    real(dk), intent(out) :: height_mid(NUM_CELLS)
    real(dk), intent(out) :: temperature(NUM_CELLS)
    real(dk), intent(out) :: pressure(NUM_CELLS)
    real(dk), intent(out) :: o3_mol_m3(NUM_CELLS)
    real(dk), intent(out) :: o2_mol_m3(NUM_CELLS)
    real(dk), intent(out) :: n2_mol_m3(NUM_CELLS)
    type(error_t), intent(inout) :: error

    type(grid_map_t), pointer :: grids
    type(profile_map_t), pointer :: profiles
    type(grid_t), pointer :: heights
    type(profile_t), pointer :: air_prof, o3_prof, o2_prof, temp_prof

    real(dk) :: air_mid(NUM_CELLS), o3_mid(NUM_CELLS), o2_mid(NUM_CELLS)
    real(dk) :: temp_mid(NUM_CELLS), height_edges(NUM_CELLS + 1)
    real(dk) :: air_mol_m3(NUM_CELLS)
    real(dk) :: conv
    integer :: i

    conv = 1.0e6_dk / AVOGADRO  ! molecule/cm³ → mol/m³

    grids => tuvx%get_grids(error)
    if (.not. error%is_success()) return
    profiles => tuvx%get_profiles(error)
    if (.not. error%is_success()) return

    heights => grids%get("height", "km", error)
    if (.not. error%is_success()) return
    call heights%get_midpoints(height_mid, error)
    if (.not. error%is_success()) return
    call heights%get_edges(height_edges, error)
    if (.not. error%is_success()) return

    ! Air density
    air_prof => profiles%get("air", "molecule cm-3", error)
    if (.not. error%is_success()) return
    call air_prof%get_midpoint_values(air_mid, error)
    if (.not. error%is_success()) return

    ! Temperature
    temp_prof => profiles%get("temperature", "K", error)
    if (.not. error%is_success()) return
    call temp_prof%get_midpoint_values(temp_mid, error)
    if (.not. error%is_success()) return

    ! O3
    o3_prof => profiles%get("O3", "molecule cm-3", error)
    if (.not. error%is_success()) return
    call o3_prof%get_midpoint_values(o3_mid, error)
    if (.not. error%is_success()) return

    ! O2
    o2_prof => profiles%get("O2", "molecule cm-3", error)
    if (.not. error%is_success()) return
    call o2_prof%get_midpoint_values(o2_mid, error)
    if (.not. error%is_success()) return

    ! Convert units and compute derived quantities
    temperature = temp_mid
    do i = 1, NUM_CELLS
      ! P = n * k_B * T, where n is in molecules/m³
      pressure(i) = air_mid(i) * 1.0e6_dk * BOLTZMANN * temp_mid(i)
      air_mol_m3(i) = air_mid(i) * conv
      o3_mol_m3(i) = o3_mid(i) * conv
      o2_mol_m3(i) = o2_mid(i) * conv
      n2_mol_m3(i) = air_mol_m3(i) * 0.78084_dk / (0.78084_dk + 0.20946_dk)
    end do

    deallocate(heights, air_prof, temp_prof, o3_prof, o2_prof)
    deallocate(profiles, grids)
  end subroutine get_initial_profiles

  ! Map TUV-x photolysis rates to MICM state rate parameters
  !
  ! photo_rates: (NUM_CELLS+1, num_photo_rxns) from TUV-x run
  ! For nighttime, pass is_day = .false. and rates will be zeroed.
  subroutine set_photolysis_rates(state, photo_rates, &
      jo2_tuvx_idx, jo3a_tuvx_idx, jo3b_tuvx_idx, &
      jo2_micm_idx, jo3a_micm_idx, jo3b_micm_idx, &
      is_day, jo3a_out, jo3b_out, jo2_out)
    type(state_t), intent(inout) :: state
    real(dk), intent(in) :: photo_rates(:,:)
    integer, intent(in) :: jo2_tuvx_idx, jo3a_tuvx_idx, jo3b_tuvx_idx
    integer, intent(in) :: jo2_micm_idx, jo3a_micm_idx, jo3b_micm_idx
    logical, intent(in) :: is_day
    real(dk), intent(out) :: jo3a_out(NUM_CELLS)
    real(dk), intent(out) :: jo3b_out(NUM_CELLS)
    real(dk), intent(out) :: jo2_out(NUM_CELLS)

    integer :: i, cell_stride, var_stride

    cell_stride = state%rate_parameters_strides%grid_cell
    var_stride = state%rate_parameters_strides%variable

    if (is_day) then
      ! Average adjacent edges to get midpoint rates
      do i = 1, NUM_CELLS
        jo3a_out(i) = 0.5_dk * (photo_rates(i, jo3a_tuvx_idx) &
                               + photo_rates(i + 1, jo3a_tuvx_idx))
        jo3b_out(i) = 0.5_dk * (photo_rates(i, jo3b_tuvx_idx) &
                               + photo_rates(i + 1, jo3b_tuvx_idx))
        jo2_out(i)  = 0.5_dk * (photo_rates(i, jo2_tuvx_idx) &
                               + photo_rates(i + 1, jo2_tuvx_idx))
      end do
    else
      jo3a_out = 0.0_dk
      jo3b_out = 0.0_dk
      jo2_out  = 0.0_dk
    end if

    do i = 1, NUM_CELLS
      state%rate_parameters(1 + (i - 1) * cell_stride &
          + (jo2_micm_idx - 1) * var_stride) = jo2_out(i)
      state%rate_parameters(1 + (i - 1) * cell_stride &
          + (jo3a_micm_idx - 1) * var_stride) = jo3a_out(i)
      state%rate_parameters(1 + (i - 1) * cell_stride &
          + (jo3b_micm_idx - 1) * var_stride) = jo3b_out(i)
    end do
  end subroutine set_photolysis_rates

end module column_model_setup
