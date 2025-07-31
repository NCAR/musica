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
    double time;
    double longitude;
    double latitude;
    CarmaCoordinates coordinates;
    std::vector<double> vertical_center;
    std::vector<double> vertical_levels;
    std::vector<double> temperature;
    std::vector<double> pressure;
    std::vector<double> pressure_levels;
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
    std::vector<double> number_mixing_ratio;         // Number mixing ratio for the bin [# cm-3]
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

  class CARMAState
  {
   public:
    explicit CARMAState(CARMA* carma, const CARMAStateParameters& params);

    ~CARMAState();

    void SetBin(int bin_index, int element_index, const std::vector<double>& values);
    void SetDetrain(int bin_index, int element_index, const std::vector<double>& values);
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

   private:
    void* f_carma_state_;
    int nz;  // Number of vertical levels
  };

}  // namespace musica