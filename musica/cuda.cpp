// Copyright (C) 2023-2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/micm/cuda.hpp>

#include <pybind11/pybind11.h>
#include <Python.h>

#include <filesystem>
#include <dlfcn.h>

namespace py = pybind11;

void* TryLoadCudaFromSitePackages(const std::string& relative_path)
{
  PyObject* sys_path = PySys_GetObject("path");
  for (Py_ssize_t i = 0; i < PyList_Size(sys_path); ++i) {
      PyObject* item = PyList_GetItem(sys_path, i);
      const char* path = PyUnicode_AsUTF8(item);
      std::filesystem::path full = std::filesystem::path(path) / relative_path;
      if (std::filesystem::exists(full)) {
          return dlopen(full.c_str(), RTLD_LAZY | RTLD_GLOBAL);
      }
  }
  return nullptr;
}

void* LoadCudaLibrary(const std::string& relative_path) {
  void* handle = dlopen(relative_path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  if (!handle) {
    throw std::runtime_error("Failed to load CUDA library: " + std::string(dlerror()));
  }
  return handle;
}

void TryLoadCuda()
{
  std::string nvidia_cudart_site_packages_path = "nvida/cuda_runtime/lib";
  std::string nvidia_cudart_so = "libcudart.so.12";
  std::string nvidia_cublas_site_packages_path = "nvida/cublas/lib";
  std::string nvidia_cublas_so = "libcublas.so.12";
  std::string nvidia_cublasLt_so = "libcublasLt.so.12";

  std::string micm_cuda_so = "libmicm_cuda.so";

  void* micm_cuda_handle = TryLoadCudaFromSitePackages(micm_cuda_so);
  if (!micm_cuda_handle) {
    micm_cuda_handle = LoadCudaLibrary(micm_cuda_so);
  }
  if (!micm_cuda_handle) {
    throw std::runtime_error("Failed to load MICM CUDA library.");
  }

  void* cudart_handle = TryLoadCudaFromSitePackages(nvidia_cudart_site_packages_path + "/" + nvidia_cudart_so);
  if (!cudart_handle) {
    cudart_handle = LoadCudaLibrary(nvidia_cudart_so);
  }
  if (!cudart_handle) {
    throw std::runtime_error("Failed to load CUDA runtime library.");
  }

  void* cublas_handle = TryLoadCudaFromSitePackages(nvidia_cublas_site_packages_path + "/" + nvidia_cublas_so);
  if (!cublas_handle) {
    cublas_handle = LoadCudaLibrary(nvidia_cublas_so);
  }
  if (!cublas_handle) {
    throw std::runtime_error("Failed to load cuBLAS library.");
  }
  void* cublasLt_handle = TryLoadCudaFromSitePackages(nvidia_cublas_site_packages_path + "/" + nvidia_cublasLt_so);
  if (!cublasLt_handle) {
    cublasLt_handle = LoadCudaLibrary(nvidia_cublasLt_so);
  }
}

void bind_cuda(py::module_ &cuda)
{
  cuda.def("_is_cuda_available", &musica::IsCudaAvailable, "Check if CUDA is available");
  cuda.def("_try_load_cuda", &TryLoadCuda, "Attempt to load CUDA libraries from site-packages or system paths");
}