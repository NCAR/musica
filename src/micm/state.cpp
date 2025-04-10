// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the State class.It includes state representations for different
// solver configurations, leveraging both vector-ordered and standard-ordered state type.
// It also includes functions for creating and deleting State instances with c bindings.
#include <musica/micm/micm.hpp>
#include <musica/micm/state.hpp>
#include <musica/util.hpp>

#include <micm/solver/rosenbrock_solver_parameters.hpp>
#include <micm/solver/solver_builder.hpp>
#include <micm/system/species.hpp>
#include <micm/version.hpp>

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <string>
#include <system_error>

namespace musica
{
  State* CreateMicmState(musica::MICM* micm, Error* error)
  {
    DeleteError(error);
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
  }

  void DeleteState(const State* state, Error* error)
  {
    DeleteError(error);
    if (state == nullptr)
    {
      *error = NoError();
      return;
    }
    try
    {
      delete state;
      *error = NoError();
    }
    catch (const std::system_error& e)
    {
      *error = ToError(e);
    }
  }

  std::vector<micm::Conditions>& State::GetConditions()
  {
    return std::visit([](auto& st) -> std::vector<micm::Conditions>& { return st.conditions_; }, state_variant_);
  }

  void State::SetConditions(const std::vector<micm::Conditions>& conditions)
  {
    std::visit(
        [&](auto& st)
        {
          for (size_t i = 0; i < conditions.size(); ++i)
          {
            st.conditions_[i] = conditions[i];
          }
        },
        state_variant_);
  }

  std::vector<double>& State::GetOrderedConcentrations()
  {
    return std::visit([](auto& st) -> std::vector<double>& { return st.variables_.AsVector(); }, state_variant_);
  }

  void State::SetOrderedConcentrations(const std::vector<double>& concentrations)
  {
    std::visit(
        [&](auto& st)
        {
          for (size_t i = 0; i < concentrations.size(); ++i)
          {
            st.variables_.AsVector()[i] = concentrations[i];
          }
        },
        state_variant_);
  }

  std::vector<double>& State::GetOrderedRateConstants()
  {
    return std::visit(
        [](auto& st) -> std::vector<double>& { return st.custom_rate_parameters_.AsVector(); }, state_variant_);
  }

  void State::SetOrderedRateConstants(const std::vector<double>& rateConstant)
  {
    std::visit(
        [&](auto& st)
        {
          for (size_t i = 0; i < rateConstant.size(); ++i)
          {
            st.custom_rate_parameters_.AsVector()[i] = rateConstant[i];
          }
        },
        state_variant_);
  }

  ConditionsVector* GetConditionsFromState(musica::State* state)
  {
    return new ConditionsVector(state->GetConditions());
  }

  void SetConditionsToState(musica::State* state, const micm::Conditions* conditions_array, std::size_t size)
  {
      std::vector<micm::Conditions> conditions_vec(conditions_array, conditions_array + size);
      state->SetConditions(conditions_vec);
  }
  
  double* GetOrderedConcentrationsToStateFortran(musica::State* state, int* number_of_species, int* number_of_grid_cells, Error* error)
  {
    return state->GetOrderedConcentrationsToState(state, number_of_species, number_of_grid_cells, error);
  }
  
  double* State::GetOrderedConcentrationsToState(musica::State* state, int* number_of_species, int* number_of_grid_cells, Error* error)
  {
    auto& vec = std::visit([](auto& st) -> std::vector<double>& {
      return st.variables_.AsVector();
    }, state_variant_);

    *number_of_grid_cells = std::visit([](auto& st) -> int {
      return static_cast<int>(st.conditions_.size());
    }, state_variant_);

    *number_of_species = static_cast<int>(vec.size() / *number_of_grid_cells);
    
    return vec.data();
  }

  double* GetOrderedRateConstantsToStateFortran(musica::State* state, int* number_of_rate_constants, int* number_of_grid_cells, Error* error)
  {
    return state->GetOrderedRateConstantsToState(state, number_of_rate_constants, number_of_grid_cells, error);
  }

  double* State::GetOrderedRateConstantsToState(musica::State* state, int* number_of_rate_constants, int* number_of_grid_cells, Error* error)
  {
    auto& vec = std::visit([](auto& st) -> std::vector<double>& {
      return st.custom_rate_parameters_.AsVector();
    }, state_variant_);

    *number_of_grid_cells = std::visit([](auto& st) -> int {
      return static_cast<int>(st.conditions_.size());
    }, state_variant_);

    *number_of_rate_constants = static_cast<int>(vec.size() / *number_of_grid_cells);
    return vec.data();
  }

  std::size_t GetConditionsSize(const ConditionsVector* vec)
  {
    return vec->size();
  }
}  // namespace musica
