#include "state_wrapper.h"

#include "micm_wrapper.h"

#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

// Include MUSICA headers for real functionality
#include <musica/micm/micm.hpp>
#include <musica/micm/micm_c_interface.hpp>
#include <musica/micm/state.hpp>
#include <musica/micm/state_c_interface.hpp>
#include <musica/util.hpp>
#include <musica/version.hpp>

#include <micm/system/conditions.hpp>

namespace musica_addon
{

  // ============================================================================
  // StateWrapper Implementation
  // ============================================================================

  void StateDeleter::operator()(musica::State* state) const
  {
    if (state != nullptr)
    {
      musica::Error error;
      musica::DeleteState(state, &error);
      musica::DeleteError(&error);
    }
  }

  StateWrapper::StateWrapper(musica::State* state)
      : state_(state, StateDeleter())
  {
  }

  void StateWrapper::SetConcentrations(const std::map<std::string, std::vector<double>>& concentrations)
  {
    musica::Error error;

    // Get species ordering
    musica::Mappings species_ordering;
    musica::GetSpeciesOrdering(state_.get(), &species_ordering, &error);
    if (!musica::IsSuccess(error))
    {
      musica::DeleteError(&error);
      throw std::runtime_error("Failed to get species ordering");
    }

    // Get concentration pointer and strides
    size_t array_size;
    double* conc_ptr = musica::GetOrderedConcentrationsPointer(state_.get(), &array_size, &error);
    if (!musica::IsSuccess(error))
    {
      musica::DeleteMappings(&species_ordering);
      musica::DeleteError(&error);
      throw std::runtime_error("Failed to get concentrations pointer");
    }

    size_t cell_stride, species_stride;
    musica::GetConcentrationsStrides(state_.get(), &error, &cell_stride, &species_stride);

    size_t num_cells = musica::GetNumberOfGridCells(state_.get(), &error);

    // Set concentrations
    for (size_t i = 0; i < species_ordering.size_; ++i)
    {
      std::string species_name = species_ordering.mappings_[i].name_.value_;
      size_t species_idx = species_ordering.mappings_[i].index_;

      auto it = concentrations.find(species_name);
      if (it != concentrations.end())
      {
        const auto& values = it->second;
        for (size_t cell = 0; cell < num_cells && cell < values.size(); ++cell)
        {
          conc_ptr[species_idx * species_stride + cell * cell_stride] = values[cell];
        }
      }
    }

    musica::DeleteMappings(&species_ordering);
    musica::DeleteError(&error);
  }

  std::map<std::string, std::vector<double>> StateWrapper::GetConcentrations()
  {
    musica::Error error;
    std::map<std::string, std::vector<double>> result;

    // Get species ordering
    musica::Mappings species_ordering;
    musica::GetSpeciesOrdering(state_.get(), &species_ordering, &error);
    if (!musica::IsSuccess(error))
    {
      musica::DeleteError(&error);
      throw std::runtime_error("Failed to get species ordering");
    }

    // Get concentration pointer and strides
    size_t array_size;
    double* conc_ptr = musica::GetOrderedConcentrationsPointer(state_.get(), &array_size, &error);
    if (!musica::IsSuccess(error))
    {
      musica::DeleteMappings(&species_ordering);
      musica::DeleteError(&error);
      throw std::runtime_error("Failed to get concentrations pointer");
    }

    size_t cell_stride, species_stride;
    musica::GetConcentrationsStrides(state_.get(), &error, &cell_stride, &species_stride);

    size_t num_cells = musica::GetNumberOfGridCells(state_.get(), &error);

    // Get concentrations
    for (size_t i = 0; i < species_ordering.size_; ++i)
    {
      std::string species_name = species_ordering.mappings_[i].name_.value_;
      size_t species_idx = species_ordering.mappings_[i].index_;

      std::vector<double> values(num_cells);
      for (size_t cell = 0; cell < num_cells; ++cell)
      {
        values[cell] = conc_ptr[species_idx * species_stride + cell * cell_stride];
      }
      result[species_name] = values;
    }

    musica::DeleteMappings(&species_ordering);
    musica::DeleteError(&error);
    return result;
  }

