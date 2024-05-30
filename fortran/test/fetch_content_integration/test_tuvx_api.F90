program combined_tuvx_tests
  use iso_c_binding
  use musica_tuvx, only: tuvx_t
  use musica_util, only: assert

  implicit none

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )

  ! Declarations
  type(tuvx_t), pointer         :: tuvx
  integer                       :: errcode

  ! Call the valid test subroutine
  call test_tuvx_fort_api()

  ! Call the invalid test subroutine
  call test_tuvx_fort_api_invalid()

contains

  ! Valid tuvx solver creation test
  subroutine test_tuvx_fort_api()
    character(len=256) :: config_path
    logical(c_bool)    :: bool_value

    config_path = "examples/ts1_tsmlt.json"

    tuvx => tuvx_t(config_path, errcode)

  end subroutine test_tuvx_fort_api

  ! Invalid tuvx solver creation test
  subroutine test_tuvx_fort_api_invalid()
    character(len=7) :: config_path

    config_path = "invalid_config"

    tuvx => tuvx_t(config_path, errcode)

    if (errcode /= 0) then
      write(*,*) "[test tuvx fort api] Failed in creating solver (Expected failure). Error code: ", errcode
    else
      write(*,*) "[test tuvx fort api] Unexpected error code when creating solver with invalid config: ", errcode
      stop 3
    endif

  end subroutine test_tuvx_fort_api_invalid

  subroutine test_tuvx_solve()

    type(tuvx_t), pointer :: tuvx
    type(grid_map_t) :: grids
    type(profile_map_t) :: profiles
    type(radiator_map_t) :: radiators
    type(grid_t), pointer :: grid
    real(dk), allocatable :: photo_rates(:,:,:)

    tuvx => tuvx_t( config_path )
    grids = tuvx%get_grids( )
    profiles = tuvx%create_profiles( )
    radiators = tuvx%create_radiators( )

    ! update conditions for each time step
    grid => grids%get( "height", "m" )
    grid%edges_(:) = some_input_array(:)
    grid%mid_points_(:) = more_input_data(:)
    
    profile => profiles%get( "O3", "mol m-3" )
    ...


    call tuvx%solve( grids, profiles, radiators, photo_rates )


  end subroutine test_tuvx_solve

end program combined_tuvx_tests
