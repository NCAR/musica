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
      ToError(e, error);
    }
    catch (const std::exception& e)
    {
      ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_UNKNOWN, e.what(), error);
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
            std::string const msg = "MICM pointer is null, cannot create state.";
            ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND, msg.c_str(), error);
            return nullptr;
          }

          State* state = new State(*micm, number_of_grid_cells);

          return state;
        },
        error);
  }

  void DeleteState(State* state, Error* error)
  {
    HandleErrors(
        [&]() -> void
        {
          if (state == nullptr)
          {
            std::string const msg = "State pointer is null, cannot delete state.";
            ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND, msg.c_str(), error);
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

  void GetSpeciesOrdering(musica::State* state, Mappings* species_ordering, Error* error)
  {
    HandleErrors(
        [&]()
        {
          std::map<std::string, std::size_t> const map =
              std::visit([](auto& state) { return state.variable_map_; }, state->state_variant_);

          species_ordering->mappings_ = new Mapping[map.size()];
          species_ordering->size_ = map.size();

          std::size_t i = 0;
          for (const auto& entry : map)
          {
            ToMapping(entry.first.c_str(), entry.second, &species_ordering->mappings_[i]);
            ++i;
          }
          NoError(error);
          return;
        },
        error);
  }

  void GetUserDefinedRateParametersOrdering(musica::State* state, Mappings* reaction_rates, Error* error)
  {
    HandleErrors(
        [&]()
        {
          std::map<std::string, std::size_t> const map =
              std::visit([](auto& state) { return state.custom_rate_parameter_map_; }, state->state_variant_);

          reaction_rates->mappings_ = new Mapping[map.size()];
          reaction_rates->size_ = map.size();

          std::size_t i = 0;
          for (const auto& entry : map)
          {
            ToMapping(entry.first.c_str(), entry.second, &reaction_rates->mappings_[i]);
            ++i;
          }
          NoError(error);
          return;
        },
        error);
  }

  size_t GetNumberOfGridCells(musica::State* state, Error* error)
  {
    return HandleErrors(
        [&]() -> size_t
        {
          size_t const number_of_grid_cells = state->NumberOfGridCells();
          NoError(error);
          return number_of_grid_cells;
        },
        error);
  }

  size_t GetNumberOfSpecies(musica::State* state, Error* error)
  {
    return HandleErrors(
        [&]() -> size_t
        {
          size_t const number_of_species = state->NumberOfSpecies();
          NoError(error);
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
          NoError(error);
        },
        error);
  }

  size_t GetNumberOfUserDefinedRateParameters(musica::State* state, Error* error)
  {
    return HandleErrors(
        [&]() -> size_t
        {
          size_t const number_of_user_defined_rate_parameters = state->NumberOfUserDefinedRateParameters();
          NoError(error);
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
          NoError(error);
        },
        error);
  }

}  // namespace musica