  void StateWrapper::SetUserDefinedRateParameters(const std::map<std::string, std::vector<double>>& params)
  {
    musica::Error error;

    // Get rate parameters ordering
    musica::Mappings params_ordering;
    musica::GetUserDefinedRateParametersOrdering(state_.get(), &params_ordering, &error);
    if (!musica::IsSuccess(error))
    {
      musica::DeleteError(&error);
      throw std::runtime_error("Failed to get user-defined rate parameters ordering");
    }

    // Get rate parameters pointer and strides
    size_t array_size;
    double* params_ptr = musica::GetOrderedRateParametersPointer(state_.get(), &array_size, &error);
    if (!musica::IsSuccess(error))
    {
      musica::DeleteMappings(&params_ordering);
      musica::DeleteError(&error);
      throw std::runtime_error("Failed to get rate parameters pointer");
    }

    size_t cell_stride, param_stride;
    musica::GetUserDefinedRateParametersStrides(state_.get(), &error, &cell_stride, &param_stride);

    size_t num_cells = musica::GetNumberOfGridCells(state_.get(), &error);

    // Set parameters
    for (size_t i = 0; i < params_ordering.size_; ++i)
    {
      std::string param_name = params_ordering.mappings_[i].name_.value_;
      size_t param_idx = params_ordering.mappings_[i].index_;

      auto it = params.find(param_name);
      if (it != params.end())
      {
        const auto& values = it->second;
        for (size_t cell = 0; cell < num_cells && cell < values.size(); ++cell)
        {
          params_ptr[param_idx * param_stride + cell * cell_stride] = values[cell];
        }
      }
    }

    musica::DeleteMappings(&params_ordering);
    musica::DeleteError(&error);
  }

  std::map<std::string, std::vector<double>> StateWrapper::GetUserDefinedRateParameters()
  {
    musica::Error error;
    std::map<std::string, std::vector<double>> result;

    // Get rate parameters ordering
    musica::Mappings params_ordering;
    musica::GetUserDefinedRateParametersOrdering(state_.get(), &params_ordering, &error);
    if (!musica::IsSuccess(error))
    {
      musica::DeleteError(&error);
      throw std::runtime_error("Failed to get user-defined rate parameters ordering");
    }

    // Get rate parameters pointer and strides
    size_t array_size;
    double* params_ptr = musica::GetOrderedRateParametersPointer(state_.get(), &array_size, &error);
    if (!musica::IsSuccess(error))
    {
      musica::DeleteMappings(&params_ordering);
      musica::DeleteError(&error);
      throw std::runtime_error("Failed to get rate parameters pointer");
    }

    size_t cell_stride, param_stride;
    musica::GetUserDefinedRateParametersStrides(state_.get(), &error, &cell_stride, &param_stride);

    size_t num_cells = musica::GetNumberOfGridCells(state_.get(), &error);

    // Get parameters
    for (size_t i = 0; i < params_ordering.size_; ++i)
    {
      std::string param_name = params_ordering.mappings_[i].name_.value_;
      size_t param_idx = params_ordering.mappings_[i].index_;

      std::vector<double> values(num_cells);
      for (size_t cell = 0; cell < num_cells; ++cell)
      {
        values[cell] = params_ptr[param_idx * param_stride + cell * cell_stride];
      }
      result[param_name] = values;
    }

    musica::DeleteMappings(&params_ordering);
    musica::DeleteError(&error);
    return result;
  }

  void StateWrapper::SetConditions(
      const std::vector<double>* temperatures,
      const std::vector<double>* pressures,
      const std::vector<double>* air_densities)
  {
    musica::Error error;
    size_t array_size;
    micm::Conditions* conditions = musica::GetConditionsPointer(state_.get(), &array_size, &error);

    if (!musica::IsSuccess(error))
    {
      musica::DeleteError(&error);
      throw std::runtime_error("Failed to get conditions pointer");
    }

    size_t num_cells = musica::GetNumberOfGridCells(state_.get(), &error);

    for (size_t i = 0; i < num_cells; ++i)
    {
      if (temperatures && i < temperatures->size())
      {
        conditions[i].temperature_ = (*temperatures)[i];
      }
      if (pressures && i < pressures->size())
      {
        conditions[i].pressure_ = (*pressures)[i];
      }
      if (air_densities && i < air_densities->size())
      {
        conditions[i].air_density_ = (*air_densities)[i];
      }
      else if (temperatures && pressures && i < temperatures->size() && i < pressures->size())
      {
        // Calculate air density from ideal gas law if not provided
        constexpr double GAS_CONSTANT = 8.31446261815324;  // J K^-1 mol^-1
        conditions[i].air_density_ = (*pressures)[i] / (GAS_CONSTANT * (*temperatures)[i]);
      }
    }

    musica::DeleteError(&error);
  }

