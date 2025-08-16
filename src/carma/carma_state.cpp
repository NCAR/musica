
// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the CARMA state class
#include <musica/carma/carma_c_interface.hpp>
#include <musica/carma/carma_state.hpp>
#include <musica/carma/error.hpp>

#include <cstring>
#include <iostream>
#include <stdexcept>

namespace musica
{

  CARMAState::CARMAState(const CARMA& carma, const CARMAStateParameters& params)
  {
    CCARMAParameters* carma_params = carma.GetCParameters();
    CARMAStateParametersC state_params;
    this->nz = carma_params->nz;
    int n_wavelength_bins = carma_params->wavelength_bin_size;
    state_params.time = params.time;
    state_params.time_step = params.time_step;
    state_params.longitude = params.longitude;
    state_params.latitude = params.latitude;
    state_params.coordinates = static_cast<int>(params.coordinates);
    state_params.vertical_center_size = static_cast<int>(params.vertical_center.size());
    state_params.vertical_center = params.vertical_center.empty() ? nullptr : params.vertical_center.data();
    if (state_params.vertical_center != nullptr && state_params.vertical_center_size != this->nz)
      throw std::invalid_argument("Vertical center heights size must match the number of vertical centers.");
    state_params.vertical_levels_size = static_cast<int>(params.vertical_levels.size());
    state_params.vertical_levels = params.vertical_levels.empty() ? nullptr : params.vertical_levels.data();
    if (state_params.vertical_levels != nullptr && state_params.vertical_levels_size != this->nz + 1)
      throw std::invalid_argument("Vertical levels size must match the number of vertical levels.");
    state_params.temperature_size = static_cast<int>(params.temperature.size());
    state_params.temperature = params.temperature.empty() ? nullptr : params.temperature.data();
    if (state_params.temperature != nullptr && state_params.temperature_size != this->nz)
      throw std::invalid_argument("Temperature profile size must match the number of vertical centers.");
    state_params.pressure_size = static_cast<int>(params.pressure.size());
    state_params.pressure = params.pressure.empty() ? nullptr : params.pressure.data();
    if (state_params.pressure != nullptr && state_params.pressure_size != this->nz)
      throw std::invalid_argument("Pressure profile size must match the number of vertical centers.");
    state_params.pressure_levels_size = static_cast<int>(params.pressure_levels.size());
    state_params.pressure_levels = params.pressure_levels.empty() ? nullptr : params.pressure_levels.data();
    if (state_params.pressure_levels != nullptr && state_params.pressure_levels_size != this->nz + 1)
      throw std::invalid_argument("Pressure levels size must match the number of vertical levels.");
    state_params.specific_humidity_size = static_cast<int>(params.specific_humidity.size());
    state_params.specific_humidity = params.specific_humidity.empty() ? nullptr : params.specific_humidity.data();
    if (state_params.specific_humidity != nullptr && state_params.specific_humidity_size != this->nz)
      throw std::invalid_argument("Specific humidity profile size must match the number of vertical centers.");
    state_params.relative_humidity_size = static_cast<int>(params.relative_humidity.size());
    state_params.relative_humidity = params.relative_humidity.empty() ? nullptr : params.relative_humidity.data();
    if (state_params.relative_humidity != nullptr && state_params.relative_humidity_size != this->nz)
      throw std::invalid_argument("Relative humidity profile size must match the number of vertical centers.");
    state_params.original_temperature_size = static_cast<int>(params.original_temperature.size());
    state_params.original_temperature = params.original_temperature.empty() ? nullptr : params.original_temperature.data();
    if (state_params.original_temperature != nullptr && state_params.original_temperature_size != this->nz)
      throw std::invalid_argument("Original temperature profile size must match the number of vertical centers.");
    state_params.radiative_intensity_dim_1_size = params.radiative_intensity_dim_1_size;
    state_params.radiative_intensity_dim_2_size = params.radiative_intensity_dim_2_size;
    state_params.radiative_intensity = params.radiative_intensity.empty() ? nullptr : params.radiative_intensity.data();
    if (state_params.radiative_intensity != nullptr)
    {
      if (state_params.radiative_intensity_dim_1_size != n_wavelength_bins)
        throw std::invalid_argument("Radiative intensity first dimension size must match the number of wavelength bins.");
      if (state_params.radiative_intensity_dim_2_size != this->nz)
        throw std::invalid_argument("Radiative intensity second dimension size must match the number of vertical centers.");
    }

    int rc;
    f_carma_state_ = InternalCreateCarmaState(
        carma.GetCarmaInstance(),
        *carma_params,
        state_params,
        &rc);  // No return code needed in this context
    if (rc != 0)
    {
      throw std::runtime_error(CarmaErrorCodeToMessage(rc));
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
        std::cerr << CarmaErrorCodeToMessage(rc) << std::endl;
      }
    }
  }

  void CARMAState::SetBin(int bin_index, int element_index, const std::vector<double>& values, const double surface_mass)
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
    InternalSetBin(
        f_carma_state_, bin_index, element_index, values.data(), static_cast<int>(values.size()), surface_mass, &rc);
    if (rc != 0)
    {
      throw std::runtime_error(CarmaErrorCodeToMessage(rc));
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
      throw std::runtime_error(CarmaErrorCodeToMessage(rc));
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
      throw std::runtime_error(CarmaErrorCodeToMessage(rc));
    }
  }

  void CARMAState::SetTemperature(const std::vector<double>& temperature)
  {
    if (f_carma_state_ == nullptr)
    {
      throw std::runtime_error("CARMA state instance is not initialized.");
    }

    if (temperature.empty())
    {
      throw std::invalid_argument("Temperature vector cannot be empty.");
    }

    int rc;
    InternalSetTemperature(f_carma_state_, temperature.data(), static_cast<int>(temperature.size()), &rc);
    if (rc != 0)
    {
      throw std::runtime_error(CarmaErrorCodeToMessage(rc));
    }
  }

  void CARMAState::SetAirDensity(const std::vector<double>& air_density)
  {
    if (f_carma_state_ == nullptr)
    {
      throw std::runtime_error("CARMA state instance is not initialized.");
    }

    if (air_density.empty())
    {
      throw std::invalid_argument("Air density vector cannot be empty.");
    }

    int rc;
    InternalSetAirDensity(f_carma_state_, air_density.data(), static_cast<int>(air_density.size()), &rc);
    if (rc != 0)
    {
      throw std::runtime_error(CarmaErrorCodeToMessage(rc));
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
        nz,
        &rc);
    if (rc != 0)
    {
      throw std::runtime_error(CarmaErrorCodeToMessage(rc));
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
      throw std::runtime_error(CarmaErrorCodeToMessage(rc));
    }

    return bin_values;
  }

  CarmaDetrainValues CARMAState::GetDetrain(int bin_index, int element_index) const
  {
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
      throw std::runtime_error(CarmaErrorCodeToMessage(rc));
    }

    return detrain_values;
  }

  CarmaGasValues CARMAState::GetGas(int gas_index) const
  {
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
      throw std::runtime_error(CarmaErrorCodeToMessage(rc));
    }

    return gas_values;
  }

  CarmaEnvironmentalValues CARMAState::GetEnvironmentalValues() const
  {
    if (f_carma_state_ == nullptr)
    {
      throw std::runtime_error("CARMA state instance is not initialized.");
    }

    CarmaEnvironmentalValues values;
    values.temperature.resize(nz);
    values.pressure.resize(nz);
    values.air_density.resize(nz);
    values.latent_heat.resize(nz);
    int rc;

    InternalGetEnvironmentalValues(
        f_carma_state_,
        nz,
        values.temperature.data(),
        values.pressure.data(),
        values.air_density.data(),
        values.latent_heat.data(),
        &rc);

    if (rc != 0)
    {
      throw std::runtime_error(CarmaErrorCodeToMessage(rc));
    }

    return values;
  }

  void CARMAState::Step(CARMAStateStepConfig& step_config)
  {
    if (f_carma_state_ == nullptr)
    {
      throw std::runtime_error("CARMA state instance is not initialized.");
    }

    CARMAStateStepConfigC step_config_c;
    step_config_c.cloud_fraction = step_config.cloud_fraction.empty() ? nullptr : step_config.cloud_fraction.data();
    step_config_c.cloud_fraction_size = static_cast<int>(step_config.cloud_fraction.size());
    step_config_c.critical_relative_humidity =
        step_config.critical_relative_humidity.empty() ? nullptr : step_config.critical_relative_humidity.data();
    step_config_c.critical_relative_humidity_size = static_cast<int>(step_config.critical_relative_humidity.size());
    step_config_c.land.surface_friction_velocity = step_config.land.surface_friction_velocity;
    step_config_c.land.aerodynamic_resistance = step_config.land.aerodynamic_resistance;
    step_config_c.land.area_fraction = step_config.land.area_fraction;
    step_config_c.ocean.surface_friction_velocity = step_config.ocean.surface_friction_velocity;
    step_config_c.ocean.aerodynamic_resistance = step_config.ocean.aerodynamic_resistance;
    step_config_c.ocean.area_fraction = step_config.ocean.area_fraction;
    step_config_c.ice.surface_friction_velocity = step_config.ice.surface_friction_velocity;
    step_config_c.ice.aerodynamic_resistance = step_config.ice.aerodynamic_resistance;
    step_config_c.ice.area_fraction = step_config.ice.area_fraction;

    int rc;
    InternalStepCarmaState(f_carma_state_, step_config_c, &rc);
    if (rc != 0)
    {
      throw std::runtime_error(CarmaErrorCodeToMessage(rc));
    }
  }
}  // namespace musica