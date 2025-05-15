// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the State class.It includes state representations for different
// solver configurations, leveraging both vector-ordered and standard-ordered state type.
// It also includes functions for creating and deleting State instances with c bindings.
#include <musica/micm/micm.hpp>
#include <musica/micm/state.hpp>
#include <musica/util.hpp>

#include <cmath>
#include <string>

namespace musica
{
  State::State(const musica::MICM& micm, std::size_t number_of_grid_cells)
  {
    std::visit(
        [&](auto& solver_ptr) { this->state_variant_ = solver_ptr->GetState(number_of_grid_cells); }, micm.solver_variant_);
  }

  std::size_t State::NumberOfGridCells()
  {
    return std::visit([](auto& st) -> std::size_t { return st.NumberOfGridCells(); }, state_variant_);
  }

  std::size_t State::NumberOfSpecies()
  {
    return std::visit([](auto& st) -> std::size_t { return st.variables_.NumColumns(); }, state_variant_);
  }

  std::size_t State::NumberOfUserDefinedRateParameters()
  {
    return std::visit([](auto& st) -> std::size_t { return st.custom_rate_parameters_.NumColumns(); }, state_variant_);
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

  std::vector<double>& State::GetOrderedRateParameters()
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

  std::pair<std::size_t, std::size_t> State::GetConcentrationsStrides()
  {
    return std::visit(
        [](auto& st) -> std::pair<std::size_t, std::size_t>
        { return std::make_pair(st.variables_.RowStride(), st.variables_.ColumnStride()); },
        state_variant_);
  }

  std::pair<std::size_t, std::size_t> State::GetUserDefinedRateParametersStrides()
  {
    return std::visit(
        [](auto& st) -> std::pair<std::size_t, std::size_t>
        { return std::make_pair(st.custom_rate_parameters_.RowStride(), st.custom_rate_parameters_.ColumnStride()); },
        state_variant_);
  }

}  // namespace musica
