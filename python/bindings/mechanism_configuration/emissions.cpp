// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Pybind11 bindings for the emissions configuration types defined in
// mechanism_configuration/types/emissions.hpp. These classes are added to the
// same `_mechanism_configuration` submodule as the species/reaction/aerosol
// types and are attached to a Mechanism via its optional `emissions` member.
//
// Following the convention used for the other mechanism_configuration types,
// each struct is bound with a default constructor plus read/write field access.
#include "../common.hpp"

#include <mechanism_configuration/types/emissions.hpp>

#include <pybind11/stl.h>

using namespace mechanism_configuration::types;

void bind_emissions(py::module_ &mechanism_configuration)
{
  // -- Enums ------------------------------------------------------------

  py::enum_<SourceMode>(mechanism_configuration, "_SourceMode").value("Offline", SourceMode::Offline);

  py::enum_<SourceType>(mechanism_configuration, "_SourceType")
      .value("Anthropogenic", SourceType::Anthropogenic)
      .value("Fire", SourceType::Fire)
      .value("Biogenic", SourceType::Biogenic)
      .value("Dust", SourceType::Dust)
      .value("SeaSalt", SourceType::SeaSalt)
      .value("Lightning", SourceType::Lightning);

  py::enum_<TemporalInterpolation>(mechanism_configuration, "_TemporalInterpolation")
      .value("Linear", TemporalInterpolation::Linear)
      .value("Nearest", TemporalInterpolation::Nearest)
      .value("None_", TemporalInterpolation::None);

  py::enum_<VerticalInjection>(mechanism_configuration, "_VerticalInjection")
      .value("Surface", VerticalInjection::Surface);

  py::enum_<RegriddingType>(mechanism_configuration, "_RegriddingType").value("None_", RegriddingType::None);

  // -- Inventories and species maps --------------------------------------

  py::class_<SpeciesMapping>(mechanism_configuration, "_SpeciesMapping")
      .def(py::init<>())
      .def_readwrite("inventory_species", &SpeciesMapping::inventory_species)
      .def_readwrite("mechanism_species", &SpeciesMapping::mechanism_species)
      .def_readwrite("scaling_factor", &SpeciesMapping::scaling_factor)
      .def(
          "__repr__",
          [](const SpeciesMapping &m)
          { return "<SpeciesMapping: " + m.inventory_species + " -> " + m.mechanism_species + ">"; });

  py::class_<SpeciesMap>(mechanism_configuration, "_SpeciesMap")
      .def(py::init<>())
      .def_readwrite("name", &SpeciesMap::name)
      .def_readwrite("mappings", &SpeciesMap::mappings)
      .def("__str__", [](const SpeciesMap &m) { return m.name; })
      .def("__repr__", [](const SpeciesMap &m) { return "<SpeciesMap: " + m.name + ">"; });

  py::class_<Inventory>(mechanism_configuration, "_Inventory")
      .def(py::init<>())
      .def_readwrite("name", &Inventory::name)
      .def_readwrite("directory", &Inventory::directory)
      .def_readwrite("file_pattern", &Inventory::file_pattern)
      .def_readwrite("convention", &Inventory::convention)
      .def("__str__", [](const Inventory &i) { return i.name; })
      .def("__repr__", [](const Inventory &i) { return "<Inventory: " + i.name + ">"; });

  // -- Sources ------------------------------------------------------------

  py::class_<SourceDescriptor>(mechanism_configuration, "_SourceDescriptor")
      .def(py::init<>())
      .def_readwrite("name", &SourceDescriptor::name)
      .def_readwrite("mode", &SourceDescriptor::mode)
      .def_readwrite("type", &SourceDescriptor::type)
      .def_readwrite("inventory", &SourceDescriptor::inventory)
      .def_readwrite("species_map", &SourceDescriptor::species_map)
      .def_readwrite("temporal_interpolation", &SourceDescriptor::temporal_interpolation)
      .def_readwrite("vertical_injection", &SourceDescriptor::vertical_injection)
      .def_readwrite("category", &SourceDescriptor::category)
      .def_readwrite("hierarchy", &SourceDescriptor::hierarchy)
      .def_readwrite("scaling_factor", &SourceDescriptor::scaling_factor)
      .def_readwrite("sector", &SourceDescriptor::sector)
      .def_readwrite("other_properties", &SourceDescriptor::unknown_properties)
      .def("__str__", [](const SourceDescriptor &s) { return s.name; })
      .def("__repr__", [](const SourceDescriptor &s) { return "<SourceDescriptor: " + s.name + ">"; });

  // -- Regridding -----------------------------------------------------------

  py::class_<Regridding>(mechanism_configuration, "_Regridding")
      .def(py::init<>())
      .def_readwrite("type", &Regridding::type)
      .def("__repr__", [](const Regridding &) { return "<Regridding>"; });

  // -- Container ------------------------------------------------------------

  py::class_<EmissionsConfig>(mechanism_configuration, "_EmissionsConfig")
      .def(py::init<>())
      .def_readwrite("inventories", &EmissionsConfig::inventories)
      .def_readwrite("species_maps", &EmissionsConfig::species_maps)
      .def_readwrite("regridding", &EmissionsConfig::regridding)
      .def_readwrite("sources", &EmissionsConfig::sources)
      .def("__repr__", [](const EmissionsConfig &) { return "<EmissionsConfig>"; });
}
