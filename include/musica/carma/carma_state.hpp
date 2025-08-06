// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the definition of the CARMA state class. This wraps the fortran
// functions in CARMA
#pragma once

#include <vector>

namespace musica
{
  class CARMA;

  enum class CarmaCoordinates
  {
    CARTESIAN = 1,
    SIGMA = 2,
    LONGITUDE_LATITUDE = 3,
    LAMBERT_CONFORMAL = 4,
    POLAR_STEREOGRAPHIC = 5,
    MERCATOR = 6,
    HYBRID = 7
  };

  struct CARMAStateParameters
  {
    double time = 0.0;       // Current time [s]
    double time_step = 0.0;  // Time step [s]
    double longitude = 0.0;  // Longitude [degrees]
    double latitude = 0.0;   // Latitude [degrees]
    CarmaCoordinates coordinates = CarmaCoordinates::CARTESIAN;
    std::vector<double> vertical_center;       // Vertical center heights [m]
    std::vector<double> vertical_levels;       // Vertical levels [m]
    std::vector<double> temperature;           // Temperature at vertical centers [K]
    std::vector<double> pressure;              // Pressure at vertical centers [Pa]
    std::vector<double> pressure_levels;       // Pressure at vertical levels [Pa]
    std::vector<double> specific_humidity;     // Specific humidity at vertical centers [kg/kg]
    std::vector<double> relative_humidity;     // Relative humidity at vertical centers [fraction]
    std::vector<double> original_temperature;  // Original temperature at vertical centers [K]

    // 2D array for radiative intensity
    // This is flattened into a 1D vector so we can pass it to Fortran
    // The first dimension is the wavelength bins, the second is the vertical centers
    std::vector<double> radiative_intensity;  // Radiative intensity at wavelength bins and vertical centers [W/m²/sr/m]
    int radiative_intensity_dim_1_size = 0;   // Number of wavelength bins
    int radiative_intensity_dim_2_size = 0;   // Number of vertical centers
  };

  struct CARMASurfaceProperties
  {
    double surface_friction_velocity = 0.0;  // Surface friction velocity [m/s]
    double aerodynamic_resistance = 0.0;     // Aerodynamic resistance [s/m]
    double area_fraction = 0.0;              // Area fraction [fraction]
  };

  struct CARMAStateStepConfig
  {
    std::vector<double> cloud_fraction;  // Cloud fraction at vertical centers [fraction]
    std::vector<double>
        critical_relative_humidity;  // Relative humidity for onset of liquid clouds at vertical centers [fraction]
    CARMASurfaceProperties land;     // Surface properties for land
    CARMASurfaceProperties ocean;    // Surface properties for ocean
    CARMASurfaceProperties ice;      // Surface properties for ice
  };

  struct CarmaStatistics
  {
    int max_number_of_substeps;      // Maximum number of substeps taken in the last run
    double max_number_of_retries;    // Maximum number of retries for convergence
    double total_number_of_steps;    // Total number of steps taken in the last run
    int total_number_of_substeps;    // Total number of substeps taken in the last run
    double total_number_of_retries;  // Total number of retries for convergence
    std::vector<double> z_substeps;  // number of substeps per vertical level
    double xc;                       // x location at the center of this CARMA state
    double yc;                       // y location at the center of this CARMA state
  };

  struct CarmaBinValues
  {
    std::vector<double> mass_mixing_ratio;           // Values for the bin [kg kg-1]
    std::vector<double> number_mixing_ratio;         // Number mixing ratio for the bin [# kg-1]
    std::vector<double> number_density;              // Number density for the bin [# cm-3]
    std::vector<double> nucleation_rate;             // Nucleation rate for the bin [# cm-3 s-1]
    std::vector<double> wet_particle_radius;         // Wet particle radius for the bin [cm]
    std::vector<double> wet_particle_density;        // Wet particle density for the bin [g cm-3]
    std::vector<double> dry_particle_density;        // Dry particle density for the bin [g cm-3]
    double particle_mass_on_surface;                 // Mass of the particle on the surface [kg m-2]
    double sedimentation_flux;                       // Sedimentation flex for the bin [kg m-2 s-1]
    std::vector<double> fall_velocity;               // Fall velocity for the bin [cm s-1]
    double deposition_velocity;                      // Deposition velocity for the bin [cm s-1]
    std::vector<double> delta_particle_temperature;  // [K]
    std::vector<double> kappa;                       // hygroscopicity parameter for the bin
    std::vector<double> total_mass_mixing_ratio;     // kg m-3
  };

