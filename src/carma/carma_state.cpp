
// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the CARMA state class
#include <musica/carma/carma_c_interface.hpp>
#include <musica/carma/carma_state.hpp>

#include <cstring>
#include <iostream>
#include <stdexcept>

namespace musica
{

  CARMAState::CARMAState(CARMA* carma, const CARMAStateParameters& params)
  {
    CCARMAParameters* carma_params = carma->GetParameters();
    this->nz = carma_params->nz;
    CARMAStateParametersC state_params;
    state_params.time = params.time;
    state_params.longitude = params.longitude;
    state_params.latitude = params.latitude;
    state_params.coordinates = static_cast<int>(params.coordinates);
    state_params.vertical_center_size = static_cast<int>(params.vertical_center.size());
    state_params.vertical_center = params.vertical_center.empty() ? nullptr : params.vertical_center.data();
    state_params.vertical_levels_size = static_cast<int>(params.vertical_levels.size());
    state_params.vertical_levels = params.vertical_levels.empty() ? nullptr : params.vertical_levels.data();
    state_params.temperature_size = static_cast<int>(params.temperature.size());
    state_params.temperature = params.temperature.empty() ? nullptr : params.temperature.data();
    state_params.pressure_size = static_cast<int>(params.pressure.size());
    state_params.pressure = params.pressure.empty() ? nullptr : params.pressure.data();
    state_params.pressure_levels_size = static_cast<int>(params.pressure_levels.size());
    state_params.pressure_levels = params.pressure_levels.empty() ? nullptr : params.pressure_levels.data();

    int rc;
    f_carma_state_ = InternalCreateCarmaState(
        carma->GetCarmaInstance(),
        *carma_params,
        state_params,
        &rc);  // No return code needed in this context
    if (f_carma_state_ == nullptr || rc != 0)
    {
      throw std::runtime_error("Failed to create CARMA state with return code: " + std::to_string(rc));
    }
  }

  CARMAState::~CARMAState()
  {
    if (f_carma_state_ != nullptr)
    {
      int rc;
      InternalDestroyCarmaState(f_carma_state_, &rc);
      f_carma_state_ = nullptr;
      if (rc != 0)
      {
        std::cerr << "Failed to destroy CARMA state with return code: " << rc << std::endl;
      }
    }
  }

  void CARMAState::SetBin(int bin_index, int element_index, const std::vector<double>& values)
  {
    if (f_carma_state_ == nullptr)
    {
      throw std::runtime_error("CARMA state instance is not initialized.");
    }

    if (values.empty())
    {
      throw std::invalid_argument("Values vector cannot be empty.");
    }

    int rc;
    InternalSetBin(f_carma_state_, bin_index, element_index, values.data(), static_cast<int>(values.size()), &rc);
    if (rc != 0)
    {
      throw std::runtime_error("Failed to set bin values with return code: " + std::to_string(rc));
    }
  }

  void CARMAState::SetDetrain(int bin_index, int element_index, const std::vector<double>& values)
  {
    if (f_carma_state_ == nullptr)
    {
      throw std::runtime_error("CARMA state instance is not initialized.");
    }

    if (values.empty())
    {
      throw std::invalid_argument("Values vector cannot be empty.");
    }

    int rc;
    InternalSetDetrain(f_carma_state_, bin_index, element_index, values.data(), static_cast<int>(values.size()), &rc);
    if (rc != 0)
    {
      throw std::runtime_error("Failed to set detrain values with return code: " + std::to_string(rc));
    }
  }

  void CARMAState::SetGas(
      int gas_index,
      const std::vector<double>& values,
      const std::vector<double>& old_mmr,
      const std::vector<double>& gas_saturation_wrt_ice,
      const std::vector<double>& gas_saturation_wrt_liquid)
  {
    if (f_carma_state_ == nullptr)
    {
      throw std::runtime_error("CARMA state instance is not initialized.");
    }

    if (values.empty())
    {
      throw std::invalid_argument("Values vector cannot be empty.");
    }

    int rc;
    InternalSetGas(
        f_carma_state_,
        gas_index,
        values.data(),
        static_cast<int>(values.size()),
        old_mmr.data(),
        static_cast<int>(old_mmr.size()),
        gas_saturation_wrt_ice.data(),
        static_cast<int>(gas_saturation_wrt_ice.size()),
        gas_saturation_wrt_liquid.data(),
        static_cast<int>(gas_saturation_wrt_liquid.size()),
        &rc);
    if (rc != 0)
    {
      throw std::runtime_error("Failed to set gas values with return code: " + std::to_string(rc));
    }
  }

