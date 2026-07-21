// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Pybind11 bindings for MIAM solver creation. The aerosol configuration types
// live in the mechanism_configuration module (see aerosol.cpp) and are attached
// to a Mechanism via its `aerosol` member; this module only exposes the MIAM
// version and the solver-creation entry point.

#include "../common.hpp"

#include <musica/miam/miam_builder.hpp>
#include <musica/micm/micm.hpp>
#include <musica/micm/micm_c_interface.hpp>

#include <mechanism_configuration/mechanism.hpp>

#include <miam/version.hpp>

namespace py = pybind11;

void bind_miam(py::module_& miam)
{
  miam.def(
      "_get_miam_version", []() { return std::string(miam::GetMiamVersion()); }, "Get the version of the MIAM instance");

  // The aerosol model is read from the mechanism's `aerosol` section.
  miam.def(
      "_create_solver_with_miam",
      [](const mechanism_configuration::Mechanism& mechanism, musica::MICMSolver solver_type)
      {
        musica::Error error;
        musica::MICM* micm = musica::CreateMicmWithMiam(mechanism, solver_type, &error);
        handle_error(error, "Error creating solver with MIAM");

        return std::shared_ptr<musica::MICM>(
            micm,
            [](musica::MICM* ptr)
            {
              musica::Error error;
              musica::DeleteMicm(ptr, &error);
              musica::DeleteError(&error);
            });
      },
      py::arg("mechanism"),
      py::arg("solver_type"),
      "Create a MICM solver with the mechanism's MIAM aerosol model attached as an external model");
}