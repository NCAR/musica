// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the State class. It includes state representations for different
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
  State::State(std::unique_ptr<IState> impl)
      : impl_(std::move(impl))
  {
  }

  State::State(const musica::MICM& micm, std::size_t number_of_grid_cells)
      : impl_(const_cast<musica::MICM&>(micm).CreateState(number_of_grid_cells))
  {
  }

  std::size_t State::NumberOfGridCells()
  {
    return impl_->NumberOfGridCells();
  }

  std::size_t State::NumberOfSpecies()
  {
    return impl_->NumberOfSpecies();
  }

  std::size_t State::NumberOfUserDefinedRateParameters()
  {
    return impl_->NumberOfUserDefinedRateParameters();
  }

  std::vector<micm::Conditions>& State::GetConditions()
  {
    return impl_->GetConditions();
  }

  void State::SetConditions(const std::vector<micm::Conditions>& conditions)
  {
    auto& state_conditions = impl_->GetConditions();
    for (size_t i = 0; i < conditions.size(); ++i)
    {
      state_conditions[i] = conditions[i];
    }
  }

  void State::SetConcentrations(const std::map<std::string, std::vector<double>>& input, musica::MICMSolver solver_type)
  {
    std::size_t vector_size_ = musica::GetVectorSize(solver_type);
    auto variable_map = impl_->GetVariableMap();
    auto& concentrations = impl_->GetOrderedConcentrations();
    size_t n_species = variable_map.size();

    for (const auto& [name, values] : input)
    {
      auto it = variable_map.find(name);
      if (it == variable_map.end())
        continue;
      size_t i_species = it->second;
      for (size_t i_cell = 0; i_cell < values.size(); ++i_cell)
      {
        size_t group_index = i_cell / vector_size_;
        size_t row_in_group = i_cell % vector_size_;
        size_t idx = (group_index * n_species + i_species) * vector_size_ + row_in_group;
        concentrations[idx] = values[i_cell];
      }
    }
  }

  std::map<std::string, std::vector<double>> State::GetConcentrations(musica::MICMSolver solver_type) const
  {
    std::size_t vector_size_ = musica::GetVectorSize(solver_type);
    std::map<std::string, std::vector<double>> output;
    auto variable_map = impl_->GetVariableMap();
    const auto& concentrations = impl_->GetOrderedConcentrations();
    size_t n_species = variable_map.size();
    size_t n_cells = impl_->NumberOfGridCells();

    for (const auto& [name, i_species] : variable_map)
    {
      output[name] = std::vector<double>(n_cells);
      for (size_t i_cell = 0; i_cell < n_cells; ++i_cell)
      {
        size_t group_index = i_cell / vector_size_;
        size_t row_in_group = i_cell % vector_size_;
        size_t idx = (group_index * n_species + i_species) * vector_size_ + row_in_group;
        output[name][i_cell] = concentrations[idx];
      }
    }
    return output;
  }

  void State::SetRateConstants(const std::map<std::string, std::vector<double>>& input, musica::MICMSolver solver_type)
  {
    std::size_t vector_size_ = musica::GetVectorSize(solver_type);
    auto rate_param_map = impl_->GetRateParameterMap();
    auto& rate_params = impl_->GetOrderedRateParameters();
    size_t n_params = rate_param_map.size();

    for (const auto& [name, values] : input)
    {
      auto it = rate_param_map.find(name);
      if (it == rate_param_map.end())
        continue;
      size_t i_param = it->second;
      for (size_t i_cell = 0; i_cell < values.size(); ++i_cell)
      {
        size_t group_index = i_cell / vector_size_;
        size_t row_in_group = i_cell % vector_size_;
        size_t idx = (group_index * n_params + i_param) * vector_size_ + row_in_group;
        rate_params[idx] = values[i_cell];
      }
    }
  }

  std::map<std::string, std::vector<double>> State::GetRateConstants(musica::MICMSolver solver_type) const
  {
    std::size_t vector_size_ = musica::GetVectorSize(solver_type);
    std::map<std::string, std::vector<double>> output;
    auto rate_param_map = impl_->GetRateParameterMap();
    const auto& rate_params = impl_->GetOrderedRateParameters();
    size_t n_params = rate_param_map.size();
    size_t n_cells = impl_->NumberOfGridCells();

    for (const auto& [name, i_param] : rate_param_map)
    {
      output[name] = std::vector<double>(n_cells);
      for (size_t i_cell = 0; i_cell < n_cells; ++i_cell)
      {
        size_t group_index = i_cell / vector_size_;
        size_t row_in_group = i_cell % vector_size_;
        size_t idx = (group_index * n_params + i_param) * vector_size_ + row_in_group;
        output[name][i_cell] = rate_params[idx];
      }
    }
    return output;
  }

  std::vector<double>& State::GetOrderedConcentrations()
  {
    return impl_->GetOrderedConcentrations();
  }

  void State::SetOrderedConcentrations(const std::vector<double>& concentrations)
  {
    auto& state_concentrations = impl_->GetOrderedConcentrations();
    for (size_t i = 0; i < concentrations.size(); ++i)
    {
      state_concentrations[i] = concentrations[i];
    }
  }

  std::vector<double>& State::GetOrderedRateParameters()
  {
    return impl_->GetOrderedRateParameters();
  }

  void State::SetOrderedRateConstants(const std::vector<double>& rateConstant)
  {
    auto& state_rate_params = impl_->GetOrderedRateParameters();
    for (size_t i = 0; i < rateConstant.size(); ++i)
    {
      state_rate_params[i] = rateConstant[i];
    }
  }

  std::pair<std::size_t, std::size_t> State::GetConcentrationsStrides()
  {
    return impl_->GetConcentrationsStrides();
  }

  std::pair<std::size_t, std::size_t> State::GetUserDefinedRateParametersStrides()
  {
    return impl_->GetRateParameterStrides();
  }

  std::unordered_map<std::string, std::size_t> State::GetVariableMap() const
  {
    return impl_->GetVariableMap();
  }

  std::unordered_map<std::string, std::size_t> State::GetRateParameterMap() const
  {
    return impl_->GetRateParameterMap();
  }

  IState* State::GetStateInterface()
  {
    return impl_.get();
  }

}  // namespace musica
