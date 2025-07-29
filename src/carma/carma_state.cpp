
// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the CARMA state class
#include <musica/carma/carma_c_interface.hpp>
#include <musica/carma/carma_state.hpp>

#include <cstring>
#include <stdexcept>

namespace musica
{

  CARMAState::CARMAState(CARMA* carma, const CARMAStateParameters& params)
  {
    CCARMAParameters* carma_params = carma->GetParameters();
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
      std::string error_message = "Failed to create CARMA state with return code: " + std::to_string(rc);
      throw std::runtime_error(error_message);
    }
  }
}