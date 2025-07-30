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
    double time = 0.0;  // Time [s]
    double time_step = 0.0;  // Time step [s]
    double longitude = 0.0;  // Longitude [degrees]
    double latitude = 0.0;  // Latitude [degrees]
    CarmaCoordinates coordinates = CarmaCoordinates::CARTESIAN;
    std::vector<double> vertical_center; // Vertical center heights [m]
    std::vector<double> vertical_levels; // Vertical levels [m]
    std::vector<double> temperature; // Temperature at vertical centers [K]
    std::vector<double> pressure; // Pressure at vertical centers [Pa]
    std::vector<double> pressure_levels; // Pressure at vertical levels [Pa]
    std::vector<double> specific_humidity; // Specific humidity at vertical centers [kg/kg]
    std::vector<double> relative_humidity; // Relative humidity at vertical centers [fraction]
    std::vector<double> original_temperature; // Original temperature at vertical centers [K]

    // 2D array for radiative intensity
    // This is flattened into a 1D vector so we can pass it to Fortran
    // The first dimension is the wavelength bins, the second is the vertical centers
    std::vector<double> radiative_intensity; // Radiative intensity at wavelength bins and vertical centers [W/m²/sr/m]
    int radiative_intensity_dim_1_size = 0; // Number of wavelength bins
    int radiative_intensity_dim_2_size = 0; // Number of vertical centers
  };

  class CARMAState
  {
   public:
    explicit CARMAState(const CARMA& carma, const CARMAStateParameters& params);

    ~CARMAState();

    void SetBin(int bin_index, int element_index, const std::vector<double>& values);
    void SetDetrain(int bin_index, int element_index, const std::vector<double>& values);
    void SetGas(
        int gas_index,
        const std::vector<double>& values,
        const std::vector<double>& old_mmr,
        const std::vector<double>& gas_saturation_wrt_ice,
        const std::vector<double>& gas_saturation_wrt_liquid);

   private:
    void* f_carma_state_;
  };

}  // namespace musica