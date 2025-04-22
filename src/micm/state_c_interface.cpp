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

  State* CreateMicmState(musica::MICM* micm, Error* error)
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

          State* state = new State();

          std::visit([&](auto& solver_ptr) { state->state_variant_ = solver_ptr->GetState(); }, micm->solver_variant_);

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

  micm::Conditions* GetConditionsToStateFortran(musica::State* state, int* number_of_grid_cells, Error* error)
  {
    return HandleErrors(
        [&]() -> micm::Conditions* { return state->GetConditionsToState(state, number_of_grid_cells)->data(); }, error);
  }

  double* GetOrderedConcentrationsToStateFortran(
      musica::State* state,
      int* number_of_species,
      int* number_of_grid_cells,
      Error* error)
  {
    return HandleErrors(
        [&]() -> double* { return state->GetOrderedConcentrationsToState(state, number_of_species, number_of_grid_cells); },
        error);
  }

  double* GetOrderedRateConstantsToStateFortran(
      musica::State* state,
      int* number_of_rate_constants,
      int* number_of_grid_cells,
      Error* error)
  {
    return HandleErrors(
        [&]() -> double*
        { return state->GetOrderedRateConstantsToState(state, number_of_rate_constants, number_of_grid_cells); },
        error);
  }

  Mappings GetSpeciesOrdering(musica::State *state, Error *error)
  {
    return HandleErrors([&]() {
      Mappings species_ordering;
      std::map<std::string, std::size_t> map = std::visit([](auto &state) { return state.variable_map_; }, state->state_variant_);

      species_ordering.mappings_ = new Mapping[map.size()];
      species_ordering.size_ = map.size();

      std::size_t i = 0;
      for (const auto &entry : map)
      {
        species_ordering.mappings_[i] = ToMapping(entry.first.c_str(), entry.second);
        ++i;
      }

      *error = NoError();
      return species_ordering;
    }, error);
  }

  Mappings GetUserDefinedReactionRatesOrdering(musica::State *state, Error *error)
  {
    return HandleErrors([&]() {
      Mappings reaction_rates;
      std::map<std::string, std::size_t> map = std::visit([](auto &state) { return state.custom_rate_parameter_map_; }, state->state_variant_);

      reaction_rates.mappings_ = new Mapping[map.size()];
      reaction_rates.size_ = map.size();

      std::size_t i = 0;
      for (const auto &entry : map)
      {
        reaction_rates.mappings_[i] = ToMapping(entry.first.c_str(), entry.second);
        ++i;
      }

      *error = NoError();
      return reaction_rates;
    }, error);
  }


}  // namespace musica
