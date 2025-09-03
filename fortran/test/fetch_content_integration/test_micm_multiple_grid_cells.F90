program test_micm_multiple_grid_cells

  use, intrinsic :: iso_c_binding
  use, intrinsic :: ieee_arithmetic

  use iso_fortran_env, only : real64
  use musica_util, only: assert, error_t, string_t, mapping_t
  use musica_micm, only: micm_t, solver_stats_t
  use musica_micm, only: Rosenbrock, RosenbrockStandardOrder
  use musica_state, only: conditions_t, state_t

#define ASSERT( expr ) call assert( expr, __FILE__, __LINE__ )
#define ASSERT_EQ( a, b ) call assert( a == b, __FILE__, __LINE__ )
#define ASSERT_NE( a, b ) call assert( a /= b, __FILE__, __LINE__ )
#define ASSERT_NEAR( a, b, tol ) call assert( abs(a - b) < abs(a + b) * tol, __FILE__, __LINE__ )
#define ASSERT_GT( a, b ) call assert( a > b, __FILE__, __LINE__ )

  implicit none

  call multiple_grid_cells_example()

contains

  !> Runs a multiple grid cells box model using the MICM solver
  subroutine multiple_grid_cells_example()

    character(len=256)      :: config_path
    integer                 :: solver_type
    integer                 :: num_grid_cells
    real(real64), parameter :: GAS_CONSTANT = 8.31446261815324_real64 ! J mol-1 K-1
    real(real64)            :: time_step
    type(string_t)          :: solver_state
    type(solver_stats_t)    :: solver_stats
    type(error_t)           :: error
    type(micm_t), pointer   :: micm
    type(state_t), pointer  :: state
    integer                 :: i, j, cell_id

    config_path = "configs/v0/analytical"
    solver_type = RosenbrockStandardOrder
    num_grid_cells = 3  ! Use 3 grid cells to demonstrate multiple cells

    write(*,*) "Creating MICM solver with", num_grid_cells, "grid cells..."
    micm => micm_t(config_path, solver_type, error)
    ASSERT( error%is_success() )

    write(*,*) "Creating State for multiple grid cells..."    
    state => micm%get_state(num_grid_cells, error)
    ASSERT( error%is_success() )

    time_step = 200

    ! Set up conditions for each grid cell
    do cell_id = 1, num_grid_cells
      state%conditions(cell_id)%temperature = 273.0 + (cell_id - 1) * 10.0  ! Different temperatures
      state%conditions(cell_id)%pressure    = 1.0e5
      state%conditions(cell_id)%air_density = state%conditions(cell_id)%pressure / &
                                               (GAS_CONSTANT * state%conditions(cell_id)%temperature)
    end do

    ! Set initial concentrations for each grid cell
    associate( cell_stride => state%species_strides%grid_cell, &
               var_stride => state%species_strides%variable )
      do cell_id = 1, num_grid_cells
        ! Set different initial concentrations for each cell
        do i = 1, 6  ! Assuming 6 species
          ! Cell 1: all species start at 1.0
          ! Cell 2: all species start at 2.0  
          ! Cell 3: all species start at 0.5
          if (cell_id == 1) then
            state%concentrations(1 + (cell_id - 1) * cell_stride + (i - 1) * var_stride) = 1.0
          else if (cell_id == 2) then
            state%concentrations(1 + (cell_id - 1) * cell_stride + (i - 1) * var_stride) = 2.0
          else
            state%concentrations(1 + (cell_id - 1) * cell_stride + (i - 1) * var_stride) = 0.5
          end if
        end do
      end do
    end associate

    ! Set rate parameters for each grid cell
    associate( cell_stride => state%rate_parameters_strides%grid_cell, &
               var_stride => state%rate_parameters_strides%variable )
      do cell_id = 1, num_grid_cells
        ! Set the same rate parameters for all cells
        state%rate_parameters(1 + (cell_id - 1) * cell_stride + 0 * var_stride) = 0.001
        state%rate_parameters(1 + (cell_id - 1) * cell_stride + 1 * var_stride) = 0.002
      end do
    end associate
    
    ! Print species information
    write(*,*) "Species in the mechanism:"
    do i = 1, state%species_ordering%size()
      write(*,*) "Species Name:", trim(state%species_ordering%name(i)), &
                 ", Index:", state%species_ordering%index(i)
    end do

    ! Print initial concentrations for each grid cell
    write(*,*) ""
    write(*,*) "Initial concentrations by grid cell:"
    associate( cell_stride => state%species_strides%grid_cell, &
               var_stride => state%species_strides%variable )
      do cell_id = 1, num_grid_cells
        write(*,'(A,I0,A,F6.1,A)') "Grid Cell ", cell_id, " (T=", &
               state%conditions(cell_id)%temperature, "K):"
        do i = 1, 6
          write(*,'(A,F8.3)', advance='no') "  ", &
                 state%concentrations(1 + (cell_id - 1) * cell_stride + (i - 1) * var_stride)
        end do
        write(*,*) ""
      end do
    end associate

    write(*,*) ""
    write(*,*) "Solving for all grid cells simultaneously..."
    call micm%solve(time_step, state, solver_state, solver_stats, error)
    ASSERT( error%is_success() )

    ! Print final concentrations for each grid cell
    write(*,*) ""
    write(*,*) "Final concentrations by grid cell:"
    associate( cell_stride => state%species_strides%grid_cell, &
               var_stride => state%species_strides%variable )
      do cell_id = 1, num_grid_cells
        write(*,'(A,I0,A,F6.1,A)') "Grid Cell ", cell_id, " (T=", &
               state%conditions(cell_id)%temperature, "K):"
        do i = 1, 6
          write(*,'(A,F8.3)', advance='no') "  ", &
                 state%concentrations(1 + (cell_id - 1) * cell_stride + (i - 1) * var_stride)
        end do
        write(*,*) ""
      end do
    end associate

    write(*,*) ""
    write(*,*) "Solver completed successfully for all", num_grid_cells, "grid cells!"

    deallocate( micm )
    deallocate( state )
  end subroutine multiple_grid_cells_example

end program test_micm_multiple_grid_cells