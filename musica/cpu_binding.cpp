// Copyright (C) 2023-2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#include "binding_common.hpp"

#include <pybind11/pybind11.h>

PYBIND11_MODULE(_musica, m)
{
  bind_all(m);
}
