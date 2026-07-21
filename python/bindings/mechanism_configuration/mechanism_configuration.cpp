// Copyright (C) 2025-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include "../common.hpp"

#include <musica/configuration/parse.hpp>

#include <mechanism_configuration/mechanism_configuration.hpp>

#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

namespace py = pybind11;
using namespace mechanism_configuration;
using namespace mechanism_configuration::types;

// The schema types are bound in their own translation units (species.cpp,
// reactions.cpp, aerosol.cpp). bind_mechanism_configuration() is the single
// entry point: it delegates to these and binds the top-level Mechanism/Version
// types plus the parser.
void bind_species(py::module_ &);
void bind_reactions(py::module_ &);
void bind_aerosol(py::module_ &);
void bind_emissions(py::module_ &);

void bind_mechanism_configuration(py::module_ &mechanism_configuration)
{
  // Species and phase types live in species.cpp.
  bind_species(mechanism_configuration);

  // Reaction types (and the Reactions container/iterator) live in reactions.cpp.
  bind_reactions(mechanism_configuration);

  // Aerosol types live in aerosol.cpp.
  bind_aerosol(mechanism_configuration);

  // Emissions types live in emissions.cpp.
  bind_emissions(mechanism_configuration);

  py::class_<Mechanism>(mechanism_configuration, "_Mechanism")
      .def(py::init<>())
      .def_readwrite("name", &Mechanism::name)
      .def_readwrite("version", &Mechanism::version)
      .def_readwrite("relative_tolerance", &Mechanism::relative_tolerance)
      .def_readwrite("species", &Mechanism::species)
      .def_readwrite("phases", &Mechanism::phases)
      .def_readwrite("reactions", &Mechanism::reactions)
      .def_readwrite("aerosol", &Mechanism::aerosol)
      .def_readwrite("emissions", &Mechanism::emissions)
      .def("__str__", [](const Mechanism &m) { return m.name; })
      .def("__repr__", [](const Mechanism &m) { return "<Mechanism: " + m.name + ">"; });

  py::class_<mechanism_configuration::Version>(mechanism_configuration, "_Version")
      .def(py::init<>())
      .def(py::init<unsigned int, unsigned int, unsigned int>())
      .def(py::init<std::string>())
      .def_readwrite("major", &mechanism_configuration::Version::major)
      .def_readwrite("minor", &mechanism_configuration::Version::minor)
      .def_readwrite("patch", &mechanism_configuration::Version::patch)
      .def("to_string", &mechanism_configuration::Version::to_string)
      .def("__str__", &mechanism_configuration::Version::to_string)
      .def("__repr__", [](const mechanism_configuration::Version &v) { return "<Version: " + v.to_string() + ">"; });

  mechanism_configuration.def(
      "_parse",
      [](const std::string &file_path)
      {
        auto parsed = Parse(file_path);
        if (parsed)
        {
          return *parsed;
        }
        else
        {
          std::string error{};
          for (const auto &[code, message] : parsed.error())
          {
            error += message + "\n";
          }
          throw std::runtime_error(error);
        }
      });
}
