// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// WASM bindings for MUSICA using Emscripten

#include <musica/micm/micm.hpp>
#include <musica/micm/micm_c_interface.hpp>
#include <musica/micm/parse.hpp>
#include <musica/util.hpp>
#include <musica/version.hpp>

#include <micm/version.hpp>

#include <emscripten/bind.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(musica_module)
{
  function("getVersion", optional_override([]() { return std::string(musica::GetMusicaVersion()); }));

  function("getMicmVersion", optional_override([]() { return std::string(micm::GetMicmVersion()); }));

  class_<micm::Conditions>("Condition")
      .constructor<>()
      .constructor<double, double, double>()
      .property("temperature", &micm::Conditions::temperature_)
      .property("pressure", &micm::Conditions::pressure_)
      .property("air_density", &micm::Conditions::air_density_);

  register_vector<double>("VectorDouble");
  register_vector<micm::Conditions>("VectorConditions");
  register_map<std::string, std::size_t>("MapStringSizeT");
  register_map<std::string, std::vector<double>>("MapStringVectorDouble");

  enum_<musica::MICMSolver>("SolverType")
      .value("Rosenbrock", musica::MICMSolver::Rosenbrock)
      .value("BackwardEuler", musica::MICMSolver::BackwardEuler)
      .value("CudaRosenbrock", musica::MICMSolver::CudaRosenbrock)
      .value("RosenbrockStandardOrder", musica::MICMSolver::RosenbrockStandardOrder)
      .value("BackwardEulerStandardOrder", musica::MICMSolver::BackwardEulerStandardOrder);

  enum_<micm::SolverState>("SolverState")
      .value("NotYetCalled", micm::SolverState::NotYetCalled)
      .value("Running", micm::SolverState::Running)
      .value("Converged", micm::SolverState::Converged)
      .value("ConvergenceExceededMaxSteps", micm::SolverState::ConvergenceExceededMaxSteps)
      .value("StepSizeTooSmall", micm::SolverState::StepSizeTooSmall)
      .value("RepeatedlySingularMatrix", micm::SolverState::RepeatedlySingularMatrix)
      .value("NaNDetected", micm::SolverState::NaNDetected)
      .value("InfDetected", micm::SolverState::InfDetected)
      .value("AcceptingUnconvergedIntegration", micm::SolverState::AcceptingUnconvergedIntegration);

  value_object<musica::SolverResultStats>("SolverResultsStats")
      .field("function_calls", &musica::SolverResultStats::function_calls_)
      .field("jacobian_updates", &musica::SolverResultStats::jacobian_updates_)
      .field("number_of_steps", &musica::SolverResultStats::number_of_steps_)
      .field("accepted", &musica::SolverResultStats::accepted_)
      .field("rejected", &musica::SolverResultStats::rejected_)
      .field("decompositions", &musica::SolverResultStats::decompositions_)
      .field("solves", &musica::SolverResultStats::solves_)
      .field("final_time", &musica::SolverResultStats::final_time_);

  class_<micm::SolverResult>("SolverResult")
      .constructor<micm::SolverState, musica::SolverResultStats>()
      .property("state", optional_override([](const micm::SolverResult& r) { return static_cast<int>(r.state_); }))
      .property("stats", &micm::SolverResult::stats_);

  class_<musica::State>("State")
      .smart_ptr<std::shared_ptr<musica::State>>("State")
      .function("number_of_grid_cells", &musica::State::NumberOfGridCells)
      .function("set_conditions", &musica::State::SetConditions)
      .function(
          "get_conditions",
          optional_override(
              [](std::shared_ptr<musica::State> state)
              {
                const std::vector<micm::Conditions>& cppVec = state->GetConditions();
                emscripten::val result = emscripten::val::array();

                for (size_t i = 0; i < cppVec.size(); ++i)
                {
                  emscripten::val cond = emscripten::val::object();
                  cond.set("temperature", cppVec[i].temperature_);
                  cond.set("pressure", cppVec[i].pressure_);
                  cond.set("air_density", cppVec[i].air_density_);
                  result.call<void>("push", cond);
                }

                return result;
              }))
      .function(
          "get_concentrations",
          optional_override(
              [](std::shared_ptr<musica::State> state, musica::MICMSolver solver)
              {
                std::map<std::string, std::vector<double>> cppMap = state->GetConcentrations(solver);
                emscripten::val result = emscripten::val::object();
                for (auto& [key, vec] : cppMap)
                {
                  emscripten::val jsArray = emscripten::val::array();
                  for (double v : vec)
                  {
                    jsArray.call<void>("push", v);
                  }
                  result.set(key, jsArray);
                }
                return result;
              }))
      .function(
          "set_concentrations",
          optional_override(
              [](std::shared_ptr<musica::State> state, emscripten::val input, musica::MICMSolver solver)
              {
                std::map<std::string, std::vector<double>> cppMap;

                emscripten::val keys = emscripten::val::global("Object").call<emscripten::val>("keys", input);
                int len = keys["length"].as<int>();
                for (int i = 0; i < len; ++i)
                {
                  std::string key = keys[i].as<std::string>();
                  emscripten::val jsArray = input[key];
                  cppMap[key] = emscripten::vecFromJSArray<double>(jsArray);
                }

                state->SetConcentrations(cppMap, solver);
              }))
      .function(
          "set_user_defined_constants",
          optional_override(
              [](std::shared_ptr<musica::State> state, emscripten::val input, musica::MICMSolver solver)
              {
                std::map<std::string, std::vector<double>> cppMap;

                emscripten::val keys = emscripten::val::global("Object").call<emscripten::val>("keys", input);
                int len = keys["length"].as<int>();
                for (int i = 0; i < len; ++i)
                {
                  std::string key = keys[i].as<std::string>();
                  emscripten::val jsArray = input[key];
                  cppMap[key] = emscripten::vecFromJSArray<double>(jsArray);
                }

                state->SetRateConstants(cppMap, solver);
              }))
      .function(
          "get_user_defined_constants",
          optional_override(
              [](std::shared_ptr<musica::State> state, musica::MICMSolver solver)
              {
                std::map<std::string, std::vector<double>> cppMap = state->GetRateConstants(solver);
                emscripten::val result = emscripten::val::object();
                for (auto& [key, vec] : cppMap)
                {
                  emscripten::val jsArray = emscripten::val::array();
                  for (double v : vec)
                  {
                    jsArray.call<void>("push", v);
                  }
                  result.set(key, jsArray);
                }
                return result;
              }));

  class_<musica::MICM>("MICM")
      .smart_ptr<std::shared_ptr<musica::MICM>>("MICM")
      .class_function(
          "fromConfigPath",
          optional_override([](std::string path, musica::MICMSolver solver)
                            { return std::make_unique<musica::MICM>(path, solver); }))
      .class_function(
          "fromConfigString",
          optional_override(
              [](std::string config_string, musica::MICMSolver solver)
              { return std::make_unique<musica::MICM>(musica::ReadConfigurationFromString(config_string), solver); }))
      .function(
          "solve",
          optional_override(
              [](musica::MICM& micm, std::shared_ptr<musica::State> state, double dt)
              {
                return micm.Solve(state.get(), dt);  // pass raw pointer internally
              }))
      .function("get_maximum_number_of_grid_cells", &musica::MICM::GetMaximumNumberOfGridCells);

  function("vector_size", musica::GetVectorSize);

  function(
      "create_state",
      optional_override([](musica::MICM& micm, std::size_t number_of_grid_cells)
                        { return std::make_shared<musica::State>(micm, number_of_grid_cells); }));

  function(
      "species_ordering",
      optional_override(
          [](std::shared_ptr<musica::State> state)
          {
            std::map<std::string, std::size_t> map;
            std::visit([&map](auto& s) { map = s.variable_map_; }, state->state_variant_);
            return map;
          }));

  function(
      "user_defined_rate_parameters_ordering",
      optional_override(
          [](std::shared_ptr<musica::State> state)
          {
            std::map<std::string, std::size_t> map;
            std::visit([&map](auto& s) { map = s.custom_rate_parameter_map_; }, state->state_variant_);
            return map;
          }));
}