  CarmaStatistics CARMAState::GetStepStatistics() const
  {
    if (f_carma_state_ == nullptr)
    {
      throw std::runtime_error("CARMA state instance is not initialized.");
    }

    CarmaStatistics stats;
    stats.z_substeps.resize(nz);
    int rc;
    InternalGetStepStatistics(
        f_carma_state_,
        &stats.max_number_of_substeps,
        &stats.max_number_of_retries,
        &stats.total_number_of_steps,
        &stats.total_number_of_substeps,
        &stats.total_number_of_retries,
        &stats.xc,
        &stats.yc,
        stats.z_substeps.data(),
        static_cast<int>(stats.z_substeps.size()),
        &rc);
    if (rc != 0)
    {
      throw std::runtime_error("Failed to get CARMA step statistics with return code: " + std::to_string(rc));
    }
    return stats;
  }

  CarmaBinValues CARMAState::GetBinValues(int bin_index, int element_index) const
  {
    if (f_carma_state_ == nullptr)
    {
      throw std::runtime_error("CARMA state instance is not initialized.");
    }

    CarmaBinValues bin_values;
    bin_values.mass_mixing_ratio.resize(nz);
    bin_values.number_mixing_ratio.resize(nz);
    bin_values.number_density.resize(nz);
    bin_values.nucleation_rate.resize(nz);
    bin_values.wet_particle_radius.resize(nz);
    bin_values.wet_particle_density.resize(nz);
    bin_values.dry_particle_density.resize(nz);
    bin_values.fall_velocity.resize(nz + 1);
    bin_values.delta_particle_temperature.resize(nz);
    bin_values.kappa.resize(nz);
    bin_values.total_mass_mixing_ratio.resize(nz);
    int rc;

    InternalGetBin(
        f_carma_state_,
        bin_index,
        element_index,
        nz,
        bin_values.mass_mixing_ratio.data(),
        bin_values.number_mixing_ratio.data(),
        bin_values.number_density.data(),
        bin_values.nucleation_rate.data(),
        bin_values.wet_particle_radius.data(),
        bin_values.wet_particle_density.data(),
        bin_values.dry_particle_density.data(),
        &bin_values.particle_mass_on_surface,
        &bin_values.sedimentation_flux,
        bin_values.fall_velocity.data(),
        &bin_values.deposition_velocity,
        bin_values.delta_particle_temperature.data(),
        bin_values.kappa.data(),
        bin_values.total_mass_mixing_ratio.data(),
        &rc);

    if (rc != 0)
    {
      throw std::runtime_error("Failed to get CARMA bin values with return code: " + std::to_string(rc));
    }

    return bin_values;
  }

  CarmaDetrainValues CARMAState::GetDetrain(int bin_index, int element_index) const {
    if (f_carma_state_ == nullptr)
    {
      throw std::runtime_error("CARMA state instance is not initialized.");
    }

    CarmaDetrainValues detrain_values;
    detrain_values.mass_mixing_ratio.resize(nz);
    detrain_values.number_mixing_ratio.resize(nz);
    detrain_values.number_density.resize(nz);
    detrain_values.wet_particle_radius.resize(nz);
    detrain_values.wet_particle_density.resize(nz);
    int rc;

    InternalGetDetrain(
        f_carma_state_,
        bin_index,
        element_index,
        nz,
        detrain_values.mass_mixing_ratio.data(),
        detrain_values.number_mixing_ratio.data(),
        detrain_values.number_density.data(),
        detrain_values.wet_particle_radius.data(),
        detrain_values.wet_particle_density.data(),
        &rc);

    if (rc != 0)
    {
      throw std::runtime_error("Failed to get CARMA detrain values with return code: " + std::to_string(rc));
    }

    return detrain_values;
  }

  CarmaGasValues CARMAState::GetGas(int gas_index) const {
    if (f_carma_state_ == nullptr)
    {
      throw std::runtime_error("CARMA state instance is not initialized.");
    }
    CarmaGasValues gas_values;
    gas_values.mass_mixing_ratio.resize(nz);
    gas_values.gas_saturation_wrt_ice.resize(nz);
    gas_values.gas_saturation_wrt_liquid.resize(nz);
    gas_values.gas_vapor_pressure_wrt_ice.resize(nz);
    gas_values.gas_vapor_pressure_wrt_liquid.resize(nz);
    gas_values.weight_pct_aerosol_composition.resize(nz);
    int rc; 

    InternalGetGas(
        f_carma_state_,
        gas_index,
        nz,
        gas_values.mass_mixing_ratio.data(),
        gas_values.gas_saturation_wrt_ice.data(),
        gas_values.gas_saturation_wrt_liquid.data(),
        gas_values.gas_vapor_pressure_wrt_ice.data(),
        gas_values.gas_vapor_pressure_wrt_liquid.data(),
        gas_values.weight_pct_aerosol_composition.data(),
        &rc);

    if (rc != 0)
    {
      throw std::runtime_error("Failed to get CARMA gas values with return code: " + std::to_string(rc));
    }

    return gas_values;
  }
}  // namespace musica