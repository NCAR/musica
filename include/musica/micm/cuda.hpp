#pragma once

#include <musica/micm/micm.hpp>
#include <musica/micm/state.hpp>
#ifdef MUSICA_ENABLE_CUDA
  #include <micm/GPU.hpp>
#endif

namespace musica
{
  bool IsCudaAvailable();

#ifdef MUSICA_ENABLE_CUDA

  class MICMCuda : public MICM
  {
   private:
      std::unique_ptr<micm::CudaRosenbrock> cuda_solver_ = nullptr;

   public:
    MICMCuda(const Chemistry &chemistry, MICMSolver solver_type)
        : MICM(chemistry, solver_type)
    {
      // Ensure the solver type is compatible with CUDA
      if (solver_type != MICMSolver::CudaRosenbrock)
      {
        throw std::system_error(make_error_code(MusicaErrCode::UnsupportedSolverStatePair),
                                "CUDA solver only supports CudaRosenbrock");
      }
    }

    ~MICMCuda() override
    {
      // Clean up CUDA resources
      // This must happen before the MICM destructor completes because
      // cuda must clean all of its runtime resources
      // Otherwise, we risk the CudaRosenbrock destructor running after
      // the cuda runtime has closed
      if (cuda_solver_)
      {
        cuda_solver_->Cleanup();
        micm::cuda::CudaStreamSingleton::GetInstance().CleanUp();
      }
    }

    /// @brief Solve the system
    /// @param state Pointer to state object
    /// @param time_step Time [s] to advance the state by
    /// @param solver_state State of the solver
    /// @param solver_stats Statistics of the solver
    void Solve(musica::State *state, double time_step, String *solver_state, SolverResultStats *solver_stats) override;
  };

  class StateCuda : public State
  {
    private:
      micm::GpuState gpu_state_;

   public:
    StateCuda(const musica::MICM &micm, std::size_t number_of_grid_cells)
        : State(micm, number_of_grid_cells)
    {
      // Ensure the MICM instance is a CUDA solver
      if (!std::holds_alternative<MICMCuda>(micm.solver_variant_))
      {
        throw std::system_error(make_error_code(MusicaErrCode::UnsupportedSolverStatePair),
                                "StateCuda only supports MICMCuda");
      }
    }

    // Additional CUDA-specific state methods can be added here
  };

#endif
}  // namespace musica