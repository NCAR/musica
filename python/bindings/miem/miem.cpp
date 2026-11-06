// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include "../common.hpp"

#include <musica/emissions/emissions.hpp>

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
            self.Run(epoch_seconds, dt_seconds);
            const musica::SurfaceFluxView flux = self.SurfaceFlux();

            // Copy into a new array: the flux buffer is overwritten by the next Run().
            py::array_t<double> result({ flux.n_species, flux.n_cells });
            auto out = result.mutable_unchecked<2>();
            for (std::size_t i_species = 0; i_species < flux.n_species; ++i_species)
            {
              for (std::size_t i_cell = 0; i_cell < flux.n_cells; ++i_cell)
              {
                out(i_species, i_cell) = flux.data[i_species * flux.species_stride + i_cell * flux.cell_stride];
              }
            }
            return result;
          },
          "Advance one time step and return surface flux as a (num_species, n_cells) numpy array [kg m-2 s-1].")
      .def_property_readonly("num_species", [](const musica::EmissionsModel& self) { return self.SpeciesNames().size(); })
      .def_property_readonly("species_names", &musica::EmissionsModel::SpeciesNames);

  miem.def(
      "_create_emissions_from_mechanism",
      [](const Mechanism& mechanism, int n_cells, int n_vert_levels) -> std::shared_ptr<musica::EmissionsModel>
      { return std::make_shared<musica::EmissionsModel>(mechanism, n_cells, n_vert_levels); },
      "Build an EmissionsModel from a parsed Mechanism's emissions section.");
}
