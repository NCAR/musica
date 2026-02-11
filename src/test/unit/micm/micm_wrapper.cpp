#include <musica/micm/cuda_availability.hpp>
#include <musica/micm/micm.hpp>
#include <musica/micm/parse.hpp>
#include <musica/micm/solver_parameters.hpp>
#include <musica/micm/state.hpp>

#include <gtest/gtest.h>

#include <iostream>

void DoChemistry(musica::MICMSolver solver_type)
{
  musica::Chemistry const chemistry = musica::ReadConfiguration("configs/v0/analytical");
  musica::MICM micm = musica::MICM(chemistry, solver_type);
  musica::State state = musica::State(micm, 1);

  std::vector<double> initial_concentrations = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
  state.SetOrderedConcentrations(initial_concentrations);
  state.SetConditions({ { .temperature_ = 298.15, .pressure_ = 101325.0 } });

  double const time_step = 60;
  auto result = micm.Solve(&state, time_step);
  EXPECT_EQ(result.state_, micm::SolverState::Converged);
  bool something_changed = false;
  for (int i = 0; i < initial_concentrations.size(); ++i)
  {
    if (state.GetOrderedConcentrations()[i] != initial_concentrations[i])
    {
      something_changed = true;
      break;
    }
  }
  EXPECT_TRUE(something_changed);
}

TEST(MICMWrapper, Rosenbrock)
{
  DoChemistry(musica::MICMSolver::Rosenbrock);
}

TEST(MICMWrapper, RosenbrockStandardOrder)
{
  DoChemistry(musica::MICMSolver::RosenbrockStandardOrder);
}

TEST(MICMWrapper, BackwardEuler)
{
  DoChemistry(musica::MICMSolver::BackwardEuler);
}

TEST(MICMWrapper, BackwardEulerStandardOrder)
{
  DoChemistry(musica::MICMSolver::BackwardEulerStandardOrder);
}

TEST(MICMWrapper, CudaRosenbrock)
{
  // Skip if CUDA is not available at runtime
  if (!musica::IsCudaAvailable())
  {
    GTEST_SKIP() << "CUDA is not available";
  }
  DoChemistry(musica::MICMSolver::CudaRosenbrock);
}

// --- Solver parameter tests ---

TEST(SolverParameters, SetGetRosenbrockParameters)
{
  musica::Chemistry const chemistry = musica::ReadConfiguration("configs/v0/analytical");
  musica::MICM micm(chemistry, musica::MICMSolver::RosenbrockStandardOrder);

  musica::RosenbrockSolverParameters params;
  params.relative_tolerance = 1e-8;
  params.h_min = 1e-10;
  params.h_max = 100.0;
  params.h_start = 1e-5;
  params.max_number_of_steps = 500;

  micm.SetSolverParameters(params);

  auto result = micm.GetRosenbrockSolverParameters();
  EXPECT_DOUBLE_EQ(result.h_min, 1e-10);
  EXPECT_DOUBLE_EQ(result.h_max, 100.0);
  EXPECT_DOUBLE_EQ(result.h_start, 1e-5);
  EXPECT_EQ(result.max_number_of_steps, 500);
  EXPECT_DOUBLE_EQ(result.relative_tolerance, 1e-8);
}

TEST(SolverParameters, SetGetBackwardEulerParameters)
{
  musica::Chemistry const chemistry = musica::ReadConfiguration("configs/v0/analytical");
  musica::MICM micm(chemistry, musica::MICMSolver::BackwardEulerStandardOrder);

  musica::BackwardEulerSolverParameters params;
  params.relative_tolerance = 1e-8;
  params.max_number_of_steps = 20;
  params.time_step_reductions = { 0.3, 0.3, 0.3, 0.3, 0.05 };

  micm.SetSolverParameters(params);

  auto result = micm.GetBackwardEulerSolverParameters();
  EXPECT_EQ(result.max_number_of_steps, 20);
  EXPECT_DOUBLE_EQ(result.relative_tolerance, 1e-8);
  ASSERT_EQ(result.time_step_reductions.size(), 5);
  EXPECT_DOUBLE_EQ(result.time_step_reductions[0], 0.3);
  EXPECT_DOUBLE_EQ(result.time_step_reductions[4], 0.05);
}

TEST(SolverParameters, WrongParameterTypeThrows)
{
  musica::Chemistry const chemistry = musica::ReadConfiguration("configs/v0/analytical");
  musica::MICM micm(chemistry, musica::MICMSolver::RosenbrockStandardOrder);

  musica::BackwardEulerSolverParameters params;
  EXPECT_THROW(micm.SetSolverParameters(params), std::runtime_error);
}

TEST(SolverParameters, ConstructorWithRosenbrockParams)
{
  musica::Chemistry const chemistry = musica::ReadConfiguration("configs/v0/analytical");

  musica::RosenbrockSolverParameters params;
  params.h_start = 1e-3;
  params.max_number_of_steps = 2000;

  musica::MICM micm(chemistry, musica::MICMSolver::RosenbrockStandardOrder, params);

  auto result = micm.GetRosenbrockSolverParameters();
  EXPECT_DOUBLE_EQ(result.h_start, 1e-3);
  EXPECT_EQ(result.max_number_of_steps, 2000);
}

TEST(SolverParameters, TolerancesAppliedToNewState)
{
  musica::Chemistry const chemistry = musica::ReadConfiguration("configs/v0/analytical");
  musica::MICM micm(chemistry, musica::MICMSolver::RosenbrockStandardOrder);

  musica::RosenbrockSolverParameters params;
  params.relative_tolerance = 1e-10;

  micm.SetSolverParameters(params);

  // Create a state and solve - should succeed with the new tolerance
  musica::State state(micm, 1);
  std::vector<double> initial_concentrations = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
  state.SetOrderedConcentrations(initial_concentrations);
  state.SetConditions({ { .temperature_ = 298.15, .pressure_ = 101325.0 } });

  auto result = micm.Solve(&state, 60.0);
  EXPECT_EQ(result.state_, micm::SolverState::Converged);
}
