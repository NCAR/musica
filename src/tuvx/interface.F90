! Copyright (C) 2023-2025 National Center for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
module tuvx_interface

   use iso_c_binding,           only : c_ptr, c_loc, c_int, c_size_t, c_char
   use tuvx_core,               only : core_t
   use tuvx_grid,               only : grid_t
   use tuvx_grid_warehouse,     only : grid_warehouse_t
   use tuvx_profile_warehouse,  only : profile_warehouse_t
   use tuvx_radiator_warehouse, only : radiator_warehouse_t
   use musica_string,           only : string_t

   implicit none

   private

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function internal_create_tuvx(c_config_path, config_path_length, &
      c_grid_map, c_profile_map, c_radiator_map, number_of_layers, error_code) &
      bind(C, name="InternalCreateTuvx")
      use iso_c_binding, only: c_ptr, c_f_pointer

      ! arguments
      character(kind=c_char), dimension(*), intent(in)  :: c_config_path
      integer(kind=c_size_t), value                     :: config_path_length
      type(c_ptr), value,                   intent(in)  :: c_grid_map
      type(c_ptr), value,                   intent(in)  :: c_profile_map
      type(c_ptr), value,                   intent(in)  :: c_radiator_map
      integer(kind=c_int),                  intent(out) :: number_of_layers
      integer(kind=c_int),                  intent(out) :: error_code
      type(c_ptr)                                       :: internal_create_tuvx

      ! local variables
      character(len=:), allocatable       :: f_config_path
      type(core_t), pointer               :: core
      type(string_t)                      :: musica_config_path
      type(grid_warehouse_t),     pointer :: grid_map
      type(profile_warehouse_t),  pointer :: profile_map
      type(radiator_warehouse_t), pointer :: radiator_map
      type(grid_t), pointer               :: height_grid
      integer                             :: i

      allocate(character(len=config_path_length) :: f_config_path)
      do i = 1, config_path_length
         f_config_path(i:i) = c_config_path(i)
      end do

      musica_config_path = string_t(f_config_path)
      call c_f_pointer(c_grid_map, grid_map)
      call c_f_pointer(c_profile_map, profile_map)
      call c_f_pointer(c_radiator_map, radiator_map)
      core => core_t(musica_config_path, grids = grid_map, &
         profiles = profile_map, radiators = radiator_map)
      height_grid => core%get_grid("height", "km")
      number_of_layers = height_grid%ncells_
      deallocate(height_grid)

      deallocate(f_config_path)
      error_code = 0

      internal_create_tuvx = c_loc(core)

   end function internal_create_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_delete_tuvx(tuvx, error_code) &
      bind(C, name="InternalDeleteTuvx")
      use iso_c_binding, only: c_ptr, c_f_pointer

      ! arguments
      type(c_ptr), value,  intent(in)  :: tuvx
      integer(kind=c_int), intent(out) :: error_code

      ! local variables
      type(core_t), pointer :: core

      call c_f_pointer(tuvx, core)
      if (associated(core)) then
         deallocate(core)
      end if
   end subroutine internal_delete_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function internal_get_grid_map(tuvx, error_code) result(grid_map_ptr) &
      bind(C, name="InternalGetGridMap")
      use iso_c_binding, only: c_ptr, c_f_pointer, c_int

      ! arguments
      type(c_ptr), value,  intent(in)  :: tuvx
      integer(kind=c_int), intent(out) :: error_code

      ! result
      type(c_ptr) :: grid_map_ptr

      ! variables
      type(core_t),           pointer :: core
      type(grid_warehouse_t), pointer :: grid_warehouse

      call c_f_pointer(tuvx, core)
      grid_warehouse => core%get_grid_warehouse()

      grid_map_ptr = c_loc(grid_warehouse)

   end function internal_get_grid_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function internal_get_profile_map(tuvx, error_code) result(profile_map_ptr) &
      bind(C, name="InternalGetProfileMap")
      use iso_c_binding, only: c_ptr, c_f_pointer, c_int

      ! arguments
      type(c_ptr), value,  intent(in)  :: tuvx
      integer(kind=c_int), intent(out) :: error_code

      ! result
      type(c_ptr) :: profile_map_ptr

      ! variables
      type(core_t),              pointer :: core
      type(profile_warehouse_t), pointer :: profile_warehouse

      call c_f_pointer(tuvx, core)
      profile_warehouse => core%get_profile_warehouse()

      profile_map_ptr = c_loc(profile_warehouse)

   end function internal_get_profile_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function internal_get_radiator_map(tuvx, error_code) result(radiator_map_ptr) &
      bind(C, name="InternalGetRadiatorMap")
      use iso_c_binding, only: c_ptr, c_f_pointer, c_int

      ! arguments
      type(c_ptr), value,  intent(in)  :: tuvx
      integer(kind=c_int), intent(out) :: error_code

      ! result
      type(c_ptr) :: radiator_map_ptr

      ! variables
      type(core_t),               pointer :: core
      type(radiator_warehouse_t), pointer :: radiator_warehouse

      call c_f_pointer(tuvx, core)
      radiator_warehouse => core%get_radiator_warehouse()

      radiator_map_ptr = c_loc(radiator_warehouse)

   end function internal_get_radiator_map

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function internal_get_photolysis_rate_constants_ordering(tuvx, error_code) &
      result(photolysis_rate_constant_ordering) &
      bind(C, name="InternalGetPhotolysisRateConstantsOrdering")
      use iso_c_binding, only: c_ptr, c_f_pointer, c_int
      use tuvx_interface_util, only: create_string_t_c, mappings_t_c, &
         mapping_t_c, allocate_mappings_c

      ! arguments
      type(c_ptr), value,  intent(in)  :: tuvx
      integer(kind=c_int), intent(out) :: error_code

      ! result
      type(mappings_t_c) :: photolysis_rate_constant_ordering

      ! variables
      type(core_t), pointer :: core
      type(string_t), allocatable :: labels(:)
      type(mapping_t_c), pointer :: mappings(:)
      type(c_ptr) :: mappings_ptr
      integer :: i, n_labels

      error_code = 0
      call c_f_pointer(tuvx, core)

      labels = core%photolysis_reaction_labels()
      n_labels = size(labels)
      mappings_ptr = allocate_mappings_c(int(n_labels, kind=c_size_t))
      call c_f_pointer(mappings_ptr, mappings, [ n_labels ])
      do i = 1, n_labels
         mappings(i)%name_ = create_string_t_c(labels(i)%val_)
         mappings(i)%index_ = int(i-1, kind=c_size_t)
      end do

      photolysis_rate_constant_ordering%mappings_ = c_loc(mappings)
      photolysis_rate_constant_ordering%size_ = n_labels

   end function internal_get_photolysis_rate_constants_ordering

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function internal_get_heating_rates_ordering(tuvx, error_code) &
      result(heating_rates_ordering) &
      bind(C, name="InternalGetHeatingRatesOrdering")
      use iso_c_binding, only: c_ptr, c_f_pointer, c_int
      use tuvx_interface_util, only: create_string_t_c, mappings_t_c, &
         mapping_t_c, allocate_mappings_c

      ! arguments
      type(c_ptr), value,  intent(in)  :: tuvx
      integer(kind=c_int), intent(out) :: error_code

      ! result
      type(mappings_t_c) :: heating_rates_ordering

      ! variables
      type(core_t), pointer :: core
      type(string_t), allocatable :: labels(:)
      type(mapping_t_c), pointer :: mappings(:)
      type(c_ptr) :: mappings_ptr
      integer :: i, n_labels

      error_code = 0
      call c_f_pointer(tuvx, core)

      labels = core%heating_rate_labels()
      n_labels = size(labels)
      mappings_ptr = allocate_mappings_c(int(n_labels, kind=c_size_t))
      call c_f_pointer(mappings_ptr, mappings, [ n_labels ])
      do i = 1, n_labels
         mappings(i)%name_ = create_string_t_c(labels(i)%val_)
         mappings(i)%index_ = int(i-1, kind=c_size_t)
      end do

      heating_rates_ordering%mappings_ = c_loc(mappings)
      heating_rates_ordering%size_ = n_labels

   end function internal_get_heating_rates_ordering

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_run_tuvx(tuvx, number_of_layers, solar_zenith_angle, &
      earth_sun_distance, photolysis_rate_constants, heating_rates, error_code) &
      bind(C, name="InternalRunTuvx")
      use iso_c_binding, only: c_ptr, c_f_pointer, c_int
      use musica_constants, only: dk => musica_dk

      ! arguments
      type(c_ptr),         value,  intent(in)  :: tuvx
      integer(kind=c_int), value,  intent(in)  :: number_of_layers
      real(kind=dk),       value,  intent(in)  :: solar_zenith_angle         ! degrees
      real(kind=dk),       value,  intent(in)  :: earth_sun_distance         ! AU
      type(c_ptr),         value,  intent(in)  :: photolysis_rate_constants  ! s^-1 (layer, reaction)
      type(c_ptr),         value,  intent(in)  :: heating_rates              ! K s^-1 (layer, reaction)
      integer(kind=c_int),         intent(out) :: error_code

      ! variables
      type(core_t), pointer :: core
      real(kind=dk), pointer :: photo_rates(:,:), heat_rates(:,:)

      call c_f_pointer(tuvx, core)
      call c_f_pointer(photolysis_rate_constants, photo_rates, &
         [number_of_layers + 1, core%number_of_photolysis_reactions()])
      call c_f_pointer(heating_rates, heat_rates, &
         [number_of_layers + 1, core%number_of_heating_rates()])
      call core%run(solar_zenith_angle, earth_sun_distance, &
         photolysis_rate_constants = photo_rates, &
         heating_rates = heat_rates, &
         diagnostic_label = "musica_tuvx_interface")
      error_code = 0

   end subroutine internal_run_tuvx

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_get_tuvx_version(version_ptr, version_length) &
      bind(C, name="InternalGetTuvxVersion")
      use iso_c_binding, only: c_ptr, c_char, c_int, c_f_pointer, c_null_char, c_loc
      use tuvx_version, only: get_tuvx_version

      ! arguments
      type(c_ptr),     intent(out) :: version_ptr
      integer(c_int),  intent(out) :: version_length

      ! local variables
      character(len=:),       allocatable :: version_fortran
      character(kind=c_char), pointer     :: version_string_ptr(:)
      integer :: i

      version_fortran = get_tuvx_version()
      version_length = len_trim(version_fortran)

      ! Allocate and copy string
      allocate(version_string_ptr(version_length + 1))
      do i = 1, version_length
         version_string_ptr(i) = version_fortran(i:i)
      end do
      version_string_ptr(version_length + 1) = c_null_char

      version_ptr = c_loc(version_string_ptr)

   end subroutine internal_get_tuvx_version

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine internal_free_tuvx_version(version_ptr, version_length) &
      bind(C, name="InternalFreeTuvxVersion")
      use iso_c_binding, only: c_ptr, c_int, c_associated, c_f_pointer, c_char

      ! arguments
      type(c_ptr),    value, intent(in) :: version_ptr
      integer(c_int), value, intent(in) :: version_length
      character(kind=c_char), pointer :: version_string_ptr(:)

      ! Free the allocated version string pointer
      if (c_associated(version_ptr)) then
         call c_f_pointer(version_ptr, version_string_ptr, [version_length + 1])
         deallocate(version_string_ptr)
      end if

   end subroutine internal_free_tuvx_version

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function create_tuvx_from_config_c(c_config_path, config_path_length, error_code) &
      bind(C, name="InternalCreateTuvxFromConfig")
      use iso_c_binding, only: c_ptr, c_f_pointer
      use musica_config, only: config_t

      ! arguments
      character(kind=c_char), dimension(*), intent(in)  :: c_config_path
      integer(kind=c_size_t), value                     :: config_path_length
      integer(kind=c_int),                  intent(out) :: error_code
      type(c_ptr)                                       :: create_tuvx_from_config_c

      ! local variables
      character(len=:), allocatable :: f_config_path
      type(core_t), pointer         :: core
      type(string_t)                :: musica_config_path
      integer                       :: i

      allocate(character(len=config_path_length) :: f_config_path)
      do i = 1, config_path_length
         f_config_path(i:i) = c_config_path(i)
      end do

      musica_config_path = f_config_path

      core => core_t(musica_config_path)
      create_tuvx_from_config_c = c_loc(core)
      error_code = 0

   end function create_tuvx_from_config_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine run_tuvx_c(tuvx, photolysis_rates, heating_rates, error_code) &
      bind(C, name="InternalRunTuvxFromConfig")
      use iso_c_binding, only: c_ptr, c_f_pointer
      use musica_constants, only: dk => musica_dk
      use tuvx_grid, only: grid_t
      use tuvx_profile, only: profile_t

      ! arguments
      type(c_ptr), value,      intent(in)  :: tuvx
      type(c_ptr), value,      intent(in)  :: photolysis_rates   ! (sza_step, layer, reaction)
      type(c_ptr), value,      intent(in)  :: heating_rates      ! (sza_step, layer, heating_type)
      integer(kind=c_int),     intent(out) :: error_code

      ! variables
      type(core_t), pointer :: core
      real(kind=dk), pointer :: photo_rates(:,:,:), heat_rates(:,:,:)
      class(grid_t), pointer :: height
      class(profile_t), pointer :: sza, earth_sun_distance
      integer :: i_sza, n_layers, n_sza_steps
      character(len=2) :: diagnostic_label

      call c_f_pointer(tuvx, core)

      ! Get grids and profiles from the configuration
      height => core%get_grid("height", "km")
      sza => core%get_profile("solar zenith angle", "degrees")
      earth_sun_distance => core%get_profile("Earth-Sun distance", "AU")

      n_layers = height%ncells_ + 1
      n_sza_steps = sza%ncells_ + 1

      call c_f_pointer(photolysis_rates, photo_rates, &
         [n_sza_steps, n_layers, core%number_of_photolysis_reactions()])
      call c_f_pointer(heating_rates, heat_rates, &
         [n_sza_steps, n_layers, core%number_of_heating_rates()])

      ! Run TUV-x for each solar zenith angle step (like in the Fortran driver)
      do i_sza = 1, n_sza_steps
         write(diagnostic_label,'(i2.2)') i_sza
         call core%run(sza%edge_val_(i_sza), &
            earth_sun_distance%edge_val_(i_sza), &
            photolysis_rate_constants = photo_rates(i_sza, :, :), &
            heating_rates = heat_rates(i_sza, :, :), &
            diagnostic_label = diagnostic_label)
      end do

      deallocate(height)
      deallocate(sza)
      deallocate(earth_sun_distance)
      error_code = 0

   end subroutine run_tuvx_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function get_photolysis_rate_count_c(tuvx, error_code) bind(C, name="InternalGetPhotolysisRateCount")
      use iso_c_binding, only: c_ptr, c_f_pointer

      ! arguments
      type(c_ptr), value,      intent(in)  :: tuvx
      integer(kind=c_int),     intent(out) :: error_code
      integer(kind=c_int)                  :: get_photolysis_rate_count_c

      ! variables
      type(core_t), pointer :: core

      call c_f_pointer(tuvx, core)
      get_photolysis_rate_count_c = core%number_of_photolysis_reactions()
      error_code = 0

   end function get_photolysis_rate_count_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function get_heating_rate_count_c(tuvx, error_code) bind(C, name="InternalGetHeatingRateCount")
      use iso_c_binding, only: c_ptr, c_f_pointer

      ! arguments
      type(c_ptr), value,      intent(in)  :: tuvx
      integer(kind=c_int),     intent(out) :: error_code
      integer(kind=c_int)                  :: get_heating_rate_count_c

      ! variables
      type(core_t), pointer :: core

      call c_f_pointer(tuvx, core)
      get_heating_rate_count_c = core%number_of_heating_rates()
      error_code = 0

   end function get_heating_rate_count_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function get_number_of_layers_c(tuvx, error_code) bind(C, name="InternalGetNumberOfLayers")
      use iso_c_binding, only: c_ptr, c_f_pointer
      use tuvx_grid, only: grid_t

      ! arguments
      type(c_ptr), value,      intent(in)  :: tuvx
      integer(kind=c_int),     intent(out) :: error_code
      integer(kind=c_int)                  :: get_number_of_layers_c

      ! variables
      type(core_t), pointer :: core
      class(grid_t), pointer :: height

      call c_f_pointer(tuvx, core)

      height => core%get_grid("height", "km")
      get_number_of_layers_c = height%ncells_ + 1
      deallocate(height)
      error_code = 0

   end function get_number_of_layers_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   function get_number_of_sza_steps_c(tuvx, error_code) bind(C, name="InternalGetNumberOfSzaSteps")
      use iso_c_binding, only: c_ptr, c_f_pointer
      use tuvx_profile, only: profile_t

      ! arguments
      type(c_ptr), value,      intent(in)  :: tuvx
      integer(kind=c_int),     intent(out) :: error_code
      integer(kind=c_int)                  :: get_number_of_sza_steps_c

      ! variables
      type(core_t), pointer :: core
      class(profile_t), pointer :: sza

      call c_f_pointer(tuvx, core)
      sza => core%get_profile("solar zenith angle", "degrees")
      get_number_of_sza_steps_c = sza%ncells_ + 1
      deallocate(sza)
      error_code = 0

   end function get_number_of_sza_steps_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine get_photolysis_rate_names_c(tuvx, names, error_code) &
      bind(C, name="InternalGetPhotolysisRateNames")
      use iso_c_binding, only: c_ptr, c_f_pointer
      use musica_string, only: string_t

      ! arguments
      type(c_ptr), value,      intent(in)  :: tuvx
      type(c_ptr), value,      intent(in)  :: names
      integer(kind=c_int),     intent(out) :: error_code

      ! variables
      type(core_t), pointer :: core
      type(string_t), allocatable :: labels(:)
      character(kind=c_char), pointer :: c_names(:)
      integer :: i, n_labels, name_length

      call c_f_pointer(tuvx, core)
      labels = core%photolysis_reaction_labels()
      n_labels = size(labels)

      ! Note: This is a simplified implementation
      ! In practice, you would need to properly allocate and manage the string array
      ! and handle the conversion from Fortran strings to C strings
      error_code = 0

   end subroutine get_photolysis_rate_names_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   subroutine get_heating_rate_names_c(tuvx, names, error_code) &
      bind(C, name="InternalGetHeatingRateNames")
      use iso_c_binding, only: c_ptr, c_f_pointer
      use musica_string, only: string_t

      ! arguments
      type(c_ptr), value,      intent(in)  :: tuvx
      type(c_ptr), value,      intent(in)  :: names
      integer(kind=c_int),     intent(out) :: error_code

      ! variables
      type(core_t), pointer :: core
      type(string_t), allocatable :: labels(:)
      character(kind=c_char), pointer :: c_names(:)
      integer :: i, n_labels, name_length

      call c_f_pointer(tuvx, core)
      labels = core%heating_rate_labels()
      n_labels = size(labels)

      ! Note: This is a simplified implementation
      ! In practice, you would need to properly allocate and manage the string array
      ! and handle the conversion from Fortran strings to C strings
      error_code = 0

   end subroutine get_heating_rate_names_c

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

end module tuvx_interface