  struct CarmaDetrainValues
  {
    std::vector<double> mass_mixing_ratio;     // Mass mixing ratio for detrainment [kg kg-1]
    std::vector<double> number_mixing_ratio;   // Number mixing ratio for detrainment [# cm-3]
    std::vector<double> number_density;        // Number density for detrainment [# cm-3]
    std::vector<double> wet_particle_radius;   // Wet particle radius for detrainment [cm]
    std::vector<double> wet_particle_density;  // Wet particle density for detrainment [g cm-3]
  };

  struct CarmaGasValues
  {
    std::vector<double> mass_mixing_ratio;               // Mass mixing ratio for the gas [kg kg-1]
    std::vector<double> gas_saturation_wrt_ice;          // Gas saturation with respect to ice [kg kg-1]
    std::vector<double> gas_saturation_wrt_liquid;       // Gas saturation with respect to liquid [kg kg-1]
    std::vector<double> gas_vapor_pressure_wrt_ice;      // Vapor pressure with respect to ice [Pa]
    std::vector<double> gas_vapor_pressure_wrt_liquid;   // Vapor pressure with respect to liquid [Pa]
    std::vector<double> weight_pct_aerosol_composition;  // Weight percentage of aerosol composition [weight %]
  };

  struct CarmaEnvironmentalValues
  {
    std::vector<double> temperature;  // Temperature [K]
    std::vector<double> pressure;     // Pressure [Pa]
    std::vector<double> air_density;  // Air density [kg m-3]
    std::vector<double> latent_heat;  // Latent heat [K s-1]
  };

  class CARMAState
  {
   public:
    explicit CARMAState(const CARMA& carma, const CARMAStateParameters& params);

    ~CARMAState();

    /// @brief Set the values for a specific bin and element
    /// @param bin_index The index of the bin
    /// @param element_index The index of the particle element
    /// @param values Bin mixing ratio at vertical centers [kg/kg]
    /// @param surface_mass Element mass on the surface [kg m-2] (0: off)
    void SetBin(int bin_index, int element_index, const std::vector<double>& values, const double surface_mass);

    /// @brief Set the mass of the detrained condensate for the bin for each particle element
    /// @param bin_index The index of the bin
    /// @param element_index The index of the particle element
    /// @param values Bin mixing ratio at vertical centers [kg/kg]
    void SetDetrain(int bin_index, int element_index, const std::vector<double>& values);

    /// @brief Set the gas profile
    /// @param gas_index The index of the gas
    /// @param values Mass mixing ratios at vertical centers [kg/kg]
    /// @param old_mmr Original mass mixing ratios at vertical centers [kg/kg]
    /// @param gas_saturation_wrt_ice The gas saturation with respect to ice [fraction]
    /// @param gas_saturation_wrt_liquid The gas saturation with respect to liquid [fraction]
    void SetGas(
        int gas_index,
        const std::vector<double>& values,
        const std::vector<double>& old_mmr,
        const std::vector<double>& gas_saturation_wrt_ice,
        const std::vector<double>& gas_saturation_wrt_liquid);
    CarmaStatistics GetStepStatistics() const;
    CarmaBinValues GetBinValues(int bin_index, int element_index) const;
    CarmaDetrainValues GetDetrain(int bin_index, int element_index) const;
    CarmaGasValues GetGas(int gas_index) const;
    CarmaEnvironmentalValues GetEnvironmentalValues() const;

    /// @brief Set the temperature profile
    /// @param temperature The temperature profile [K] (number of vertical centers)
    void SetTemperature(const std::vector<double>& temperature);

    /// @brief Set the air density profile
    /// @param air_density The air density profile [kg/m3] (number of vertical centers)
    void SetAirDensity(const std::vector<double>& air_density);
    void Step(CARMAStateStepConfig& step_config);

   private:
    void* f_carma_state_;
    int nz;  // Number of vertical levels
  };

}  // namespace musica