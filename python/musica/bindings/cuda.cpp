// Copyright (C) 2023-2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/micm/cuda_availability.hpp>

#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_cuda(py::module_ &cuda)
{
  cuda.def("_is_cuda_available", &musica::IsCudaAvailable, "Check if CUDA is available");
}
