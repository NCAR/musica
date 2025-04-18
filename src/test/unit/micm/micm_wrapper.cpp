#include <musica/micm/micm.hpp>
#include <musica/micm/parse.hpp>

#include <gtest/gtest.h>

#include <iostream>

TEST(MICMWrapper, CanCreateRosenbrock)
{
  musica::Chemistry chemistry = musica::ReadConfiguration("configs/chapman");
  musica::MICM micm = musica::MICM(chemistry, musica::MICMSolver::Rosenbrock, 1);
}

TEST(MICMWrapper, CanCreateRosenbrockStandardOrder)
{
  musica::Chemistry chemistry = musica::ReadConfiguration("configs/chapman");
  musica::MICM micm = musica::MICM(chemistry, musica::MICMSolver::RosenbrockStandardOrder, 1);
}

TEST(MICMWrapper, CanCreateBackwardEuler)
{
  musica::Chemistry chemistry = musica::ReadConfiguration("configs/chapman");
  musica::MICM micm = musica::MICM(chemistry, musica::MICMSolver::BackwardEuler, 1);
}

TEST(MICMWrapper, CanCreateBackwardEulerStandardOrder)
{
  musica::Chemistry chemistry = musica::ReadConfiguration("configs/chapman");
  musica::MICM micm = musica::MICM(chemistry, musica::MICMSolver::BackwardEulerStandardOrder, 1);
}

#ifdef MUSICA_ENABLE_CUDA
TEST(MICMWrapper, CanCreateCudaRosenbrock)
{
    musica::Chemistry chemistry = musica::ReadConfiguration("configs/chapman");
    musica::MICM micm = musica::MICM(chemistry, musica::MICMSolver::CudaRosenbrock, 1);
    musica::State state = musica::State(micm);
}
#endif