  std::map<std::string, std::vector<double>> StateWrapper::GetConditions()
  {
    musica::Error error;
    std::map<std::string, std::vector<double>> result;

    size_t array_size;
    micm::Conditions* conditions = musica::GetConditionsPointer(state_.get(), &array_size, &error);

    if (!musica::IsSuccess(error))
    {
      musica::DeleteError(&error);
      throw std::runtime_error("Failed to get conditions pointer");
    }

    size_t num_cells = musica::GetNumberOfGridCells(state_.get(), &error);

    result["temperature"] = std::vector<double>(num_cells);
    result["pressure"] = std::vector<double>(num_cells);
    result["air_density"] = std::vector<double>(num_cells);

    for (size_t i = 0; i < num_cells; ++i)
    {
      result["temperature"][i] = conditions[i].temperature_;
      result["pressure"][i] = conditions[i].pressure_;
      result["air_density"][i] = conditions[i].air_density_;
    }

    musica::DeleteError(&error);
    return result;
  }

  std::map<std::string, int> StateWrapper::GetSpeciesOrdering()
  {
    musica::Error error;
    std::map<std::string, int> result;

    musica::Mappings species_ordering;
    musica::GetSpeciesOrdering(state_.get(), &species_ordering, &error);

    if (!musica::IsSuccess(error))
    {
      musica::DeleteError(&error);
      throw std::runtime_error("Failed to get species ordering");
    }

    for (size_t i = 0; i < species_ordering.size_; ++i)
    {
      result[species_ordering.mappings_[i].name_.value_] = species_ordering.mappings_[i].index_;
    }

    musica::DeleteMappings(&species_ordering);
    musica::DeleteError(&error);
    return result;
  }

  std::map<std::string, int> StateWrapper::GetUserDefinedRateParametersOrdering()
  {
    musica::Error error;
    std::map<std::string, int> result;

    musica::Mappings params_ordering;
    musica::GetUserDefinedRateParametersOrdering(state_.get(), &params_ordering, &error);

    if (!musica::IsSuccess(error))
    {
      musica::DeleteError(&error);
      throw std::runtime_error("Failed to get user-defined rate parameters ordering");
    }

    for (size_t i = 0; i < params_ordering.size_; ++i)
    {
      result[params_ordering.mappings_[i].name_.value_] = params_ordering.mappings_[i].index_;
    }

    musica::DeleteMappings(&params_ordering);
    musica::DeleteError(&error);
    return result;
  }

  void StateWrapper::GetConcentrationStrides(size_t& cell_stride, size_t& species_stride)
  {
    musica::Error error;
    musica::GetConcentrationsStrides(state_.get(), &error, &cell_stride, &species_stride);
    musica::DeleteError(&error);
  }

  void StateWrapper::GetUserDefinedRateParameterStrides(size_t& cell_stride, size_t& param_stride)
  {
    musica::Error error;
    musica::GetUserDefinedRateParametersStrides(state_.get(), &error, &cell_stride, &param_stride);
    musica::DeleteError(&error);
  }

  size_t StateWrapper::GetNumberOfGridCells()
  {
    musica::Error error;
    size_t num_cells = musica::GetNumberOfGridCells(state_.get(), &error);
    musica::DeleteError(&error);
    return num_cells;
  }

  double* StateWrapper::GetConcentrationsPointer(size_t& array_size)
  {
    musica::Error error;
    double* ptr = musica::GetOrderedConcentrationsPointer(state_.get(), &array_size, &error);
    musica::DeleteError(&error);
    return ptr;
  }

}  // namespace musica_addon