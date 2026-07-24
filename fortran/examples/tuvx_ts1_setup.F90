! Copyright (C) 2023-2026 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
! TUV-x vTS1/TSMLT configuration setup module
!
! This module mirrors the Python setup in python/musica/tuvx/vTS1.py,
! providing grid, profile, and radiator initialization for the TS1/TSMLT
! photolysis configuration of TUV-x. It reuses the height grid, profile
! loading, and aerosol radiator helpers from the v5.4 setup module and only
! overrides the wavelength grid, configuration file, and TS1-specific data
! files.
!
module tuvx_ts1_setup
  use musica_util, only: dk => musica_dk, error_t
  use musica_tuvx, only: tuvx_t
  use musica_tuvx_grid, only: grid_t
  use musica_tuvx_grid_map, only: grid_map_t
  use musica_tuvx_profile, only: profile_t
  use musica_tuvx_profile_map, only: profile_map_t
  use musica_tuvx_radiator, only: radiator_t
  use musica_tuvx_radiator_map, only: radiator_map_t
  use tuvx_v54_setup, only: setup_height_grid, load_profile, &
      load_aerosol_radiator

  implicit none
  private
  public :: get_tuvx_ts1_calculator, config_file_path

  integer, parameter :: NUM_WAVELENGTH_SECTIONS = 102

  character(len=*), parameter :: CONFIG_PATH = "ts1_tsmlt.json"

