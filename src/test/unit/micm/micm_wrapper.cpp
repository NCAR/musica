#include <musica/micm/micm.hpp>
#include <musica/micm/parse.hpp>

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

#ifdef MUSICA_ENABLE_CUDA
TEST(MICMWrapper, CudaRosenbrock)
{
  DoChemistry(musica::MICMSolver::CudaRosenbrock);
}
#endif
