// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the State class.It includes state representations for different
// solver configurations, leveraging both vector-ordered and standard-ordered state type.
// It also includes functions for creating and deleting State instances with c bindings.
#include <musica/micm/micm.hpp>
#include <musica/micm/micm_c_interface.hpp>
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

  void State::SetConcentrations(const std::map<std::string, std::vector<double>>& input, musica::MICMSolver solver_type)
  {
    std::visit(
        [&](auto& st)
        {
          std::size_t vector_size_ = musica::GetVectorSize(solver_type);
          size_t n_species = st.variable_map_.size();
          for (const auto& [name, values] : input)
          {
            auto it = st.variable_map_.find(name);
            if (it == st.variable_map_.end())
              continue;
            size_t i_species = it->second;
            for (size_t i_cell = 0; i_cell < values.size(); ++i_cell)
            {
              size_t group_index = i_cell / vector_size_;
              size_t row_in_group = i_cell % vector_size_;
              size_t idx = (group_index * n_species + i_species) * vector_size_ + row_in_group;
              st.variables_.AsVector()[idx] = values[i_cell];
            }
          }
        },
        state_variant_);
  }

  std::map<std::string, std::vector<double>> State::GetConcentrations(musica::MICMSolver solver_type) const
  {
    return std::visit(
        [&](auto& st)
        {
          std::size_t vector_size_ = musica::GetVectorSize(solver_type);
          std::map<std::string, std::vector<double>> output;
          size_t n_species = st.variable_map_.size();
          for (const auto& [name, i_species] : st.variable_map_)
          {
            output[name] = std::vector<double>(st.NumberOfGridCells());
            for (size_t i_cell = 0; i_cell < st.NumberOfGridCells(); ++i_cell)
            {
              size_t group_index = i_cell / vector_size_;
              size_t row_in_group = i_cell % vector_size_;
              size_t idx = (group_index * n_species + i_species) * vector_size_ + row_in_group;
              output[name][i_cell] = st.variables_.AsVector()[idx];
            }
          }
          return output;
        },
        state_variant_);
  }

  void State::SetRateConstants(const std::map<std::string, std::vector<double>>& input, musica::MICMSolver solver_type)
  {
    std::visit(
        [&](auto& st)
        {
          std::size_t vector_size_ = musica::GetVectorSize(solver_type);
          size_t n_params = st.custom_rate_parameter_map_.size();
          for (const auto& [name, values] : input)
          {
            auto it = st.custom_rate_parameter_map_.find(name);
            if (it == st.custom_rate_parameter_map_.end())
              continue;
            size_t i_param = it->second;
            for (size_t i_cell = 0; i_cell < values.size(); ++i_cell)
            {
              size_t group_index = i_cell / vector_size_;
              size_t row_in_group = i_cell % vector_size_;
              size_t idx = (group_index * n_params + i_param) * vector_size_ + row_in_group;
              st.custom_rate_parameters_.AsVector()[idx] = values[i_cell];
            }
          }
        },
        state_variant_);
  }

  std::map<std::string, std::vector<double>> State::GetRateConstants(musica::MICMSolver solver_type) const
  {
    return std::visit(
        [&](auto& st)
        {
          std::size_t vector_size_ = musica::GetVectorSize(solver_type);
          std::map<std::string, std::vector<double>> output;
          size_t n_params = st.custom_rate_parameter_map_.size();
          for (const auto& [name, i_param] : st.custom_rate_parameter_map_)
          {
            output[name] = std::vector<double>(st.NumberOfGridCells());
            for (size_t i_cell = 0; i_cell < st.NumberOfGridCells(); ++i_cell)
            {
              size_t group_index = i_cell / vector_size_;
              size_t row_in_group = i_cell % vector_size_;
              size_t idx = (group_index * n_params + i_param) * vector_size_ + row_in_group;
              output[name][i_cell] = st.custom_rate_parameters_.AsVector()[idx];
            }
          }
          return output;
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
