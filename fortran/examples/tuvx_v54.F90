! Copyright (C) 2023-2026 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
! Fortran example for TUV-x v5.4 photolysis configuration
!
! This program mirrors the Python example in python/musica/examples/tuvx_5_4.py.
! It sets up a 120 km vertical column, runs TUV-x twice (standard and doubled O3),
! and prints the O3 photolysis rates at each level.
!
program tuvx_v54_example
  use musica_util, only: dk => musica_dk, error_t, mappings_t
  use musica_tuvx, only: tuvx_t
  use musica_tuvx_grid, only: grid_t
  use musica_tuvx_grid_map, only: grid_map_t
  use musica_tuvx_profile, only: profile_t
  use musica_tuvx_profile_map, only: profile_map_t
  use tuvx_v54_setup, only: get_tuvx_calculator

  implicit none

  integer, parameter :: NUM_HEIGHTS = 120

  type(tuvx_t), pointer :: tuvx
  type(error_t) :: error
  type(mappings_t), pointer :: photo_mappings, heat_mappings
  type(grid_map_t), pointer :: grids
  type(profile_map_t), pointer :: profiles
  type(grid_t), pointer :: heights
  type(profile_t), pointer :: o3_profile

  integer :: num_photo, num_heat, jo3a_idx, jo3b_idx, i
  real(dk), allocatable :: photo_rates(:,:), heating_rates(:,:)
  real(dk), allocatable :: photo_rates_mod(:,:), heating_rates_mod(:,:)
  real(dk) :: edge_vals(NUM_HEIGHTS + 1), mid_vals(NUM_HEIGHTS)
  real(dk) :: layer_dens(NUM_HEIGHTS)
  real(dk) :: height_edges(NUM_HEIGHTS + 1)
  real(dk) :: conv

  character(len=:), allocatable :: rxn_name
  integer :: csv_unit

  write(*,*) "TUV-x v5.4 Fortran Example"
  write(*,*) "=========================="

  ! Create TUV-x calculator
  tuvx => get_tuvx_calculator(error)
  if (.not. error%is_success()) then
    write(*,*) "Error creating TUV-x: ", error%message()
    stop 1
  end if

  ! Get reaction ordering
  photo_mappings => tuvx%get_photolysis_rate_constants_ordering(error)
  if (.not. error%is_success()) then
    write(*,*) "Error getting photo ordering: ", error%message()
    stop 1
  end if
  heat_mappings => tuvx%get_heating_rates_ordering(error)
  if (.not. error%is_success()) then
    write(*,*) "Error getting heating ordering: ", error%message()
    stop 1
  end if

  num_photo = photo_mappings%size()
  num_heat = heat_mappings%size()

  write(*,'(A,I0,A)') " Found ", num_photo, " photolysis reactions"
  write(*,'(A,I0,A)') " Found ", num_heat, " heating rates"
  write(*,*)

  ! Print reaction names
  write(*,*) "Photolysis reactions:"
  do i = 1, num_photo
    rxn_name = photo_mappings%name(i)
    write(*,'(A,I3,A,A)') "  ", i, ": ", rxn_name
  end do
  write(*,*)

  ! Find O3 photolysis reaction indices
  jo3a_idx = photo_mappings%index("O3+hv->O2+O(1D)", error)
  if (.not. error%is_success()) then
    write(*,*) "Error finding O3+hv->O2+O(1D): ", error%message()
    stop 1
  end if
  jo3b_idx = photo_mappings%index("O3+hv->O2+O(3P)", error)
  if (.not. error%is_success()) then
    write(*,*) "Error finding O3+hv->O2+O(3P): ", error%message()
    stop 1
  end if

  write(*,'(A,I0)') " O3+hv->O2+O(1D) index: ", jo3a_idx
  write(*,'(A,I0)') " O3+hv->O2+O(3P) index: ", jo3b_idx
  write(*,*)

  ! Allocate output arrays: (n_layers+1, n_reactions)
  allocate(photo_rates(NUM_HEIGHTS + 1, num_photo))
  allocate(heating_rates(NUM_HEIGHTS + 1, num_heat))
  allocate(photo_rates_mod(NUM_HEIGHTS + 1, num_photo))
  allocate(heating_rates_mod(NUM_HEIGHTS + 1, num_heat))

  photo_rates = 0.0_dk
  heating_rates = 0.0_dk

  ! Run 1: Standard conditions, SZA=0 degrees, earth-sun distance=1.0 AU
  write(*,*) "Running TUV-x with standard conditions..."
  call tuvx%run(0.0_dk, 1.0_dk, photo_rates, heating_rates, error)
  if (.not. error%is_success()) then
    write(*,*) "Error running TUV-x: ", error%message()
    stop 1
  end if
  write(*,*) "Standard run complete."

  ! Double the O3 concentrations
  grids => tuvx%get_grids(error)
  if (.not. error%is_success()) stop 1
  profiles => tuvx%get_profiles(error)
  if (.not. error%is_success()) stop 1
  heights => grids%get("height", "km", error)
  if (.not. error%is_success()) stop 1
  o3_profile => profiles%get("O3", "molecule cm-3", error)
  if (.not. error%is_success()) stop 1

  ! Get current O3 values
  call o3_profile%get_edge_values(edge_vals, error)
  if (.not. error%is_success()) stop 1
  call o3_profile%get_midpoint_values(mid_vals, error)
  if (.not. error%is_success()) stop 1

  ! Double edge and midpoint values
  edge_vals = edge_vals * 2.0_dk
  mid_vals = mid_vals * 2.0_dk

  call o3_profile%set_edge_values(edge_vals, error)
  if (.not. error%is_success()) stop 1
  call o3_profile%set_midpoint_values(mid_vals, error)
  if (.not. error%is_success()) stop 1

  ! Recalculate layer densities (midpoint * delta_height * 1e5)
  call heights%get_edges(height_edges, error)
  if (.not. error%is_success()) stop 1
  conv = 1.0e5_dk
  do i = 1, NUM_HEIGHTS
    layer_dens(i) = mid_vals(i) * (height_edges(i+1) - height_edges(i)) * conv
  end do
  call o3_profile%set_layer_densities(layer_dens, error)
  if (.not. error%is_success()) stop 1

  ! Run 2: With doubled O3
  photo_rates_mod = 0.0_dk
  heating_rates_mod = 0.0_dk

  write(*,*) "Running TUV-x with doubled O3..."
  call tuvx%run(0.0_dk, 1.0_dk, photo_rates_mod, heating_rates_mod, error)
  if (.not. error%is_success()) then
    write(*,*) "Error running TUV-x (modified): ", error%message()
    stop 1
  end if
  write(*,*) "Modified run complete."
  write(*,*)

  ! Print O3 photolysis rates at each level
  write(*,*) "O3 photolysis rates comparison:"
  write(*,'(A6,4A26)') "Level", &
      "O(1D) std", "O(1D) 2xO3", &
      "O(3P) std", "O(3P) 2xO3"
  write(*,*) repeat('-', 110)
  do i = 1, NUM_HEIGHTS + 1
    write(*,'(I6,4ES26.16)') i, &
        photo_rates(i, jo3a_idx), photo_rates_mod(i, jo3a_idx), &
        photo_rates(i, jo3b_idx), photo_rates_mod(i, jo3b_idx)
  end do

  ! Write CSV file for comparison with Python output
  open(newunit=csv_unit, file='tuvx_v54_fortran_results.csv', &
      status='replace', action='write')
  write(csv_unit,'(A)') 'level,height_km,' // &
      'jo3a_std,jo3a_2xO3,jo3b_std,jo3b_2xO3'
  do i = 1, NUM_HEIGHTS + 1
    write(csv_unit,'(I0,",",F8.1,4(",",ES26.16))') i, &
        real(i - 1, dk), &
        photo_rates(i, jo3a_idx), photo_rates_mod(i, jo3a_idx), &
        photo_rates(i, jo3b_idx), photo_rates_mod(i, jo3b_idx)
  end do
  close(csv_unit)
  write(*,*)
  write(*,*) "Results written to tuvx_v54_fortran_results.csv"

  ! Clean up
  deallocate(photo_rates, heating_rates)
  deallocate(photo_rates_mod, heating_rates_mod)
  deallocate(photo_mappings)
  deallocate(heat_mappings)
  deallocate(heights)
  deallocate(o3_profile)
  deallocate(profiles)
  deallocate(grids)
  deallocate(tuvx)

end program tuvx_v54_example
