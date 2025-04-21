// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/micm/micm.hpp>
#include <musica/micm/state.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <mechanism_configuration/v1/types.hpp>

namespace py = pybind11;
using Mechanism = mechanism_configuration::v1::types::Mechanism;

// Wraps micm.cpp
PYBIND11_MODULE(musica, m)
{
  // Import mechanism configuration module
  py::module_ mechanism_config = py::module_::import("mechanism_config");
  
  py::class_<micm::Conditions>(m, "Conditions")
      .def(py::init<>())
      .def_readwrite("temperature", &micm::Conditions::temperature_)
      .def_readwrite("pressure", &micm::Conditions::pressure_)
      .def_readwrite("air_density", &micm::Conditions::air_density_);

  py::class_<musica::MICM>(m, "micm")
      .def(py::init<>())
      .def(
          "__del__",
          [](musica::MICM *micm)
          {
            musica::Error *error;
            musica::DeleteMicm(micm, error);
          });

  py::class_<musica::State>(m, "state")
      .def(py::init<>())
      .def("__del__", [](musica::State &state) { })
      .def_property(
          "conditions",
          &musica::State::GetConditions,
          py::overload_cast<const std::vector<micm::Conditions> &>(&musica::State::SetConditions))
      .def_property(
          "ordered_concentrations",
          &musica::State::GetOrderedConcentrations,
          py::overload_cast<const std::vector<double> &>(&musica::State::SetOrderedConcentrations))
      .def_property(
          "ordered_rate_constants",
          &musica::State::GetOrderedRateConstants,
          py::overload_cast<const std::vector<double> &>(&musica::State::SetOrderedRateConstants));

  py::enum_<musica::MICMSolver>(m, "micmsolver")
      .value("rosenbrock", musica::MICMSolver::Rosenbrock)
      .value("rosenbrock_standard_order", musica::MICMSolver::RosenbrockStandardOrder)
      .value("backward_euler", musica::MICMSolver::BackwardEuler)
      .value("backward_euler_standard_order", musica::MICMSolver::BackwardEulerStandardOrder);

  m.def(
      "create_solver_from_mechanism",
      [](const &Mechansim mechanism, musica::MICMSolver solver_type, int num_grid_cells)
      {
        musica::Error error;
        musica::MICM *micm = musica::CreateMicmFromMechanism(mechanism, solver_type, num_grid_cells, &error);
        if (!musica::IsSuccess(error))
        {
          std::string message = "Error creating solver: " + std::string(error.message_.value_);
          DeleteError(&error);
          throw std::runtime_error(message);
        }
        return micm;
      },
      "Create a solver from a mechanism in code");

  m.def(
      "create_solver",
      [](const char *config_path, musica::MICMSolver solver_type, int num_grid_cells)
      {
        musica::Error error;
        musica::MICM *micm = musica::CreateMicm(config_path, solver_type, num_grid_cells, &error);
        if (!musica::IsSuccess(error))
        {
          std::string message = "Error creating solver: " + std::string(error.message_.value_);
          DeleteError(&error);
          throw std::runtime_error(message);
        }
        return micm;
      },
      "Create a solver from a configuration file");

  m.def(
      "create_state",
      [](musica::MICM *micm)
      {
        musica::Error error;
        musica::State *state = musica::CreateMicmState(micm, &error);
        if (!musica::IsSuccess(error))
        {
          std::string message = "Error creating state: " + std::string(error.message_.value_);
          DeleteError(&error);
          throw std::runtime_error(message);
        }
        return state;
      },
      "Create a state object for use with the solver");

  m.def("delete_micm", &musica::DeleteMicm, "Delete a solver");

  m.def(
      "micm_solve",
      [](musica::MICM *micm, musica::State *state, double time_step)
      {
        musica::String solver_state;
        musica::SolverResultStats solver_stats;
        musica::Error error;
        musica::MicmSolve(micm, state, time_step, &solver_state, &solver_stats, &error);
        if (!musica::IsSuccess(error))
        {
          std::string message = "Error solving system: " + std::string(error.message_.value_);
          DeleteError(&error);
          throw std::runtime_error(message);
        }
      },
      "Solve the system, updating the state object with the results");

  m.def(
      "species_ordering",
      [](musica::MICM *micm, musica::State *state)
      {
        std::map<std::string, std::size_t> map;
        std::visit([&map](auto &state) { map = state.variable_map_; }, state->state_variant_);
        return map;
      },
      "Return a map of species name to index in the state::concentrations vector");

  m.def(
      "user_defined_reaction_rates",
      [](musica::MICM *micm, musica::State *state)
      {
        std::map<std::string, std::size_t> map;

        std::visit([&map](auto &state) { map = state.custom_rate_parameter_map_; }, state->state_variant_);
        return map;
      },
      "Return a map of user-defined reaction rate parameter names to indices in the state::custom_rate_parameters vector");
}