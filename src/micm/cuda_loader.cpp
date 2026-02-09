// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file implements the CudaLoader class for runtime CUDA solver loading.
#include <musica/micm/cuda_loader.hpp>

#include <cstring>
#include <stdexcept>

// Only support dynamic loading on Linux
#if defined(__linux__) && !defined(__ANDROID__)
  #include <dlfcn.h>
  #define MUSICA_HAS_DLOPEN 1
#else
  #define MUSICA_HAS_DLOPEN 0
#endif

namespace musica
{
  CudaLoader& CudaLoader::GetInstance()
  {
    static CudaLoader instance;
    return instance;
  }

  CudaLoader::CudaLoader()
      : library_handle_(nullptr),
        create_rosenbrock_func_(nullptr),
        destroy_solver_func_(nullptr),
        devices_available_func_(nullptr),
        cleanup_func_(nullptr)
  {
  }

  CudaLoader::~CudaLoader()
  {
    UnloadLibrary();
  }

  void CudaLoader::LoadLibrary() const
  {
#if MUSICA_HAS_DLOPEN
    // Try to load libmusica_cuda.so from various locations
    const char* library_names[] = {
      "libmusica_cuda.so",       // System path or LD_LIBRARY_PATH
      "./libmusica_cuda.so",     // Current directory
      nullptr
    };

    for (const char** name = library_names; *name != nullptr; ++name)
    {
      library_handle_ = dlopen(*name, RTLD_NOW | RTLD_LOCAL);
      if (library_handle_)
      {
        break;
      }
    }

    if (!library_handle_)
    {
      last_error_ = "Could not load libmusica_cuda.so: ";
      last_error_ += dlerror();
      return;
    }

    // Clear any existing errors
    dlerror();

    // Load function pointers
    create_rosenbrock_func_ =
        reinterpret_cast<CreateRosenbrockFunc>(dlsym(library_handle_, "musica_cuda_create_rosenbrock"));
    destroy_solver_func_ = reinterpret_cast<DestroySolverFunc>(dlsym(library_handle_, "musica_cuda_destroy_solver"));
    devices_available_func_ =
        reinterpret_cast<DevicesAvailableFunc>(dlsym(library_handle_, "musica_cuda_devices_available"));
    cleanup_func_ = reinterpret_cast<CleanUpFunc>(dlsym(library_handle_, "musica_cuda_cleanup"));

    // Check that all required functions were loaded
    if (!create_rosenbrock_func_ || !destroy_solver_func_ || !devices_available_func_)
    {
      last_error_ = "Failed to load required symbols from libmusica_cuda.so";
      UnloadLibrary();
      return;
    }

    last_error_.clear();
#else
    last_error_ = "CUDA runtime loading not supported on this platform";
#endif
  }

  void CudaLoader::UnloadLibrary() const
  {
#if MUSICA_HAS_DLOPEN
    if (library_handle_)
    {
      dlclose(library_handle_);
      library_handle_ = nullptr;
    }
#endif
    create_rosenbrock_func_ = nullptr;
    destroy_solver_func_ = nullptr;
    devices_available_func_ = nullptr;
    cleanup_func_ = nullptr;
  }

  bool CudaLoader::IsAvailable() const
  {
    // Thread-safe lazy initialization using std::call_once
    std::call_once(load_flag_, [this]() { LoadLibrary(); });
    return library_handle_ != nullptr && create_rosenbrock_func_ != nullptr;
  }

  bool CudaLoader::HasDevices() const
  {
    if (!IsAvailable())
    {
      return false;
    }
    return devices_available_func_();
  }

  CudaSolverPtr CudaLoader::CreateRosenbrockSolver(const Chemistry& chemistry)
  {
    if (!IsAvailable())
    {
      throw std::runtime_error("CUDA solver not available: " + last_error_);
    }

    if (!HasDevices())
    {
      throw std::runtime_error("No CUDA devices available");
    }

    char error_msg[1024] = { 0 };
    IMicmSolver* solver = create_rosenbrock_func_(&chemistry, error_msg, sizeof(error_msg));

    if (!solver)
    {
      throw std::runtime_error("Failed to create CUDA solver: " + std::string(error_msg));
    }

    // Wrap in unique_ptr with custom deleter that calls the CUDA library's destroy function
    return CudaSolverPtr(solver, CudaSolverDeleter(destroy_solver_func_));
  }

  void CudaLoader::CleanUp()
  {
    if (IsAvailable() && cleanup_func_)
    {
      cleanup_func_();
    }
  }

  std::string CudaLoader::GetLastError() const
  {
    return last_error_;
  }

}  // namespace musica
