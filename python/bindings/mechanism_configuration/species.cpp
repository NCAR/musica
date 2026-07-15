// Copyright (C) 2025-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Pybind11 bindings for the species and phase types defined in
// mechanism_configuration. These classes are added to the
// `_mechanism_configuration` submodule and bound from
// bind_mechanism_configuration().
#include "../common.hpp"

#include <mechanism_configuration/types/species.hpp>

#include <pybind11/stl.h>

namespace py = pybind11;
using namespace mechanism_configuration::types;

void bind_species(py::module_ &mechanism_configuration)
{
  py::class_<Species>(mechanism_configuration, "_Species")
      .def(py::init<>())
      .def_readwrite("name", &Species::name)
      .def_readwrite("molecular_weight_kg_mol", &Species::molecular_weight)
      .def_readwrite("density_kg_m3", &Species::density)
      .def_readwrite("constant_concentration_mol_m3", &Species::constant_concentration)
      .def_readwrite("constant_mixing_ratio_mol_mol", &Species::constant_mixing_ratio)
      .def_readwrite("is_third_body", &Species::is_third_body)
      .def_readwrite("other_properties", &Species::unknown_properties)
      .def("__str__", [](const Species &s) { return s.name; })
      .def("__repr__", [](const Species &s) { return "<Species: " + s.name + ">"; });

  py::class_<PhaseSpecies>(mechanism_configuration, "_PhaseSpecies")
      .def(py::init<>())
      .def_readwrite("name", &PhaseSpecies::name)
      .def_readwrite("diffusion_coefficient_m2_s", &PhaseSpecies::diffusion_coefficient)
      .def_readwrite("density_kg_m3", &PhaseSpecies::density)
      .def_readwrite("other_properties", &PhaseSpecies::unknown_properties)
      .def("__str__", [](const PhaseSpecies &s) { return s.name; })
      .def("__repr__", [](const PhaseSpecies &s) { return "<PhaseSpecies: " + s.name + ">"; });

  py::class_<Phase>(mechanism_configuration, "_Phase")
      .def(py::init<>())
      .def_readwrite("name", &Phase::name)
      .def_readwrite("species", &Phase::species)
      .def_readwrite("other_properties", &Phase::unknown_properties)
      .def("__str__", [](const Phase &p) { return p.name; })
      .def("__repr__", [](const Phase &p) { return "<Phase: " + p.name + ">"; });
}
