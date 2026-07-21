// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include "../common.hpp"

#include <musica/miem/emissions.hpp>

#include <mechanism_configuration/mechanism.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>
#include <vector>

namespace py = pybind11;
using namespace mechanism_configuration;

void bind_miem(py::module_& miem)
{
  py::class_<musica::EmissionsModel, std::shared_ptr<musica::EmissionsModel>>(miem, "_EmissionsModel")
      .def(
          "run",
          [](musica::EmissionsModel& self, double epoch_seconds, double dt_seconds)
          {
            const miem::EmissionsState& state = self.Run(epoch_seconds, dt_seconds);
            int n_species = state.surface_flux_.n_species();
            int n_cells = state.surface_flux_.n_cells();

            // Copy the flux buffer into a freshly heap-allocated vector immediately
            // after Run(), rather than pointing into state_ directly: EmissionsModel
            // reassigns its internal state on every Run() call, so a live view into
            // that buffer could dangle once the next Run() is called.
            auto buffer = std::make_unique<std::vector<miem::Real>>(
                state.surface_flux_.raw().begin(), state.surface_flux_.raw().end());
            miem::Real* data_ptr = buffer->data();
            auto capsule = py::capsule(buffer.get(), [](void* v) { delete reinterpret_cast<std::vector<miem::Real>*>(v); });
            buffer.release();
            return py::array_t<miem::Real>({ n_species, n_cells }, data_ptr, capsule);
          },
          "Advance one time step and return surface flux as a (num_species, n_cells) numpy array [kg m-2 s-1].")
      .def_property_readonly("num_species", &musica::EmissionsModel::NumSpecies)
      .def_property_readonly("species_names", &musica::EmissionsModel::SpeciesNames);

  miem.def(
      "_create_emissions_from_mechanism",
      [](const Mechanism& mechanism, int n_cells, int n_vert_levels) -> std::shared_ptr<musica::EmissionsModel>
      {
        return std::make_shared<musica::EmissionsModel>(
            musica::EmissionsModel::FromMechanism(mechanism, n_cells, n_vert_levels));
      },
      "Build an EmissionsModel from a parsed Mechanism's emissions section.");
}
