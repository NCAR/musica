// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file provides C-compatible factory functions for the CUDA plugin.
// These functions are loaded at runtime via dlopen/dlsym.
#include <musica/micm/chemistry.hpp>
#include <musica/micm/solver_interface.hpp>

#include <micm/GPU.hpp>

#include <cstring>
#include <exception>

// Include the solver declarations
#include "cuda_solver.hpp"

extern "C"
{
  /// @brief Create a CUDA Rosenbrock solver
  /// @param chemistry Pointer to the Chemistry configuration
  /// @param error_msg Buffer to receive error message on failure
  /// @param error_msg_size Size of the error message buffer
  /// @return Pointer to the created solver, or nullptr on failure
  musica::IMicmSolver*
  musica_cuda_create_rosenbrock(const musica::Chemistry* chemistry, char* error_msg, std::size_t error_msg_size)
  {
    try
    {
      if (!chemistry)
      {
        if (error_msg && error_msg_size > 0)
        {
          std::strncpy(error_msg, "Chemistry pointer is null", error_msg_size - 1);
          error_msg[error_msg_size - 1] = '\0';
        }
        return nullptr;
      }

      return new musica::cuda::CudaRosenbrockSolver(*chemistry);
    }
    catch (const std::exception& e)
    {
      if (error_msg && error_msg_size > 0)
      {
        std::strncpy(error_msg, e.what(), error_msg_size - 1);
        error_msg[error_msg_size - 1] = '\0';
      }
      return nullptr;
    }
    catch (...)
    {
      if (error_msg && error_msg_size > 0)
      {
        std::strncpy(error_msg, "Unknown error creating CUDA solver", error_msg_size - 1);
        error_msg[error_msg_size - 1] = '\0';
      }
      return nullptr;
    }
  }

  /// @brief Destroy a CUDA solver
  /// @param solver Pointer to the solver to destroy
  void musica_cuda_destroy_solver(musica::IMicmSolver* solver)
  {
    delete solver;
  }

  /// @brief Check if CUDA devices are available
  /// @return true if at least one CUDA device is available
  bool musica_cuda_devices_available()
  {
    int device_count = 0;
    cudaError_t error = cudaGetDeviceCount(&device_count);
    if (error != cudaSuccess)
    {
      return false;
    }
    return device_count > 0;
  }

  /// @brief Clean up CUDA resources
  /// Should be called before program exit
  void musica_cuda_cleanup()
  {
    micm::cuda::CudaStreamSingleton::GetInstance().CleanUp();
  }
}
