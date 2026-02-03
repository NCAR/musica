// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the CudaLoader class for runtime CUDA solver loading.
// On Linux, it uses dlopen/dlsym to load libmusica_cuda.so at runtime.
// On other platforms or when CUDA is not available, it returns "not available".
#pragma once

#include <musica/micm/chemistry.hpp>
#include <musica/micm/solver_interface.hpp>

#include <functional>
#include <memory>
#include <string>

namespace musica
{
  /// @brief Custom deleter for CUDA solvers that uses the plugin's destroy function
  struct CudaSolverDeleter
  {
    using DestroyFunc = void (*)(IMicmSolver*);
    DestroyFunc destroy_func_ = nullptr;

    CudaSolverDeleter() = default;
    explicit CudaSolverDeleter(DestroyFunc func)
        : destroy_func_(func)
    {
    }

    void operator()(IMicmSolver* ptr) const
    {
      if (ptr && destroy_func_)
      {
        destroy_func_(ptr);
      }
    }
  };

  /// @brief Unique pointer type for CUDA solvers with custom deleter
  using CudaSolverPtr = std::unique_ptr<IMicmSolver, CudaSolverDeleter>;

  /// @brief Singleton class for runtime loading of CUDA solvers
  /// Uses dlopen/dlsym on Linux to load libmusica_cuda.so at runtime.
  /// This allows a single library to work both with and without CUDA.
  class CudaLoader
  {
   public:
    /// @brief Get the singleton instance
    static CudaLoader& GetInstance();

    // Disable copy and move
    CudaLoader(const CudaLoader&) = delete;
    CudaLoader& operator=(const CudaLoader&) = delete;
    CudaLoader(CudaLoader&&) = delete;
    CudaLoader& operator=(CudaLoader&&) = delete;

    /// @brief Check if the CUDA library was loaded successfully
    /// @return true if libmusica_cuda.so is loaded and functional
    bool IsAvailable() const;

    /// @brief Check if CUDA devices are available on this system
    /// @return true if CUDA devices are present and usable
    bool HasDevices() const;

    /// @brief Create a CUDA Rosenbrock solver
    /// @param chemistry The chemistry configuration
    /// @return Unique pointer to the created solver with custom deleter
    /// @throws std::runtime_error if CUDA is not available
    CudaSolverPtr CreateRosenbrockSolver(const Chemistry& chemistry);

    /// @brief Clean up CUDA resources
    /// Should be called before program exit when CUDA was used
    void CleanUp();

    /// @brief Get the last error message
    /// @return The last error message, or empty string if no error
    std::string GetLastError() const;

   private:
    CudaLoader();
    ~CudaLoader();

    void LoadLibrary();
    void UnloadLibrary();

    void* library_handle_;
    std::string last_error_;
    bool attempted_load_;

    // Function pointers for CUDA factory functions
    using AbiVersionFunc = int (*)();
    using CreateRosenbrockFunc = IMicmSolver* (*)(const Chemistry*, char*, std::size_t);
    using DestroySolverFunc = void (*)(IMicmSolver*);
    using DevicesAvailableFunc = bool (*)();
    using CleanUpFunc = void (*)();

    AbiVersionFunc abi_version_func_;
    CreateRosenbrockFunc create_rosenbrock_func_;
    DestroySolverFunc destroy_solver_func_;
    DevicesAvailableFunc devices_available_func_;
    CleanUpFunc cleanup_func_;
  };

}  // namespace musica
