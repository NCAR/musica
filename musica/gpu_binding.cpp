// Copyright (C) 2023-2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#include <pybind11/pybind11.h>
#include "binding_common.hpp"

PYBIND11_MODULE(_musica_gpu, m)
{
  bind_all(m);
}
