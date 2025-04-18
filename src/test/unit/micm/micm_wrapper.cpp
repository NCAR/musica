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
  micm.Solve(&state, time_step, &solver_state, &solver_stats);
  EXPECT_EQ(std::string(solver_state.value_), std::string("Converged"));
  bool something_changed = false;
  for(int i = 0; i < initial_concnetrations.size(); ++i)
  {
    if (state.GetOrderedConcentrations()[i] != initial_concnetrations[i])
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

#ifdef MUSICA_ENABLE_CUDA
TEST(MICMWrapper, CudaRosenbrock)
{
  DoChemistry(musica::MICMSolver::CudaRosenbrock);
}
#endif