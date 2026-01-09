// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// WASM bindings for MUSICA using Emscripten

#include "micm/micm_wrapper.h"
#include "micm/state_wrapper.h"

#include <musica/version.hpp>

#include <micm/version.hpp>

#include <emscripten/bind.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace emscripten;
using namespace musica_addon;

// Wrapper functions to return std::string instead of const char*
std::string GetVersion()
{
  return std::string(musica::GetMusicaVersion());
}

std::string GetMicmVersion()
{
  return std::string(micm::GetMicmVersion());
}

// Helper function to convert JavaScript array to C++ vector
template<typename T>
std::vector<T> jsArrayToVector(const val& jsArray)
{
  std::vector<T> vec;
  unsigned int length = jsArray["length"].as<unsigned int>();
  vec.reserve(length);
  for (unsigned int i = 0; i < length; ++i)
  {
    vec.push_back(jsArray[i].as<T>());
  }
  return vec;
}

// ============================================================================
// StateWrapper bindings
// ============================================================================

class StateWrapperWASM
{
 public:
  // Constructor - takes ownership of the state pointer via StateWrapper
  explicit StateWrapperWASM(musica::State* state)
      : wrapper_(std::make_unique<StateWrapper>(state))
  {
  }

  // Move constructor and assignment
  StateWrapperWASM(StateWrapperWASM&& other) = default;
  StateWrapperWASM& operator=(StateWrapperWASM&& other) = default;

  // Delete copy constructor and assignment
  StateWrapperWASM(const StateWrapperWASM&) = delete;
  StateWrapperWASM& operator=(const StateWrapperWASM&) = delete;

  void setConcentrations(const val& concentrations)
  {
    std::map<std::string, std::vector<double>> conc_map;
    // Convert JavaScript object to C++ map
    auto keys = val::global("Object").call<val>("keys", concentrations);
    unsigned int length = keys["length"].as<unsigned int>();
    for (unsigned int i = 0; i < length; ++i)
    {
      std::string key = keys[i].as<std::string>();
      val value = concentrations[key];
      std::vector<double> vec = jsArrayToVector<double>(value);
      conc_map[key] = vec;
    }
    wrapper_->SetConcentrations(conc_map);
  }

  val getConcentrations()
  {
    auto conc_map = wrapper_->GetConcentrations();
    val result = val::object();
    for (const auto& pair : conc_map)
    {
      result.set(pair.first, val::array(pair.second.begin(), pair.second.end()));
    }
    return result;
  }

  void setUserDefinedRateParameters(const val& params)
  {
    std::map<std::string, std::vector<double>> param_map;
    auto keys = val::global("Object").call<val>("keys", params);
    unsigned int length = keys["length"].as<unsigned int>();
    for (unsigned int i = 0; i < length; ++i)
    {
      std::string key = keys[i].as<std::string>();
      val value = params[key];
      std::vector<double> vec = jsArrayToVector<double>(value);
      param_map[key] = vec;
    }
    wrapper_->SetUserDefinedRateParameters(param_map);
  }

  val getUserDefinedRateParameters()
  {
    auto param_map = wrapper_->GetUserDefinedRateParameters();
    val result = val::object();
    for (const auto& pair : param_map)
    {
      result.set(pair.first, val::array(pair.second.begin(), pair.second.end()));
    }
    return result;
  }

  void setConditions(const val& conditions)
  {
    const std::vector<double>* temperatures = nullptr;
    const std::vector<double>* pressures = nullptr;
    const std::vector<double>* air_densities = nullptr;

    std::vector<double> temp_vec, press_vec, air_vec;

    if (conditions.hasOwnProperty("temperatures"))
    {
      temp_vec = jsArrayToVector<double>(conditions["temperatures"]);
      temperatures = &temp_vec;
    }
    if (conditions.hasOwnProperty("pressures"))
    {
      press_vec = jsArrayToVector<double>(conditions["pressures"]);
      pressures = &press_vec;
    }
    if (conditions.hasOwnProperty("air_densities"))
    {
      air_vec = jsArrayToVector<double>(conditions["air_densities"]);
      air_densities = &air_vec;
    }

    wrapper_->SetConditions(temperatures, pressures, air_densities);
  }

  val getConditions()
  {
    auto cond_map = wrapper_->GetConditions();
    val result = val::object();
    for (const auto& pair : cond_map)
    {
      result.set(pair.first, val::array(pair.second.begin(), pair.second.end()));
    }
    return result;
  }

  val getSpeciesOrdering()
  {
    auto ordering = wrapper_->GetSpeciesOrdering();
    val result = val::object();
    for (const auto& pair : ordering)
    {
      result.set(pair.first, val(pair.second));
    }
    return result;
  }

  val getUserDefinedRateParametersOrdering()
  {
    auto ordering = wrapper_->GetUserDefinedRateParametersOrdering();
    val result = val::object();
    for (const auto& pair : ordering)
    {
      result.set(pair.first, val(pair.second));
    }
    return result;
  }

  size_t getNumberOfGridCells()
  {
    return wrapper_->GetNumberOfGridCells();
  }

  val concentrationStrides()
  {
    size_t cell_stride, species_stride;
    wrapper_->GetConcentrationStrides(cell_stride, species_stride);
    val result = val::object();
    result.set("cell_stride", val(cell_stride));
    result.set("species_stride", val(species_stride));
    return result;
  }

