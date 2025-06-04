// Copyright (C) 2023-2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_musica(py::module_ &);
void bind_mechanism_configuration(py::module_ &);

// Wraps micm.cpp
PYBIND11_MODULE(_musica, m)
{
  py::module_ core = m.def_submodule("_core", "Wrapper classes for MUSICA C library structs and functions");
  py::module_ mechanism_configuration = m.def_submodule("_mechanism_configuration", "Wrapper classes for Mechanism Configuration library structs and functions");

  bind_musica(core);
  bind_mechanism_configuration(mechanism_configuration);
}