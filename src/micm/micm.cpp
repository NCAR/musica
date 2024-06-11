/* Copyright (C) 2023-2024 National Center for Atmospheric Research
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * This file contains the implementation of the MICM class, which represents a
 * multi-component reactive transport model. It also includes functions for
 * creating and deleting MICM instances, creating solvers, and solving the model.
 */
#include <musica/micm.hpp>

#include <micm/version.hpp>
#include <micm/solver/rosenbrock_solver_parameters.hpp>
#include <micm/system/species.hpp>

#include <cmath>
#include <filesystem>
#include <iostream>

namespace musica
{

  String get_micm_version()
  {
    String micm_version = CreateString(micm::GetMicmVersion());
    return micm_version;
  }

  MICM *CreateMicm(const char *config_path, Error *error)
  {
    DeleteError(error);
    MICM *micm = new MICM();
    micm->Create(std::string(config_path), error);
    if (!IsSuccess(*error))
    {
      delete micm;
      return nullptr;
    }
    return micm;
  }

  void DeleteMicm(const MICM *micm, Error *error)
  {
    DeleteError(error);
    if (micm == nullptr)
    {
      *error = NoError();
      return;
    }
    try
    {
      delete micm;
      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
  }

  void MicmSolve(
      MICM *micm,
      double time_step,
      double temperature,
      double pressure,
      int num_concentrations,
      double *concentrations,
      int num_custom_rate_parameters,
      double *custom_rate_parameters,
      Error *error)
  {
    DeleteError(error);
    micm->Solve(
        time_step,
        temperature,
        pressure,
        num_concentrations,
        concentrations,
        num_custom_rate_parameters,
        custom_rate_parameters,
        error);
  }

  Mapping *GetSpeciesOrdering(MICM *micm, size_t *array_size, Error *error)
  {
    DeleteError(error);
    auto map = micm->GetSpeciesOrdering(error);
    if (!IsSuccess(*error))
    {
      return nullptr;
    }
    Mapping *species_ordering = new Mapping[map.size()];

    // Copy data from the map to the array of structs
    size_t i = 0;
    for (const auto &entry : map)
    {
      species_ordering[i] = ToMapping(entry.first.c_str(), entry.second);
      ++i;
    }

    // Set the size of the array
    *array_size = map.size();
    return species_ordering;
  }

  Mapping *GetUserDefinedReactionRatesOrdering(MICM *micm, size_t *array_size, Error *error)
  {
    DeleteError(error);
    auto map = micm->GetUserDefinedReactionRatesOrdering(error);
    if (!IsSuccess(*error))
    {
      return nullptr;
    }
    Mapping *reactionRates = new Mapping[map.size()];

    // Copy data from the map to the array of structs
    size_t i = 0;
    for (const auto &entry : map)
    {
      reactionRates[i] = ToMapping(entry.first.c_str(), entry.second);
      ++i;
    }

    // Set the size of the array
    *array_size = map.size();
    return reactionRates;
  }

  String GetSpeciesPropertyString(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    DeleteError(error);
    std::string species_name_str(species_name);
    std::string property_name_str(property_name);
    const std::string value_str = micm->GetSpeciesProperty<std::string>(species_name_str, property_name_str, error);
    String value;
    if (!IsSuccess(*error))
    {
      return value;
    }
    return CreateString(value_str.c_str());
  }

  double GetSpeciesPropertyDouble(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    DeleteError(error);
    std::string species_name_str(species_name);
    std::string property_name_str(property_name);
    return micm->GetSpeciesProperty<double>(species_name_str, property_name_str, error);
  }

  int GetSpeciesPropertyInt(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    DeleteError(error);
    std::string species_name_str(species_name);
    std::string property_name_str(property_name);
    return micm->GetSpeciesProperty<int>(species_name_str, property_name_str, error);
  }

  bool GetSpeciesPropertyBool(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    DeleteError(error);
    std::string species_name_str(species_name);
    std::string property_name_str(property_name);
    return micm->GetSpeciesProperty<bool>(species_name_str, property_name_str, error);
  }

  void MICM::Create(const std::string &config_path, Error *error)
  {
    try
    {
      micm::SolverConfig<> solver_config;
      solver_config.ReadAndParse(std::filesystem::path(config_path));
      solver_parameters_ = std::make_unique<micm::SolverParameters>(solver_config.GetSolverParams());
      auto params = micm::RosenbrockSolverParameters::ThreeStageRosenbrockParameters(NUM_GRID_CELLS);
      params.ignore_unused_species_ = true;
      solver_ =
          std::make_unique<micm::RosenbrockSolver<>>(solver_parameters_->system_, solver_parameters_->processes_, params);
      DeleteError(error);
      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      DeleteError(error);
      *error = ToError(e);
    }
  }

  void MICM::Solve(
      double time_step,
      double temperature,
      double pressure,
      int num_concentrations,
      double *concentrations,
      int num_custom_rate_parameters,
      double *custom_rate_parameters,
      Error *error)
  {
    try
    {
      micm::State state = solver_->GetState();

      for (size_t i{}; i < NUM_GRID_CELLS; i++)
      {
        state.conditions_[i].temperature_ = temperature;
        state.conditions_[i].pressure_ = pressure;
      }

      state.variables_.AsVector().assign(concentrations, concentrations + num_concentrations);

      state.custom_rate_parameters_.AsVector().assign(
          custom_rate_parameters, custom_rate_parameters + num_custom_rate_parameters);

      auto result = solver_->Solve(time_step, state);

      for (int i = 0; i < result.result_.AsVector().size(); i++)
      {
        concentrations[i] = result.result_.AsVector()[i];
      }
      DeleteError(error);
      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      DeleteError(error);
      *error = ToError(e);
    }
  }

  std::map<std::string, size_t> MICM::GetSpeciesOrdering(Error *error)
  {
    try
    {
      micm::State state = solver_->GetState();
      DeleteError(error);
      *error = NoError();
      return state.variable_map_;
    }
    catch (const std::system_error &e)
    {
      DeleteError(error);
      *error = ToError(e);
      return std::map<std::string, size_t>();
    }
  }

  std::map<std::string, size_t> MICM::GetUserDefinedReactionRatesOrdering(Error *error)
  {
    try
    {
      micm::State state = solver_->GetState();
      DeleteError(error);
      *error = NoError();
      return state.custom_rate_parameter_map_;
    }
    catch (const std::system_error &e)
    {
      DeleteError(error);
      *error = ToError(e);
      return std::map<std::string, size_t>();
    }
  }

}  // namespace musica
