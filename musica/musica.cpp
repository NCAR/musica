// Copyright (C) 2023-2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/micm/micm.hpp>
#include <musica/micm/micm_c_interface.hpp>
#include <musica/micm/state_c_interface.hpp>
#include <musica/micm/state.hpp>
#include <mechanism_configuration/v1/types.hpp>
#include <musica/micm/cuda_availability.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;
namespace v1 = mechanism_configuration::v1::types;

PYBIND11_MAKE_OPAQUE(std::vector<double>)
PYBIND11_MAKE_OPAQUE(std::vector<micm::Conditions>)

void bind_musica(py::module_ &core)
{
  py::bind_vector<std::vector<double>>(core, "VectorDouble");
  py::bind_vector<std::vector<micm::Conditions>>(core, "VectorConditions");
  
  py::class_<micm::Conditions>(core, "_Conditions")
      .def(py::init<>())
      .def_readwrite("temperature", &micm::Conditions::temperature_)
      .def_readwrite("pressure", &micm::Conditions::pressure_)
      .def_readwrite("air_density", &micm::Conditions::air_density_);

  py::class_<musica::State>(core, "_State")
      .def(py::init<>())
      .def("__del__", [](musica::State &state) { })
      .def("number_of_grid_cells",
           [](musica::State &state) {
             return state.NumberOfGridCells();
           })
      .def_property(
          "conditions",
          [](musica::State &state) -> std::vector<micm::Conditions>& {
            return state.GetConditions();
          },
          nullptr,
          "list of conditions structs for each grid cell")
      .def_property(
          "concentrations",
          [](musica::State &state) -> std::vector<double>& {
            return state.GetOrderedConcentrations();
          },
          nullptr,
          "native 1D list of concentrations, ordered by species and grid cell according to matrix type")
      .def_property(
          "user_defined_rate_parameters",
          [](musica::State &state) -> std::vector<double>& {
            return state.GetOrderedRateParameters();
          },
          nullptr,
          "native 1D list of user-defined rate parameters, ordered by parameter and grid cell according to matrix type")
      .def("concentration_strides",
          [](musica::State &state) {
            return state.GetConcentrationsStrides();
          })
      .def("user_defined_rate_parameter_strides",
          [](musica::State &state) {
            return state.GetUserDefinedRateParametersStrides();
          });

  py::enum_<musica::MICMSolver>(core, "_SolverType")
      .value("rosenbrock", musica::MICMSolver::Rosenbrock)
      .value("rosenbrock_standard_order", musica::MICMSolver::RosenbrockStandardOrder)
      .value("backward_euler", musica::MICMSolver::BackwardEuler)
      .value("backward_euler_standard_order", musica::MICMSolver::BackwardEulerStandardOrder)
      .value("cuda_rosenbrock", musica::MICMSolver::CudaRosenbrock);

  py::class_<musica::MICM>(core, "_Solver");

  core.def("_vector_size",
      [](const musica::MICMSolver solver_type)
      {
        if (solver_type == musica::MICMSolver::RosenbrockStandardOrder ||
            solver_type == musica::MICMSolver::BackwardEulerStandardOrder) {
          return static_cast<std::size_t>(0);
        } else if (solver_type == musica::MICMSolver::Rosenbrock ||
                 solver_type == musica::MICMSolver::BackwardEuler) {
          return musica::MUSICA_VECTOR_SIZE;
        } else {
          throw py::value_error("Invalid MICM solver type.");
        }
      },
      "Returns the vector dimension for vector-ordered solvers, 0 otherwise.");

  core.def(
      "_create_solver",
      [](const char *config_path, musica::MICMSolver solver_type)
      {
        musica::Error error;
        musica::MICM *micm = musica::CreateMicm(config_path, solver_type, &error);
        if (!musica::IsSuccess(error))
        {
          std::cout << "Error creating solver: " << error.message_.value_ << " solver_type: " << solver_type
                    << " config_path: " << config_path << std::endl;
          std::string message = "Error creating solver: " + std::string(error.message_.value_);
          DeleteError(&error);
          throw py::value_error(message);
        }
        return micm;
      });

  core.def(
      "_create_solver_from_mechanism",
      [](const v1::Mechanism &mechanism, musica::MICMSolver solver_type)
      {
        musica::Error error;
        musica::Chemistry chemistry = musica::ConvertV1Mechanism(mechanism);
        musica::MICM *micm = musica::CreateMicmFromChemistryMechanism(&chemistry, solver_type, &error);
        if (!musica::IsSuccess(error))
        {
          std::string message = "Error creating solver: " + std::string(error.message_.value_);
          DeleteError(&error);
          throw py::value_error(message);
        }
        return micm;
      });
  
  core.def(
      "_create_state",
      [](musica::MICM *micm, std::size_t number_of_grid_cells)
      {
        musica::Error error;
        musica::State *state = musica::CreateMicmState(micm, number_of_grid_cells, &error);
        if (!musica::IsSuccess(error))
        {
          std::string message = "Error creating state: " + std::string(error.message_.value_);
          DeleteError(&error);
          throw py::value_error(message);
        }
        return state;
      });

  core.def(
      "_micm_solve",
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
          throw py::value_error(message);
        }
      },
      "Solve the chemistry system");

  core.def(
      "_species_ordering",
      [](musica::State *state)
      {
        std::map<std::string, std::size_t> map;
        std::visit([&map](auto &state) { map = state.variable_map_; }, state->state_variant_);
        return map;
      },
      "Return map of species names to their indices in the state concentrations vector");

  core.def(
      "_user_defined_rate_parameters_ordering",
      [](musica::State *state)
      {
        std::map<std::string, std::size_t> map;

        std::visit([&map](auto &state) { map = state.custom_rate_parameter_map_; }, state->state_variant_);
        return map;
      },
      "Return map of reaction rate parameters to their indices in the state user-defined rate parameters vector");

  core.def("_is_cuda_available", &musica::IsCudaAvailable, "Check if CUDA is available");

  core.def(
      "_print_state",
      [](musica::State *state, const double current_time)
      {
        std::visit([&current_time](auto &state) {
          std::cout << "Current time: " << current_time << std::endl;
          std::cout << "State variables: " << std::endl;
          std::vector<std::string> species_names(state.variable_map_.size());
          for (const auto &species : state.variable_map_)
            species_names[species.second] = species.first;
          for (const auto &name : species_names)
            std::cout << name << ",";
          std::cout << std::endl << state.variables_ << std::endl;
          std::cout << "User-defined rate parameters: " << std::endl;
          std::vector<std::string> rate_param_names(state.custom_rate_parameter_map_.size());
          for (const auto &rate : state.custom_rate_parameter_map_)
            rate_param_names[rate.second] = rate.first;
          for (const auto &name : rate_param_names)
            std::cout << name << ",";
          std::cout << std::endl << state.custom_rate_parameters_ << std::endl;
          std::cout << "Conditions: " << std::endl;
          std::cout << "Temperature,Pressure,Air density" << std::endl;
          for (const auto &condition : state.conditions_)
          {
            std::cout << condition.temperature_ << ","
                      << condition.pressure_ << ","
                      << condition.air_density_ << std::endl;
          }
        }, state->state_variant_);
      },
      "Print the state to stdout with the current time");
}