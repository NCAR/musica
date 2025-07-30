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

  struct CarmaStatistics {
    int max_number_of_substeps;  // Maximum number of substeps taken in the last run
    double max_number_of_retries;  // Maximum number of retries for convergence
    double total_number_of_steps;  // Total number of steps taken in the last run
    int total_number_of_substeps;  // Total number of substeps taken in the last run
    double total_number_of_retries;  // Total number of retries for convergence
    std::vector<double> z_substeps;  // number of substeps per vertical level
    double xc;  // x location at the center of this CARMA state
    double yc;  // y location at the center of this CARMA state
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

   private:
    void* f_carma_state_;
    int nz; // Number of vertical levels
  };

}  // namespace musica