contains

  !> Build a TUV-x calculator configured for the TS1/TSMLT photolysis setup
  function get_tuvx_ts1_calculator(error) result(tuvx)
    type(error_t), intent(inout) :: error
    type(tuvx_t), pointer :: tuvx

    type(grid_t), pointer :: heights, wavelengths
    type(grid_map_t), pointer :: grids
    type(profile_t), pointer :: prof
    type(profile_map_t), pointer :: profiles
    type(radiator_t), pointer :: aer
    type(radiator_map_t), pointer :: radiators

    ! Create grids (height grid is shared with the v5.4 setup)
    heights => setup_height_grid(error)
    if (.not. error%is_success()) return
    wavelengths => setup_ts1_wavelength_grid(error)
    if (.not. error%is_success()) return

    grids => grid_map_t(error)
    if (.not. error%is_success()) return
    call grids%add(heights, error)
    if (.not. error%is_success()) return
    call grids%add(wavelengths, error)
    if (.not. error%is_success()) return

    ! Create profiles. Atmospheric profiles reuse the v5.4 data files; the
    ! surface albedo and extraterrestrial flux use TS1-specific files.
    profiles => profile_map_t(error)
    if (.not. error%is_success()) return

    prof => load_profile("air", "molecule cm-3", heights, &
        "data/profiles/atmosphere/air.v54.dat", error)
    if (.not. error%is_success()) return
    call profiles%add(prof, error)
    if (.not. error%is_success()) return
    deallocate(prof)

    prof => load_profile("O3", "molecule cm-3", heights, &
        "data/profiles/atmosphere/o3.v54.dat", error)
    if (.not. error%is_success()) return
    call profiles%add(prof, error)
    if (.not. error%is_success()) return
    deallocate(prof)

    prof => load_profile("O2", "molecule cm-3", heights, &
        "data/profiles/atmosphere/o2.v54.dat", error)
    if (.not. error%is_success()) return
    call profiles%add(prof, error)
    if (.not. error%is_success()) return
    deallocate(prof)

    prof => load_profile("temperature", "K", heights, &
        "data/profiles/atmosphere/temperature.v54.dat", error)
    if (.not. error%is_success()) return
    call profiles%add(prof, error)
    if (.not. error%is_success()) return
    deallocate(prof)

    prof => load_profile("surface albedo", "none", wavelengths, &
        "data/profiles/solar/surface_albedo.ts1.dat", error)
    if (.not. error%is_success()) return
    call profiles%add(prof, error)
    if (.not. error%is_success()) return
    deallocate(prof)

    prof => load_profile("extraterrestrial flux", "photon cm-2 s-1", &
        wavelengths, &
        "data/profiles/solar/extraterrestrial_flux.ts1.dat", &
        error)
    if (.not. error%is_success()) return
    call profiles%add(prof, error)
    if (.not. error%is_success()) return
    deallocate(prof)

    ! Create radiators
    radiators => radiator_map_t(error)
    if (.not. error%is_success()) return
    aer => load_aerosol_radiator(heights, wavelengths, &
        "data/radiators/aerosol.ts1.dat", error)
    if (.not. error%is_success()) return
    call radiators%add(aer, error)
    if (.not. error%is_success()) return
    deallocate(aer)

    ! Create TUV-x instance with the TS1/TSMLT configuration file
    tuvx => tuvx_t(CONFIG_PATH, grids, profiles, radiators, error)

    ! Clean up construction-time maps (TUV-x owns internal copies)
    deallocate(heights)
    deallocate(wavelengths)
    deallocate(grids)
    deallocate(profiles)
    deallocate(radiators)

  end function get_tuvx_ts1_calculator

  !> Path to the TS1/TSMLT TUV-x configuration file (resolved from CWD)
  function config_file_path() result(path)
    character(len=:), allocatable :: path
    path = CONFIG_PATH
  end function config_file_path

  !> Create the 102-section TS1/TSMLT wavelength grid
  function setup_ts1_wavelength_grid(error) result(grid)
    type(error_t), intent(inout) :: error
    type(grid_t), pointer :: grid

    real(dk) :: edges(NUM_WAVELENGTH_SECTIONS + 1)
    real(dk) :: midpoints(NUM_WAVELENGTH_SECTIONS)
    integer :: i

    grid => grid_t("wavelength", "nm", NUM_WAVELENGTH_SECTIONS, error)
    if (.not. error%is_success()) return

    edges = [ &
        120.0_dk, 121.4_dk, 121.9_dk, 123.5_dk, 124.3_dk, &
        125.5_dk, 126.3_dk, 127.1_dk, 130.1_dk, 131.1_dk, &
        135.0_dk, 140.0_dk, 145.0_dk, 150.0_dk, 155.0_dk, &
        160.0_dk, 165.0_dk, 168.0_dk, 171.0_dk, 173.0_dk, &
        174.4_dk, 175.4_dk, 177.0_dk, 178.6_dk, 180.2_dk, &
        181.8_dk, 183.5_dk, 185.2_dk, 186.9_dk, 188.7_dk, &
        190.5_dk, 192.3_dk, 194.2_dk, 196.1_dk, 198.0_dk, &
        200.0_dk, 202.0_dk, 204.1_dk, 206.2_dk, 208.0_dk, &
        211.0_dk, 214.0_dk, 217.0_dk, 220.0_dk, 223.0_dk, &
        226.0_dk, 229.0_dk, 232.0_dk, 235.0_dk, 238.0_dk, &
        241.0_dk, 244.0_dk, 247.0_dk, 250.0_dk, 253.0_dk, &
        256.0_dk, 259.0_dk, 263.0_dk, 267.0_dk, 271.0_dk, &
        275.0_dk, 279.0_dk, 283.0_dk, 287.0_dk, 291.0_dk, &
        295.0_dk, 298.5_dk, 302.5_dk, 305.5_dk, 308.5_dk, &
        311.5_dk, 314.5_dk, 317.5_dk, 322.5_dk, 327.5_dk, &
        332.5_dk, 337.5_dk, 342.5_dk, 347.5_dk, 350.0_dk, &
        355.0_dk, 360.0_dk, 365.0_dk, 370.0_dk, 375.0_dk, &
        380.0_dk, 385.0_dk, 390.0_dk, 395.0_dk, 400.0_dk, &
        405.0_dk, 410.0_dk, 415.0_dk, 420.0_dk, 430.0_dk, &
        440.0_dk, 450.0_dk, 500.0_dk, 550.0_dk, 600.0_dk, &
        650.0_dk, 700.0_dk, 750.0_dk ]

    do i = 1, NUM_WAVELENGTH_SECTIONS
      midpoints(i) = 0.5_dk * (edges(i) + edges(i + 1))
    end do

    call grid%set_edges(edges, error)
    if (.not. error%is_success()) return
    call grid%set_midpoints(midpoints, error)

  end function setup_ts1_wavelength_grid

end module tuvx_ts1_setup
