program test_micm_box_model

  use, intrinsic :: iso_c_binding
  use, intrinsic :: ieee_arithmetic

  use musica_util, only: error_t, string_t, mapping_t
  use musica_micm, only: micm_t, solver_stats_t
  use musica_micm, only: Rosenbrock, RosenbrockStandardOrder

  implicit none

  call box_model()

contains

  subroutine box_model()

    character(len=256) :: config_path
    integer(c_int)     :: solver_type
    integer(c_int)     :: num_grid_cells

    real(c_double), parameter :: GAS_CONSTANT = 8.31446261815324_c_double ! J mol-1 K-1

    real(c_double) :: time_step
    real(c_double), target :: temperature(1)
    real(c_double), target :: pressure(1)
    real(c_double), target :: air_density(1)
    real(c_double), target :: concentrations(6)
    real(c_double), target :: user_defined_reaction_rates(2)

    type(string_t)       :: solver_state
    type(solver_stats_t) :: solver_stats
    type(error_t)        :: error

    type(micm_t), pointer :: micm

    integer :: i

    config_path = "configs/analytical"
    solver_type = RosenbrockStandardOrder
    num_grid_cells = 1

    time_step = 200
    temperature(1) = 273.0
    pressure(1) = 1.0e5
    air_density(:) = pressure(:) / (GAS_CONSTANT * temperature(:))

    concentrations = (/ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 /)
    user_defined_reaction_rates = (/ 0.001, 0.002 /)

    write(*,*) "Creating MICM solver..."
    micm => micm_t(config_path, solver_type, num_grid_cells, error)

    do i = 1, micm%species_ordering%size()
      print *, "Species Name:", micm%species_ordering%name(i), &
               ", Index:", micm%species_ordering%index(i)
    end do

    write(*,*) "Solving starts..."
    call micm%solve(time_step, temperature, pressure, air_density, concentrations, &
        user_defined_reaction_rates, solver_state, solver_stats, error)
    write(*,*) "After solving, concentrations", concentrations

    deallocate( micm )

  end subroutine box_model

end program test_micm_box_model
