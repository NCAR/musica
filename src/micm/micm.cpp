// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the MICM class, which represents a
// multi-component reactive transport model. It also includes functions for
// creating and deleting MICM instances, creating solvers, and solving the model.
#include <musica/micm/cpu_solver.hpp>
#include <musica/micm/cuda_loader.hpp>
#include <musica/micm/lambda_callback.hpp>
#include <musica/micm/micm.hpp>
#include <musica/micm/parse.hpp>
#include <musica/micm/state.hpp>

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <string>
#include <system_error>

namespace musica
{
  std::string ToString(MICMSolver solver_type)
  {
    switch (solver_type)
    {
      case UndefinedSolver: return "UndefinedSolver";
      case Rosenbrock: return "Rosenbrock";
      case RosenbrockStandardOrder: return "RosenbrockStandardOrder";
      case BackwardEuler: return "BackwardEuler";
      case BackwardEulerStandardOrder: return "BackwardEulerStandardOrder";
      case CudaRosenbrock: return "CudaRosenbrock";
      default: throw std::system_error(make_error_code(MusicaErrCode::Unknown), "Unknown solver type");
    }
  }

  MICM::MICM(std::string config_path, MICMSolver solver_type)
      : MICM(ReadConfiguration(config_path), solver_type)
  {
  }

  MICM::MICM(const Chemistry& chemistry, MICMSolver solver_type, const RosenbrockSolverParameters& params)
      : MICM(chemistry, solver_type)
  {
    SetSolverParameters(params);
  }

  MICM::MICM(std::string config_path, MICMSolver solver_type, const RosenbrockSolverParameters& params)
      : MICM(ReadConfiguration(config_path), solver_type, params)
  {
  }

  MICM::MICM(const Chemistry& chemistry, MICMSolver solver_type, const BackwardEulerSolverParameters& params)
      : MICM(chemistry, solver_type)
  {
    SetSolverParameters(params);
  }

  MICM::MICM(std::string config_path, MICMSolver solver_type, const BackwardEulerSolverParameters& params)
      : MICM(ReadConfiguration(config_path), solver_type, params)
  {
  }

  MICM::MICM(const Chemistry& chemistry, MICMSolver solver_type)
      : solver_type_(solver_type)
  {
    // Default deleter for CPU solvers (just delete)
    auto default_deleter = [](IMicmSolver* ptr) { delete ptr; };

    switch (solver_type)
    {
      case MICMSolver::Rosenbrock:
      case MICMSolver::RosenbrockStandardOrder:
      case MICMSolver::BackwardEuler:
      case MICMSolver::BackwardEulerStandardOrder:
        // Create CPU solver with default deleter
        solver_ = SolverPtr(new CpuSolver(chemistry, static_cast<int>(solver_type)), default_deleter);
        break;

      case MICMSolver::CudaRosenbrock:
      {
        // Try to create CUDA solver via runtime loading
        auto& cuda_loader = CudaLoader::GetInstance();
        if (cuda_loader.IsAvailable() && cuda_loader.HasDevices())
        {
          // The CudaLoader returns a CudaSolverPtr with custom deleter
          // We need to transfer ownership to our SolverPtr
          auto cuda_solver = cuda_loader.CreateRosenbrockSolver(chemistry);
          auto deleter = cuda_solver.get_deleter();
          solver_ = SolverPtr(cuda_solver.release(), [deleter](IMicmSolver* ptr) { deleter(ptr); });
        }
        else
        {
          std::string msg = "CUDA solver requested but not available";
          if (!cuda_loader.GetLastError().empty())
          {
            msg += ": " + cuda_loader.GetLastError();
          }
          throw std::system_error(make_error_code(MusicaErrCode::SolverTypeNotFound), msg);
        }
        break;
      }

      default:
        std::string const msg = "Solver type " + ToString(solver_type) + " not supported";
        throw std::system_error(make_error_code(MusicaErrCode::SolverTypeNotFound), msg);
    }
  }

  MICM::~MICM()
  {
    // If using CUDA, ensure proper cleanup order
    if (solver_type_ == MICMSolver::CudaRosenbrock)
    {
      // Reset the solver first to trigger its destructor
      solver_.reset();
      // Then clean up CUDA resources
      CudaLoader::GetInstance().CleanUp();
    }
  }

  micm::SolverResult MICM::Solve(musica::State* state, double time_step)
  {
    return solver_->Solve(state->GetStateInterface(), time_step);
  }

  std::size_t MICM::GetMaximumNumberOfGridCells()
  {
    return solver_->MaximumNumberOfGridCells();
  }

  std::unique_ptr<IState> MICM::CreateState(std::size_t number_of_grid_cells)
  {
    return solver_->CreateState(number_of_grid_cells);
  }

  micm::System MICM::GetSystem() const
  {
    return solver_->GetSystem();
  }

  std::unordered_map<std::string, std::size_t> MICM::GetSpeciesOrdering() const
  {
    return solver_->GetSpeciesOrdering();
  }

  std::unordered_map<std::string, std::size_t> MICM::GetRateParameterOrdering() const
  {
    return solver_->GetRateParameterOrdering();
  }

  MICMSolver MICM::GetSolverType() const
  {
    return solver_type_;
  }

  std::size_t MICM::GetVectorSize() const
  {
    return solver_->GetVectorSize();
  }

  IMicmSolver* MICM::GetSolverInterface()
  {
    return solver_.get();
  }

  void MICM::SetSolverParameters(const RosenbrockSolverParameters& params)
  {
    solver_->SetRosenbrockSolverParameters(params);
  }

  void MICM::SetSolverParameters(const BackwardEulerSolverParameters& params)
  {
    solver_->SetBackwardEulerSolverParameters(params);
  }

  RosenbrockSolverParameters MICM::GetRosenbrockSolverParameters() const
  {
    return solver_->GetRosenbrockSolverParameters();
  }

  BackwardEulerSolverParameters MICM::GetBackwardEulerSolverParameters() const
  {
    return solver_->GetBackwardEulerSolverParameters();
  }

  void MICM::SetLambdaRateCallback(
    const std::string& label,
    int callback_id)
  {
    GetLambdaCallbackIds()[label] = callback_id;
  }

}  // namespace musica
