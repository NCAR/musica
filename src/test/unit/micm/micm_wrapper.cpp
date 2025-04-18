#include <musica/micm/micm.hpp>
#include <musica/micm/parse.hpp>

#include <gtest/gtest.h>

#include <iostream>

void DoChemistry(musica::MICMSolver solver_type)
{
  musica::Chemistry chemistry = musica::ReadConfiguration("configs/analytical");
  musica::MICM micm = musica::MICM(chemistry, solver_type, 1);
  musica::State state = musica::State(micm);

  std::vector<double> initial_concnetrations = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
  state.SetOrderedConcentrations(initial_concnetrations);
  state.SetConditions({ {.temperature_ = 298.15, .pressure_ = 101325.0 } });

  double time_step = 60;
  musica::SolverResultStats solver_stats;
  musica::String solver_state;
  micm.Solve(&micm, &state, time_step, &solver_state, &solver_stats);
  EXPECT_EQ(std::string(solver_state.value_), std::string("Converged"));
  EXPECT_NE(state.GetOrderedConcentrations()[0], initial_concnetrations[0]);
  EXPECT_NE(state.GetOrderedConcentrations()[1], initial_concnetrations[1]);
  EXPECT_NE(state.GetOrderedConcentrations()[2], initial_concnetrations[2]);
  EXPECT_NE(state.GetOrderedConcentrations()[3], initial_concnetrations[3]);
  EXPECT_NE(state.GetOrderedConcentrations()[4], initial_concnetrations[4]);
  EXPECT_NE(state.GetOrderedConcentrations()[5], initial_concnetrations[5]);
}

TEST(MICMWrapper, CanCreateRosenbrock)
{
  DoChemistry(musica::MICMSolver::Rosenbrock);
}

TEST(MICMWrapper, CanCreateRosenbrockStandardOrder)
{
  DoChemistry(musica::MICMSolver::RosenbrockStandardOrder);
}

TEST(MICMWrapper, CanCreateBackwardEuler)
{
  DoChemistry(musica::MICMSolver::BackwardEuler);
}

TEST(MICMWrapper, CanCreateBackwardEulerStandardOrder)
{
  DoChemistry(musica::MICMSolver::BackwardEulerStandardOrder);
}

#ifdef MUSICA_ENABLE_CUDA
TEST(MICMWrapper, CanCreateCudaRosenbrock)
{
  DoChemistry(musica::MICMSolver::CudaRosenbrock);
}
#endif