  val userDefinedRateParameterStrides()
  {
    size_t cell_stride, param_stride;
    wrapper_->GetUserDefinedRateParameterStrides(cell_stride, param_stride);
    val result = val::object();
    result.set("cell_stride", val(cell_stride));
    result.set("param_stride", val(param_stride));
    return result;
  }

  // Make wrapper accessible for MICMWrapperWASM
  StateWrapper& getWrapper()
  {
    return *wrapper_;
  }

 private:
  std::unique_ptr<StateWrapper> wrapper_;
};

// ============================================================================
// MICMWrapper bindings
// ============================================================================

class MICMWrapperWASM
{
 public:
  // Constructor
  explicit MICMWrapperWASM(std::unique_ptr<MICMWrapper> wrapper)
      : wrapper_(std::move(wrapper))
  {
  }

  // Move constructor and assignment
  MICMWrapperWASM(MICMWrapperWASM&& other) = default;
  MICMWrapperWASM& operator=(MICMWrapperWASM&& other) = default;

  // Delete copy constructor and assignment
  MICMWrapperWASM(const MICMWrapperWASM&) = delete;
  MICMWrapperWASM& operator=(const MICMWrapperWASM&) = delete;

  static std::shared_ptr<MICMWrapperWASM> fromConfigPath(const std::string& config_path, int solver_type)
  {
    auto wrapper = MICMWrapper::FromConfigPath(config_path, solver_type);
    return std::make_shared<MICMWrapperWASM>(std::move(wrapper));
  }

  static std::shared_ptr<MICMWrapperWASM> fromConfigString(const std::string& config_string, int solver_type)
  {
    auto wrapper = MICMWrapper::FromConfigString(config_string, solver_type);
    return std::make_shared<MICMWrapperWASM>(std::move(wrapper));
  }

  std::shared_ptr<StateWrapperWASM> createState(size_t number_of_grid_cells)
  {
    musica::State* state = wrapper_->CreateState(number_of_grid_cells);
    return std::make_shared<StateWrapperWASM>(state);
  }

  val solve(StateWrapperWASM& state, double time_step)
  {
    // Note: We need to extract the raw state pointer to pass to the underlying
    // MICM solver. The StateWrapperWASM maintains ownership of the state.
    // This is safe as long as the state outlives this solve call.
    musica::State* raw_state = state.getWrapper().GetState();
    auto result = wrapper_->Solve(raw_state, time_step);

    // Convert SolverResult to JavaScript object
    val js_result = val::object();
    js_result.set("state", val(static_cast<int>(result.state_)));

    val stats = val::object();
    stats.set("function_calls", val(result.stats_.function_calls_));
    stats.set("jacobian_updates", val(result.stats_.jacobian_updates_));
    stats.set("number_of_steps", val(result.stats_.number_of_steps_));
    stats.set("accepted", val(result.stats_.accepted_));
    stats.set("rejected", val(result.stats_.rejected_));
    stats.set("decompositions", val(result.stats_.decompositions_));
    stats.set("solves", val(result.stats_.solves_));
    stats.set("final_time", val(result.stats_.final_time_));

    js_result.set("stats", stats);

    return js_result;
  }

  int solverType() const
  {
    return wrapper_->GetSolverType();
  }

 private:
  std::unique_ptr<MICMWrapper> wrapper_;
};

EMSCRIPTEN_BINDINGS(musica_module)
{
  function("getVersion", &GetVersion);
  function("getMicmVersion", &GetMicmVersion);

  class_<StateWrapperWASM>("State")
      .smart_ptr<std::shared_ptr<StateWrapperWASM>>("StatePtr")
      .function("setConcentrations", &StateWrapperWASM::setConcentrations)
      .function("getConcentrations", &StateWrapperWASM::getConcentrations)
      .function("setUserDefinedRateParameters", &StateWrapperWASM::setUserDefinedRateParameters)
      .function("getUserDefinedRateParameters", &StateWrapperWASM::getUserDefinedRateParameters)
      .function("setConditions", &StateWrapperWASM::setConditions)
      .function("getConditions", &StateWrapperWASM::getConditions)
      .function("getSpeciesOrdering", &StateWrapperWASM::getSpeciesOrdering)
      .function("getUserDefinedRateParametersOrdering", &StateWrapperWASM::getUserDefinedRateParametersOrdering)
      .function("getNumberOfGridCells", &StateWrapperWASM::getNumberOfGridCells)
      .function("concentrationStrides", &StateWrapperWASM::concentrationStrides)
      .function("userDefinedRateParameterStrides", &StateWrapperWASM::userDefinedRateParameterStrides);

  class_<MICMWrapperWASM>("MICM")
      .smart_ptr<std::shared_ptr<MICMWrapperWASM>>("MICMPtr")
      .class_function("fromConfigPath", &MICMWrapperWASM::fromConfigPath)
      .class_function("fromConfigString", &MICMWrapperWASM::fromConfigString)
      .function("createState", &MICMWrapperWASM::createState)
      .function("solve", &MICMWrapperWASM::solve)
      .function("solverType", &MICMWrapperWASM::solverType);
}
