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

  class CARMAState
  {
   public:
    explicit CARMAState(CARMA* carma, const CARMAStateParameters& params);

    ~CARMAState();

    void SetBin(int bin_index, int element_index, const std::vector<double>& values);
    void SetDetrain(int bin_index, int element_index, const std::vector<double>& values);

   private:
    void* f_carma_state_;
  };

}  // namespace musica