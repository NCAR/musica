// Copyright (C) 2023-2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include "../common.hpp"

#include <musica/micm/cuda_availability.hpp>
#include <musica/micm/micm.hpp>
#include <musica/micm/micm_c_interface.hpp>
#include <musica/micm/state.hpp>
#include <musica/micm/state_c_interface.hpp>

#include <micm/version.hpp>

#include <mechanism_configuration/v1/types.hpp>

#include <iostream>

namespace py = pybind11;
namespace v1 = mechanism_configuration::v1::types;

void bind_micm(py::module_& micm)
{
  py::class_<musica::MICM, std::shared_ptr<musica::MICM>>(micm, "MICM");

  // Expose SolverState enum
  py::enum_<micm::SolverState>(micm, "_SolverState")
      .value("NotYetCalled", micm::SolverState::NotYetCalled)
      .value("Running", micm::SolverState::Running)
      .value("Converged", micm::SolverState::Converged)
      .value("ConvergenceExceededMaxSteps", micm::SolverState::ConvergenceExceededMaxSteps)
      .value("StepSizeTooSmall", micm::SolverState::StepSizeTooSmall)
      .value("RepeatedlySingularMatrix", micm::SolverState::RepeatedlySingularMatrix)
      .value("NaNDetected", micm::SolverState::NaNDetected)
      .value("InfDetected", micm::SolverState::InfDetected)
      .value("AcceptingUnconvergedIntegration", micm::SolverState::AcceptingUnconvergedIntegration)
      .export_values();

  micm.def(
      "_vector_size",
      [](const musica::MICMSolver solver_type)
      {
        switch (solver_type)
        {
          case musica::MICMSolver::Rosenbrock:
          case musica::MICMSolver::BackwardEuler:
          case musica::MICMSolver::CudaRosenbrock: return musica::MUSICA_VECTOR_SIZE;
          case musica::MICMSolver::RosenbrockStandardOrder:
          case musica::MICMSolver::BackwardEulerStandardOrder: return static_cast<std::size_t>(1);
          default: throw py::value_error("Invalid MICM solver type.");
        }
      },
      "Returns the vector dimension for vector-ordered solvers, 0 otherwise.");

  micm.def(
      "_create_solver",
      [](const char* config_path, musica::MICMSolver solver_type)
      {
        musica::Error error;
        musica::MICM* micm = musica::CreateMicm(config_path, solver_type, &error);
        if (!musica::IsSuccess(error))
        {
          std::cerr << "Error creating solver: " << error.message_.value_ << " solver_type: " << solver_type
                    << " config_path: " << config_path << std::endl;
          std::string message = "Error creating solver: " + std::string(error.message_.value_);
          musica::DeleteError(&error);
          throw py::value_error(message);
        }

        return std::shared_ptr<musica::MICM>(
            micm,
            [](musica::MICM* ptr)
            {
              musica::Error error;
              musica::DeleteMicm(ptr, &error);
              if (!musica::IsSuccess(error))
              {
                std::cerr << "Error deleting MICM: " << error.message_.value_ << std::endl;
                musica::DeleteError(&error);
              }
            });
      });

  micm.def(
      "_create_solver_from_mechanism",
      [](const v1::Mechanism& mechanism, musica::MICMSolver solver_type, bool ignore_non_gas_phases)
      {
        musica::Error error;
        musica::Chemistry chemistry = musica::ConvertV1Mechanism(mechanism, ignore_non_gas_phases);
        musica::MICM* micm = musica::CreateMicmFromChemistryMechanism(&chemistry, solver_type, &error);
        if (!musica::IsSuccess(error))
        {
          std::string message = "Error creating solver: " + std::string(error.message_.value_);
          musica::DeleteError(&error);
          throw py::value_error(message);
        }

        return std::shared_ptr<musica::MICM>(
            micm,
            [](musica::MICM* ptr)
            {
              musica::Error error;
              musica::DeleteMicm(ptr, &error);
              if (!musica::IsSuccess(error))
              {
                std::cerr << "Error deleting MICM: " << error.message_.value_ << std::endl;
                musica::DeleteError(&error);
              }
            });
      });

  micm.def(
      "_create_state",
      [](musica::MICM* micm, std::size_t number_of_grid_cells)
      {
        musica::Error error;
        musica::State* state = musica::CreateMicmState(micm, number_of_grid_cells, &error);
        if (!musica::IsSuccess(error))
        {
          std::string message = "Error creating state: " + std::string(error.message_.value_);
          DeleteError(&error);
          throw py::value_error(message);
        }

        return std::unique_ptr<musica::State, std::function<void(musica::State*)>>(
            state,
            [](musica::State* ptr)
            {
              if (!ptr)
                return;

              musica::Error error;
              musica::DeleteState(ptr, &error);
              if (!musica::IsSuccess(error))
              {
                std::cerr << "Error deleting State: " << error.message_.value_ << std::endl;
                musica::DeleteError(&error);
              }
            });
      });

  py::class_<musica::SolverResultStats>(micm, "_SolverResultsStats")
      .def(py::init<>())
      .def_readwrite("function_calls", &musica::SolverResultStats::function_calls_)
      .def_readwrite("jacobian_updates", &musica::SolverResultStats::jacobian_updates_)
      .def_readwrite("number_of_steps", &musica::SolverResultStats::number_of_steps_)
      .def_readwrite("accepted", &musica::SolverResultStats::accepted_)
      .def_readwrite("rejected", &musica::SolverResultStats::rejected_)
      .def_readwrite("decompositions", &musica::SolverResultStats::decompositions_)
      .def_readwrite("solves", &musica::SolverResultStats::solves_)
      .def_readwrite("final_time", &musica::SolverResultStats::final_time_)
      .def(
          "__str__",
          [](const musica::SolverResultStats& e)
          {
            return std::string("SolverResultStats{function_calls: ") + std::to_string(e.function_calls_) +
                   ", jacobian_updates: " + std::to_string(e.jacobian_updates_) +
                   ", number_of_steps: " + std::to_string(e.number_of_steps_) +
                   ", accepted: " + std::to_string(e.accepted_) + ", rejected: " + std::to_string(e.rejected_) +
                   ", decompositions: " + std::to_string(e.decompositions_) + ", solves: " + std::to_string(e.solves_) +
                   ", final_time: " + std::to_string(e.final_time_) + "}";
          })
      .def(
          "__repr__",
          [](const musica::SolverResultStats& e)
          {
            return std::string("SolverResultStats(") + std::to_string(e.function_calls_) + ", " +
                   std::to_string(e.jacobian_updates_) + ", " + std::to_string(e.number_of_steps_) + ", " +
                   std::to_string(e.accepted_) + ", " + std::to_string(e.rejected_) + ", " +
                   std::to_string(e.decompositions_) + ", " + std::to_string(e.solves_) + ", " +
                   std::to_string(e.final_time_) + ")";
          });

  py::class_<micm::SolverResult>(micm, "_SolverResult")
      .def_readonly("state", &micm::SolverResult::state_)
      .def_readonly("stats", &micm::SolverResult::stats_)
      .def("__str__", [](const micm::SolverResult& e) { return "<SolverResult>"; })
      .def("__repr__", [](const micm::SolverResult& e) { return "<SolverResult>"; });

  micm.def(
      "_micm_solve",
      [](musica::MICM* micm, musica::State* state, double time_step) { return micm->Solve(state, time_step); },
      "Solve the chemistry system");

  micm.def(
      "_species_ordering",
      [](musica::State* state)
      {
        std::map<std::string, std::size_t> map;
        std::visit([&map](auto& state) { map = state.variable_map_; }, state->state_variant_);
        return map;
      },
      "Return map of species names to their indices in the state concentrations vector");

  micm.def(
      "_user_defined_rate_parameters_ordering",
      [](musica::State* state)
      {
        std::map<std::string, std::size_t> map;

        std::visit([&map](auto& state) { map = state.custom_rate_parameter_map_; }, state->state_variant_);
        return map;
      },
      "Return map of reaction rate parameters to their indices in the state user-defined rate parameters vector");

  micm.def("_is_cuda_available", &musica::IsCudaAvailable, "Check if CUDA is available");

  micm.def("_get_micm_version", []() { return micm::GetMicmVersion(); }, "Get the version of MICM");

  micm.def(
      "_print_state",
      [](musica::State* state, const double current_time)
      {
        std::visit(
            [&current_time](auto& state)
            {
              std::cout << "Current time: " << current_time << std::endl;
              std::cout << "State variables: " << std::endl;
              std::vector<std::string> species_names(state.variable_map_.size());
              for (const auto& species : state.variable_map_)
                species_names[species.second] = species.first;
              for (const auto& name : species_names)
                std::cout << name << ",";
              std::cout << std::endl << state.variables_ << std::endl;
              std::cout << "User-defined rate parameters: " << std::endl;
              std::vector<std::string> rate_param_names(state.custom_rate_parameter_map_.size());
              for (const auto& rate : state.custom_rate_parameter_map_)
                rate_param_names[rate.second] = rate.first;
              for (const auto& name : rate_param_names)
                std::cout << name << ",";
              std::cout << std::endl << state.custom_rate_parameters_ << std::endl;
              std::cout << "Conditions: " << std::endl;
              std::cout << "Temperature,Pressure,Air density" << std::endl;
              for (const auto& condition : state.conditions_)
              {
                std::cout << condition.temperature_ << "," << condition.pressure_ << "," << condition.air_density_
                          << std::endl;
              }
            },
            state->state_variant_);
      },
      "Print the state to stdout with the current time");
}
