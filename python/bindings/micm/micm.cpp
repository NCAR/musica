// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include "../common.hpp"

#include <musica/micm/cuda_availability.hpp>
#include <musica/micm/micm.hpp>
#include <musica/micm/micm_c_interface.hpp>
#include <musica/micm/solver_parameters.hpp>
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

  micm.def("_vector_size", &musica::GetVectorSize, "Returns the vector dimension for vector-ordered solvers, 1 otherwise.");

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
      [](const v1::Mechanism& mechanism, musica::MICMSolver solver_type)
      {
        musica::Error error;
        musica::Chemistry chemistry = musica::ConvertV1Mechanism(mechanism);
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

  py::class_<musica::RosenbrockSolverParameters>(micm, "_RosenbrockSolverParameters")
      .def(py::init<>())
      .def_readwrite("relative_tolerance", &musica::RosenbrockSolverParameters::relative_tolerance)
      .def_readwrite("absolute_tolerances", &musica::RosenbrockSolverParameters::absolute_tolerances)
      .def_readwrite("h_min", &musica::RosenbrockSolverParameters::h_min)
      .def_readwrite("h_max", &musica::RosenbrockSolverParameters::h_max)
      .def_readwrite("h_start", &musica::RosenbrockSolverParameters::h_start)
      .def_readwrite("max_number_of_steps", &musica::RosenbrockSolverParameters::max_number_of_steps);

  py::class_<musica::BackwardEulerSolverParameters>(micm, "_BackwardEulerSolverParameters")
      .def(py::init<>())
      .def_readwrite("relative_tolerance", &musica::BackwardEulerSolverParameters::relative_tolerance)
      .def_readwrite("absolute_tolerances", &musica::BackwardEulerSolverParameters::absolute_tolerances)
      .def_readwrite("max_number_of_steps", &musica::BackwardEulerSolverParameters::max_number_of_steps)
      .def_readwrite("time_step_reductions", &musica::BackwardEulerSolverParameters::time_step_reductions);

  micm.def(
      "_set_rosenbrock_solver_parameters",
      [](musica::MICM* micm, const musica::RosenbrockSolverParameters& params) { micm->SetSolverParameters(params); },
      "Set Rosenbrock solver parameters");

  micm.def(
      "_set_backward_euler_solver_parameters",
      [](musica::MICM* micm, const musica::BackwardEulerSolverParameters& params) { micm->SetSolverParameters(params); },
      "Set Backward Euler solver parameters");

  micm.def(
      "_get_rosenbrock_solver_parameters",
      [](musica::MICM* micm) { return micm->GetRosenbrockSolverParameters(); },
      "Get Rosenbrock solver parameters");

  micm.def(
      "_get_backward_euler_solver_parameters",
      [](musica::MICM* micm) { return micm->GetBackwardEulerSolverParameters(); },
      "Get Backward Euler solver parameters");

  micm.def(
      "_micm_solve",
      [](musica::MICM* micm, musica::State* state, double time_step) { return micm->Solve(state, time_step); },
      "Solve the chemistry system");

  micm.def(
      "_species_ordering",
      [](musica::State* state) { return state->GetVariableMap(); },
      "Return map of species names to their indices in the state concentrations vector");

  micm.def(
      "_user_defined_rate_parameters_ordering",
      [](musica::State* state) { return state->GetRateParameterMap(); },
      "Return map of reaction rate parameters to their indices in the state user-defined rate parameters vector");

  micm.def("_is_cuda_available", &musica::IsCudaAvailable, "Check if CUDA is available");

  micm.def("_get_micm_version", []() { return micm::GetMicmVersion(); }, "Get the version of MICM");

  micm.def(
      "_print_state",
      [](musica::State* state, const double current_time)
      {
        std::cout << "Current time: " << current_time << std::endl;
        std::cout << "State variables: " << std::endl;
        auto variable_map = state->GetVariableMap();
        std::vector<std::string> species_names(variable_map.size());
        for (const auto& species : variable_map)
          species_names[species.second] = species.first;
        for (const auto& name : species_names)
          std::cout << name << ",";
        std::cout << std::endl;
        auto& concentrations = state->GetOrderedConcentrations();
        for (const auto& c : concentrations)
          std::cout << c << " ";
        std::cout << std::endl;
        std::cout << "User-defined rate parameters: " << std::endl;
        auto rate_param_map = state->GetRateParameterMap();
        std::vector<std::string> rate_param_names(rate_param_map.size());
        for (const auto& rate : rate_param_map)
          rate_param_names[rate.second] = rate.first;
        for (const auto& name : rate_param_names)
          std::cout << name << ",";
        std::cout << std::endl;
        auto& rate_params = state->GetOrderedRateParameters();
        for (const auto& r : rate_params)
          std::cout << r << " ";
        std::cout << std::endl;
        std::cout << "Conditions: " << std::endl;
        std::cout << "Temperature,Pressure,Air density" << std::endl;
        auto& conditions = state->GetConditions();
        for (const auto& condition : conditions)
        {
          std::cout << condition.temperature_ << "," << condition.pressure_ << "," << condition.air_density_ << std::endl;
        }
      },
      "Print the state to stdout with the current time");
}
