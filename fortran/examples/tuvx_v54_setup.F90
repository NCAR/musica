! Copyright (C) 2023-2026 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
! TUV-x v5.4 configuration setup module
!
! This module mirrors the Python setup in python/musica/tuvx/v54.py,
! providing grid, profile, and radiator initialization for TUV-x v5.4.
!
module tuvx_v54_setup
  use musica_util, only: dk => musica_dk, error_t
  use musica_tuvx, only: tuvx_t
  use musica_tuvx_grid, only: grid_t
  use musica_tuvx_grid_map, only: grid_map_t
  use musica_tuvx_profile, only: profile_t
  use musica_tuvx_profile_map, only: profile_map_t
  use musica_tuvx_radiator, only: radiator_t
  use musica_tuvx_radiator_map, only: radiator_map_t

  implicit none
  private
  public :: get_tuvx_calculator

  integer, parameter :: NUM_HEIGHT_SECTIONS = 120
  integer, parameter :: NUM_WAVELENGTH_SECTIONS = 156

  character(len=*), parameter :: CONFIG_PATH = "tuv_5_4.json"

contains

  function get_tuvx_calculator(error) result(tuvx)
    type(error_t), intent(inout) :: error
    type(tuvx_t), pointer :: tuvx

    type(grid_t), pointer :: heights, wavelengths
    type(grid_map_t), pointer :: grids
    type(profile_t), pointer :: prof
    type(profile_map_t), pointer :: profiles
    type(radiator_t), pointer :: aer
    type(radiator_map_t), pointer :: radiators

    ! Create grids
    heights => setup_height_grid(error)
    if (.not. error%is_success()) return
    wavelengths => setup_wavelength_grid(error)
    if (.not. error%is_success()) return

    grids => grid_map_t(error)
    if (.not. error%is_success()) return
    call grids%add(heights, error)
    if (.not. error%is_success()) return
    call grids%add(wavelengths, error)
    if (.not. error%is_success()) return

    ! Create profiles
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
        "data/profiles/solar/surface_albedo.v54.dat", error)
    if (.not. error%is_success()) return
    call profiles%add(prof, error)
    if (.not. error%is_success()) return
    deallocate(prof)

    prof => load_profile("extraterrestrial flux", "photon cm-2 s-1", &
        wavelengths, &
        "data/profiles/solar/extraterrestrial_flux.v54.dat", &
        error)
    if (.not. error%is_success()) return
    call profiles%add(prof, error)
    if (.not. error%is_success()) return
    deallocate(prof)

    ! Create radiators
    radiators => radiator_map_t(error)
    if (.not. error%is_success()) return
    aer => load_aerosol_radiator(heights, wavelengths, error)
    if (.not. error%is_success()) return
    call radiators%add(aer, error)
    if (.not. error%is_success()) return
    deallocate(aer)

    ! Create TUV-x instance
    tuvx => tuvx_t(CONFIG_PATH, grids, profiles, radiators, error)

    ! Clean up construction-time maps (TUV-x owns internal copies)
    deallocate(heights)
    deallocate(wavelengths)
    deallocate(grids)
    deallocate(profiles)
    deallocate(radiators)

  end function get_tuvx_calculator

  !> Create 120-section height grid: 0–120 km, 1 km resolution
  function setup_height_grid(error) result(grid)
    type(error_t), intent(inout) :: error
    type(grid_t), pointer :: grid

    real(dk) :: edges(NUM_HEIGHT_SECTIONS + 1)
    real(dk) :: midpoints(NUM_HEIGHT_SECTIONS)
    integer :: i

    grid => grid_t("height", "km", NUM_HEIGHT_SECTIONS, error)
    if (.not. error%is_success()) return

    do i = 1, NUM_HEIGHT_SECTIONS + 1
      edges(i) = real(i - 1, dk)
    end do
    do i = 1, NUM_HEIGHT_SECTIONS
      midpoints(i) = 0.5_dk * (edges(i) + edges(i + 1))
    end do

    call grid%set_edges(edges, error)
    if (.not. error%is_success()) return
    call grid%set_midpoints(midpoints, error)

  end function setup_height_grid

  !> Create 156-section wavelength grid with exact v5.4 edges
  function setup_wavelength_grid(error) result(grid)
    type(error_t), intent(inout) :: error
    type(grid_t), pointer :: grid

    real(dk) :: edges(NUM_WAVELENGTH_SECTIONS + 1)
    real(dk) :: midpoints(NUM_WAVELENGTH_SECTIONS)
    integer :: i

    grid => grid_t("wavelength", "nm", NUM_WAVELENGTH_SECTIONS, error)
    if (.not. error%is_success()) return

    edges = [ &
        120.0000_dk, 121.4000_dk, 121.9000_dk, 122.3000_dk, 123.1000_dk, &
        123.8000_dk, 124.6000_dk, 125.4000_dk, 126.2000_dk, 127.0000_dk, &
        128.6000_dk, 129.4000_dk, 130.3000_dk, 132.0000_dk, 135.0000_dk, &
        137.0000_dk, 145.0000_dk, 155.0000_dk, 165.0000_dk, 170.0000_dk, &
        175.4000_dk, 177.0000_dk, 178.6000_dk, 180.2000_dk, 181.8000_dk, &
        183.5000_dk, 185.2000_dk, 186.9000_dk, 188.7000_dk, 190.5000_dk, &
        192.3000_dk, 194.2000_dk, 196.1000_dk, 198.0000_dk, 200.0000_dk, &
        202.0000_dk, 204.1000_dk, 206.2000_dk, 208.3330_dk, 210.5260_dk, &
        212.7660_dk, 215.0540_dk, 217.3910_dk, 219.7800_dk, 222.2220_dk, &
        224.7190_dk, 227.2730_dk, 229.8850_dk, 232.5580_dk, 235.2940_dk, &
        238.0950_dk, 240.9640_dk, 243.9020_dk, 246.9140_dk, 250.0000_dk, &
        253.1650_dk, 256.4100_dk, 259.7400_dk, 263.1580_dk, 266.6670_dk, &
        270.2700_dk, 273.9730_dk, 277.7780_dk, 281.6900_dk, 285.7140_dk, &
        289.8550_dk, 294.1180_dk, 298.5000_dk, 302.5000_dk, 303.5000_dk, &
        304.5000_dk, 305.5000_dk, 306.5000_dk, 307.5000_dk, 308.5000_dk, &
        309.5000_dk, 310.5000_dk, 311.5000_dk, 312.5000_dk, 313.5000_dk, &
        314.5000_dk, 317.5000_dk, 322.5000_dk, 327.5000_dk, 332.5000_dk, &
        337.5000_dk, 342.5000_dk, 347.5000_dk, 352.5000_dk, 357.5000_dk, &
        362.5000_dk, 367.5000_dk, 372.5000_dk, 377.5000_dk, 382.5000_dk, &
        387.5000_dk, 392.5000_dk, 397.5000_dk, 402.5000_dk, 407.5000_dk, &
        412.5000_dk, 417.5000_dk, 422.5000_dk, 427.5000_dk, 432.5000_dk, &
        437.5000_dk, 442.5000_dk, 447.5000_dk, 452.5000_dk, 457.5000_dk, &
        462.5000_dk, 467.5000_dk, 472.5000_dk, 477.5000_dk, 482.5000_dk, &
        487.5000_dk, 492.5000_dk, 497.5000_dk, 502.5000_dk, 507.5000_dk, &
        512.5000_dk, 517.5000_dk, 522.5000_dk, 527.5000_dk, 532.5000_dk, &
        537.5000_dk, 542.5000_dk, 547.5000_dk, 552.5000_dk, 557.5000_dk, &
        562.5000_dk, 567.5000_dk, 572.5000_dk, 577.5000_dk, 582.5000_dk, &
        587.5000_dk, 592.5000_dk, 597.5000_dk, 602.5000_dk, 607.5000_dk, &
        612.5000_dk, 617.5000_dk, 622.5000_dk, 627.5000_dk, 632.5000_dk, &
        637.5000_dk, 642.5000_dk, 647.1000_dk, 655.0000_dk, 665.0000_dk, &
        675.0000_dk, 685.0000_dk, 695.0000_dk, 705.0000_dk, 715.0000_dk, &
        725.0000_dk, 735.0000_dk ]

    do i = 1, NUM_WAVELENGTH_SECTIONS
      midpoints(i) = 0.5_dk * (edges(i) + edges(i + 1))
    end do

    call grid%set_edges(edges, error)
    if (.not. error%is_success()) return
    call grid%set_midpoints(midpoints, error)

  end function setup_wavelength_grid

  !> Load a profile from a .dat file and set it up on the given grid
  function load_profile(prof_name, prof_units, grid, filepath, error) &
      result(prof)
    character(len=*), intent(in) :: prof_name
    character(len=*), intent(in) :: prof_units
    type(grid_t), intent(in) :: grid
    character(len=*), intent(in) :: filepath
    type(error_t), intent(inout) :: error
    type(profile_t), pointer :: prof

    integer :: unit_num, ios, num_sections, i
    character(len=1024) :: line
    logical :: in_midpoint, in_edge
    real(dk) :: exo_layer_density

    ! File data arrays (max 200 entries)
    real(dk) :: file_mid_coords(200), file_mid_values(200)
    real(dk) :: file_layer_dens(200)
    real(dk) :: file_edge_coords(300), file_edge_values(300)
    integer :: n_mid, n_edge

    ! Grid arrays
    real(dk), allocatable :: grid_edges(:), grid_midpoints(:)

    ! Interpolated arrays
    real(dk), allocatable :: interp_edges(:), interp_midpoints(:)
    real(dk), allocatable :: interp_layer_dens(:)

    ! Parse file columns
    real(dk) :: col1, col2, col3, col4, col5
    character(len=64) :: s1, s2, s3, s4, s5, s6

    num_sections = grid%number_of_sections(error)
    if (.not. error%is_success()) return

    allocate(grid_edges(num_sections + 1))
    allocate(grid_midpoints(num_sections))
    allocate(interp_edges(num_sections + 1))
    allocate(interp_midpoints(num_sections))
    allocate(interp_layer_dens(num_sections))

    call grid%get_edges(grid_edges, error)
    if (.not. error%is_success()) return
    call grid%get_midpoints(grid_midpoints, error)
    if (.not. error%is_success()) return

    ! Parse the data file
    in_midpoint = .false.
    in_edge = .false.
    n_mid = 0
    n_edge = 0
    exo_layer_density = 0.0_dk

    open(newunit=unit_num, file=filepath, status='old', action='read', &
        iostat=ios)
    if (ios /= 0) then
      write(*,*) "ERROR: Could not open file: ", trim(filepath)
      stop 1
    end if

    do
      read(unit_num, '(A)', iostat=ios) line
      if (ios /= 0) exit

      line = adjustl(line)

      ! Check for section headers
      if (line(1:1) == '#' .or. len_trim(line) == 0) then
        if (index(line, 'mid-point') > 0 .and. &
            (index(line, 'height (km)') > 0 .or. &
             index(line, 'wavelength (nm)') > 0)) then
          in_midpoint = .true.
          in_edge = .false.
        else if (index(line, 'edge') > 0 .and. &
            (index(line, 'height (km)') > 0 .or. &
             index(line, 'wavelength (nm)') > 0)) then
          in_edge = .true.
          in_midpoint = .false.
        end if
        cycle
      end if

      if (in_midpoint) then
        ! Check for exo row (all --- entries)
        if (index(line, '---,---,---') > 0 .and. &
            line(1:3) == '---') then
          ! Parse exo_layer_density from column 5 if present
          call parse_exo_row(line, exo_layer_density)
          cycle
        end if
        ! Parse: coord, value, delta, layer_density, ...
        call parse_midpoint_line(line, col1, col2, col4, ios)
        if (ios == 0) then
          n_mid = n_mid + 1
          file_mid_coords(n_mid) = col1
          file_mid_values(n_mid) = col2
          file_layer_dens(n_mid) = col4
        end if
      else if (in_edge) then
        ! Parse: coord, value
        call parse_edge_line(line, col1, col2, ios)
        if (ios == 0) then
          n_edge = n_edge + 1
          file_edge_coords(n_edge) = col1
          file_edge_values(n_edge) = col2
        end if
      end if
    end do

    close(unit_num)

    ! Interpolate onto the target grid
    call linear_interp(file_mid_coords(1:n_mid), file_mid_values(1:n_mid), &
        grid_midpoints, interp_midpoints)
    call linear_interp(file_edge_coords(1:n_edge), file_edge_values(1:n_edge),&
        grid_edges, interp_edges)
    call linear_interp(file_mid_coords(1:n_mid), file_layer_dens(1:n_mid), &
        grid_midpoints, interp_layer_dens)

    ! Remove exo layer density from the top layer density if present
    ! (it will be added back by set_exo_layer_density)
    if (exo_layer_density > 0.0_dk) then
      interp_layer_dens(num_sections) = &
          interp_layer_dens(num_sections) - exo_layer_density
    end if

    ! Create and configure the profile
    prof => profile_t(prof_name, prof_units, grid, error)
    if (.not. error%is_success()) return

    call prof%set_edge_values(interp_edges, error)
    if (.not. error%is_success()) return
    call prof%set_midpoint_values(interp_midpoints, error)
    if (.not. error%is_success()) return
    call prof%set_layer_densities(interp_layer_dens, error)
    if (.not. error%is_success()) return

    if (exo_layer_density > 0.0_dk) then
      call prof%set_exo_layer_density(exo_layer_density, error)
      if (.not. error%is_success()) return
    end if

    deallocate(grid_edges, grid_midpoints)
    deallocate(interp_edges, interp_midpoints, interp_layer_dens)

  end function load_profile

  !> Parse a midpoint data line (comma-separated)
  subroutine parse_midpoint_line(line, coord, value, layer_dens, ios)
    character(len=*), intent(in) :: line
    real(dk), intent(out) :: coord, value, layer_dens
    integer, intent(out) :: ios

    character(len=64) :: fields(6)
    integer :: nf

    call split_csv(line, fields, nf)
    if (nf < 4) then
      ios = 1
      return
    end if

    read(fields(1), *, iostat=ios) coord
    if (ios /= 0) return
    read(fields(2), *, iostat=ios) value
    if (ios /= 0) return
    read(fields(4), *, iostat=ios) layer_dens
    if (ios /= 0) layer_dens = 0.0_dk

    ios = 0
  end subroutine parse_midpoint_line

  !> Parse an edge data line (comma-separated)
  subroutine parse_edge_line(line, coord, value, ios)
    character(len=*), intent(in) :: line
    real(dk), intent(out) :: coord, value
    integer, intent(out) :: ios

    character(len=64) :: fields(6)
    integer :: nf

    call split_csv(line, fields, nf)
    if (nf < 2) then
      ios = 1
      return
    end if

    read(fields(1), *, iostat=ios) coord
    if (ios /= 0) return
    read(fields(2), *, iostat=ios) value
  end subroutine parse_edge_line

  !> Parse the exo row to extract exo_layer_density
  subroutine parse_exo_row(line, exo_layer_density)
    character(len=*), intent(in) :: line
    real(dk), intent(inout) :: exo_layer_density

    character(len=64) :: fields(6)
    integer :: nf, ios

    call split_csv(line, fields, nf)
    if (nf >= 5) then
      if (trim(adjustl(fields(5))) /= '---') then
        read(fields(5), *, iostat=ios) exo_layer_density
      end if
    end if
  end subroutine parse_exo_row

  !> Split a comma-separated line into fields
  subroutine split_csv(line, fields, nf)
    character(len=*), intent(in) :: line
    character(len=64), intent(out) :: fields(:)
    integer, intent(out) :: nf

    integer :: pos, start, flen

    nf = 0
    start = 1
    flen = len_trim(line)

    do while (start <= flen .and. nf < size(fields))
      pos = index(line(start:flen), ',')
      nf = nf + 1
      if (pos == 0) then
        fields(nf) = adjustl(line(start:flen))
        exit
      else
        fields(nf) = adjustl(line(start:start + pos - 2))
        start = start + pos
      end if
    end do
  end subroutine split_csv

  !> Linear interpolation: numpy.interp equivalent
  subroutine linear_interp(xp, fp, x, result)
    real(dk), intent(in) :: xp(:), fp(:), x(:)
    real(dk), intent(out) :: result(:)

    integer :: i, j, np
    real(dk) :: t

    np = size(xp)

    do i = 1, size(x)
      if (x(i) <= xp(1)) then
        result(i) = fp(1)
      else if (x(i) >= xp(np)) then
        result(i) = fp(np)
      else
        ! Find the bracketing interval
        do j = 1, np - 1
          if (x(i) >= xp(j) .and. x(i) <= xp(j + 1)) then
            t = (x(i) - xp(j)) / (xp(j + 1) - xp(j))
            result(i) = fp(j) + t * (fp(j + 1) - fp(j))
            exit
          end if
        end do
      end if
    end do
  end subroutine linear_interp

  !> Load aerosol radiator from the v5.4 data file
  function load_aerosol_radiator(heights, wavelengths, error) result(rad)
    type(grid_t), intent(in) :: heights
    type(grid_t), intent(in) :: wavelengths
    type(error_t), intent(inout) :: error
    type(radiator_t), pointer :: rad

    character(len=*), parameter :: filepath = &
        "data/radiators/aerosol.v54.dat"

    integer :: nh, nw, unit_num, ios, hi, wi, i
    real(dk), allocatable :: h_mid(:), w_mid(:)
    real(dk), allocatable :: optical_depths(:,:)
    real(dk), allocatable :: ssa(:,:)
    real(dk), allocatable :: asym(:,:,:)

    real(dk) :: file_h, file_w, file_od, file_ssa, file_g
    real(dk) :: min_dist
    character(len=1024) :: line

    nh = heights%number_of_sections(error)
    if (.not. error%is_success()) return
    nw = wavelengths%number_of_sections(error)
    if (.not. error%is_success()) return

    allocate(h_mid(nh))
    allocate(w_mid(nw))
    allocate(optical_depths(nh, nw))
    allocate(ssa(nh, nw))
    allocate(asym(nh, nw, 1))

    call heights%get_midpoints(h_mid, error)
    if (.not. error%is_success()) return
    call wavelengths%get_midpoints(w_mid, error)
    if (.not. error%is_success()) return

    optical_depths = 0.0_dk
    ssa = 0.0_dk
    asym = 0.0_dk

    open(newunit=unit_num, file=filepath, status='old', action='read', &
        iostat=ios)
    if (ios /= 0) then
      write(*,*) "ERROR: Could not open file: ", trim(filepath)
      stop 1
    end if

    do
      read(unit_num, '(A)', iostat=ios) line
      if (ios /= 0) exit

      line = adjustl(line)
      if (len_trim(line) == 0 .or. line(1:1) == '#') cycle

      read(line, *, iostat=ios) file_h, file_w, file_od, file_ssa, file_g
      if (ios /= 0) cycle

      ! Find closest height midpoint
      hi = 1
      min_dist = abs(h_mid(1) - file_h)
      do i = 2, nh
        if (abs(h_mid(i) - file_h) < min_dist) then
          min_dist = abs(h_mid(i) - file_h)
          hi = i
        end if
      end do

      ! Find closest wavelength midpoint
      wi = 1
      min_dist = abs(w_mid(1) - file_w)
      do i = 2, nw
        if (abs(w_mid(i) - file_w) < min_dist) then
          min_dist = abs(w_mid(i) - file_w)
          wi = i
        end if
      end do

      optical_depths(hi, wi) = file_od
      ssa(hi, wi) = file_ssa
      asym(hi, wi, 1) = file_g
    end do

    close(unit_num)

    ! Create radiator and set arrays
    rad => radiator_t("aerosol", heights, wavelengths, error)
    if (.not. error%is_success()) return

    call rad%set_optical_depths(optical_depths, error)
    if (.not. error%is_success()) return
    call rad%set_single_scattering_albedos(ssa, error)
    if (.not. error%is_success()) return
    call rad%set_asymmetry_factors(asym, error)
    if (.not. error%is_success()) return

    deallocate(h_mid, w_mid, optical_depths, ssa, asym)

  end function load_aerosol_radiator

end module tuvx_v54_setup
