// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the State class.It includes state representations for different
// solver configurations, leveraging both vector-ordered and standard-ordered state type.
// It also includes functions for creating and deleting State instances with c bindings.
#include <musica/micm/state_c_interface.hpp>

namespace musica
{
  template<typename Func>
  auto HandleErrors(Func func, Error* error) -> decltype(func())
  {
    DeleteError(error);
    try
    {
      return func();
    }
    catch (const std::system_error& e)
    {
      *error = ToError(e);
    }
    catch (const std::exception& e)
    {
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_UNKNOWN, e.what());
    }
    return decltype(func())();
  }

  State* CreateMicmState(musica::MICM* micm, std::size_t number_of_grid_cells, Error* error)
  {
    return HandleErrors(
        [&]() -> State*
        {
          if (!micm)
          {
            std::string msg = "MICM pointer is null, cannot create state.";
            *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND, msg.c_str());
            delete micm;
            return nullptr;
          }

          State* state = new State(*micm, number_of_grid_cells);

          return state;
        },
        error);
  }

  void DeleteState(const State* state, Error* error)
  {
    HandleErrors(
        [&]() -> void
        {
          if (state == nullptr)
          {
            std::string msg = "State pointer is null, cannot delete state.";
            *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND, msg.c_str());
            return;
          }

          delete state;
        },
        error);
  }

  micm::Conditions* GetConditionsPointer(musica::State* state, size_t* array_size, Error* error)
  {
    return HandleErrors(
        [&]() -> micm::Conditions*
        {
          std::vector<micm::Conditions>& conditions = state->GetConditions();
          *array_size = conditions.size();
          return conditions.data();
        },
        error);
  }

  double* GetOrderedConcentrationsPointer(musica::State* state, size_t* array_size, Error* error)
  {
    return HandleErrors(
        [&]() -> double*
        {
          std::vector<double>& concentrations = state->GetOrderedConcentrations();
          *array_size = concentrations.size();
          return concentrations.data();
        },
        error);
  }

  double* GetOrderedRateParametersPointer(musica::State* state, size_t* array_size, Error* error)
  {
    return HandleErrors(
        [&]() -> double*
        {
          std::vector<double>& rate_constants = state->GetOrderedRateParameters();
          *array_size = rate_constants.size();
          return rate_constants.data();
        },
        error);
  }

  Mappings GetSpeciesOrdering(musica::State* state, Error* error)
  {
    return HandleErrors(
        [&]()
        {
          Mappings species_ordering;
          std::map<std::string, std::size_t> map =
              std::visit([](auto& state) { return state.variable_map_; }, state->state_variant_);

          species_ordering.mappings_ = new Mapping[map.size()];
          species_ordering.size_ = map.size();

          std::size_t i = 0;
          for (const auto& entry : map)
          {
            species_ordering.mappings_[i] = ToMapping(entry.first.c_str(), entry.second);
            ++i;
          }

          *error = NoError();
          return species_ordering;
        },
        error);
  }

  Mappings GetUserDefinedRateParametersOrdering(musica::State* state, Error* error)
  {
    return HandleErrors(
        [&]()
        {
          Mappings reaction_rates;
          std::map<std::string, std::size_t> map =
              std::visit([](auto& state) { return state.custom_rate_parameter_map_; }, state->state_variant_);

          reaction_rates.mappings_ = new Mapping[map.size()];
          reaction_rates.size_ = map.size();

          std::size_t i = 0;
          for (const auto& entry : map)
          {
            reaction_rates.mappings_[i] = ToMapping(entry.first.c_str(), entry.second);
            ++i;
          }

          *error = NoError();
          return reaction_rates;
        },
        error);
  }

  size_t GetNumberOfGridCells(musica::State* state, Error* error)
  {
    return HandleErrors(
        [&]() -> size_t
        {
          size_t number_of_grid_cells = state->NumberOfGridCells();
          *error = NoError();
          return number_of_grid_cells;
        },
        error);
  }

  size_t GetNumberOfSpecies(musica::State* state, Error* error)
  {
    return HandleErrors(
        [&]() -> size_t
        {
          size_t number_of_species = state->NumberOfSpecies();
          *error = NoError();
          return number_of_species;
        },
        error);
  }

  void GetConcentrationsStrides(musica::State* state, Error* error, size_t* grid_cell_stride, size_t* species_stride)
  {
    HandleErrors(
        [&]() -> void
        {
          auto strides = state->GetConcentrationsStrides();
          *grid_cell_stride = strides.first;
          *species_stride = strides.second;
          *error = NoError();
        },
        error);
  }

  size_t GetNumberOfUserDefinedRateParameters(musica::State* state, Error* error)
  {
    return HandleErrors(
        [&]() -> size_t
        {
          size_t number_of_user_defined_rate_parameters = state->NumberOfUserDefinedRateParameters();
          *error = NoError();
          return number_of_user_defined_rate_parameters;
        },
        error);
  }

  void GetUserDefinedRateParametersStrides(
      musica::State* state,
      Error* error,
      size_t* grid_cell_stride,
      size_t* rate_parameter_stride)
  {
    HandleErrors(
        [&]() -> void
        {
          auto strides = state->GetUserDefinedRateParametersStrides();
          *grid_cell_stride = strides.first;
          *rate_parameter_stride = strides.second;
          *error = NoError();
        },
        error);
  }

}  // namespace musica
