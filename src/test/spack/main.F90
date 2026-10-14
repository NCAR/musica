! Copyright (C) 2023-2026 University Corporation for Atmospheric Research
! SPDX-License-Identifier: Apache-2.0
!
! Links musica::musica-fortran from an installed prefix and calls into it.
!
! When built with MUSICA_TEST_MICM, this parses a mechanism and runs one solve,
! so that a missing or mis-ordered link dependency (yaml-cpp,
! mechanism_configuration, fmt, ...) fails here rather than in a downstream
! model.
!
! Usage: test_fortran [path to a v1 mechanism config.json]
program test_fortran

  use iso_fortran_env, only: real64, error_unit
  use musica_util, only: error_t, string_t
  use musica, only: get_musica_version
#ifdef MUSICA_TEST_MICM
  use musica_micm, only: micm_t, solver_stats_t, get_micm_version, &
                         RosenbrockStandardOrder
  use musica_state, only: state_t
#endif

  implicit none

  type(string_t) :: version

#ifdef MUSICA_TEST_MICM
  real(real64), parameter :: GAS_CONSTANT = 8.31446261815324_real64
  character(len=1024)     :: config_path
  type(string_t)          :: solver_state
  type(micm_t),  pointer  :: micm
  type(state_t), pointer  :: state
  type(solver_stats_t)    :: stats
  type(error_t)           :: error
  integer                 :: O2_index, O3_index, jO2_index, jO3a_index, jO3b_index
  real(real64)            :: o3_before, o3_after
#endif

  version = get_musica_version()
  print *, "MUSICA version: ", version%get_char_array()
  if (len_trim(version%get_char_array()) == 0) call fail("empty MUSICA version")

#ifdef MUSICA_TEST_MICM
  if (command_argument_count() /= 1) call fail("usage: test_fortran <config.json>")
  call get_command_argument(1, config_path)

  version = get_micm_version()
  if (len_trim(version%get_char_array()) == 0) call fail("empty MICM version")
  print *, "MICM version: ", trim(version%get_char_array())

  ! Parsing the mechanism exercises the yaml/mechanism_configuration libraries
  micm => micm_t(trim(config_path), RosenbrockStandardOrder, error)
  if (.not. error%is_success()) call fail("failed to parse mechanism " // trim(config_path))

  state => micm%get_state(1, error)
  if (.not. error%is_success()) call fail("failed to create state")

  O2_index = state%species_ordering%index("O2", error)
  if (.not. error%is_success()) call fail("species O2 not found in parsed mechanism")
  O3_index = state%species_ordering%index("O3", error)
  if (.not. error%is_success()) call fail("species O3 not found in parsed mechanism")
  jO2_index = state%rate_parameters_ordering%index("PHOTO.jo2_b", error)
  if (.not. error%is_success()) call fail("rate parameter PHOTO.jo2_b not found")
  jO3a_index = state%rate_parameters_ordering%index("PHOTO.jo3_a", error)
  if (.not. error%is_success()) call fail("rate parameter PHOTO.jo3_a not found")
  jO3b_index = state%rate_parameters_ordering%index("PHOTO.jo3_b", error)
  if (.not. error%is_success()) call fail("rate parameter PHOTO.jo3_b not found")

  state%conditions(1)%temperature = 272.5_real64
  state%conditions(1)%pressure    = 101253.4_real64
  state%conditions(1)%air_density = state%conditions(1)%pressure &
                                    / (GAS_CONSTANT * state%conditions(1)%temperature)

  state%concentrations(1 + (O2_index - 1) * state%species_strides%variable) = 0.75_real64
  state%concentrations(1 + (O3_index - 1) * state%species_strides%variable) = 8.1e-6_real64
  state%rate_parameters(1 + (jO2_index  - 1) * state%rate_parameters_strides%variable) = 2.7e-19_real64
  state%rate_parameters(1 + (jO3a_index - 1) * state%rate_parameters_strides%variable) = 1.13e-9_real64
  state%rate_parameters(1 + (jO3b_index - 1) * state%rate_parameters_strides%variable) = 5.8e-8_real64

  o3_before = state%concentrations(1 + (O3_index - 1) * state%species_strides%variable)

  call micm%solve(200.0_real64, state, solver_state, stats, error)
  if (.not. error%is_success()) call fail("solve failed")
  if (solver_state%get_char_array() /= "Converged") &
    call fail("solver did not converge: " // solver_state%get_char_array())

  o3_after = state%concentrations(1 + (O3_index - 1) * state%species_strides%variable)
  if (o3_after == o3_before) call fail("O3 did not change after solve")
  if (o3_after /= o3_after .or. o3_after < 0.0_real64) call fail("O3 is NaN or negative")

  print *, "O3 before/after: ", o3_before, o3_after
#endif

  print *, "PASS"

contains

  subroutine fail(message)
    character(len=*), intent(in) :: message
    write(error_unit, *) "FAIL: ", message
    error stop 1
  end subroutine fail

end program test_fortran
