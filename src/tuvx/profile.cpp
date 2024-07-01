// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the TUVX class, which represents a multi-component
// reactive transport model. It also includes functions for creating and deleting TUVX instances.
#include <musica/tuvx/profile.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

namespace musica
{

  // Profile external C API functions

  void DeleteProfile(Profile *profile, Error *error)
  {
    *error = NoError();
    try
    {
      delete profile;
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
  }

  void SetProfileEdgeValues(Profile *profile, double edge_values[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    profile->SetEdgeValues(edge_values, num_values, error);
  }

  void SetProfileMidpointValues(Profile *profile, double midpoint_values[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    profile->SetMidpointValues(midpoint_values, num_values, error);
  }

  void SetProfileLayerDensities(Profile *profile, double layer_densities[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    profile->SetLayerDensities(layer_densities, num_values, error);
  }

  void SetProfileExoLayerDensity(Profile *profile, double exo_layer_density, Error *error)
  {
    DeleteError(error);
    profile->SetExoLayerDensity(exo_layer_density, error);
  }

  void CalculateProfileExoLayerDensity(Profile *profile, double scale_height, Error *error)
  {
    DeleteError(error);
    profile->CalculateExoLayerDensity(scale_height, error);
  }

  // Profile class functions

  Profile::~Profile()
  {
    int error_code = 0;
    if (profile_ != nullptr)
      InternalDeleteProfile(profile_, &error_code);
    profile_ = nullptr;
  }

  void Profile::SetEdgeValues(double edge_values[], std::size_t num_values, Error *error)
  {
    int error_code = 0;
    InternalSetEdgeValues(profile_, edge_values, num_values, &error_code);
    *error = NoError();
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set edge values") };
    }
  }

  void Profile::SetMidpointValues(double midpoint_values[], std::size_t num_values, Error *error)
  {
    int error_code = 0;
    InternalSetMidpointValues(profile_, midpoint_values, num_values, &error_code);
    *error = NoError();
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set midpoint values") };
    }
  }

  void Profile::SetLayerDensities(double layer_densities[], std::size_t num_values, Error *error)
  {
    int error_code = 0;
    InternalSetLayerDensities(profile_, layer_densities, num_values, &error_code);
    *error = NoError();
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set layer densities") };
    }
  }

  void Profile::SetExoLayerDensity(double exo_layer_density, Error *error)
  {
    int error_code = 0;
    InternalSetExoLayerDensity(profile_, exo_layer_density, &error_code);
    *error = NoError();
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set exo layer density") };
    }
  }

  void Profile::CalculateExoLayerDensity(double scale_height, Error *error)
  {
    int error_code = 0;
    InternalCalculateExoLayerDensity(profile_, scale_height, &error_code);
    *error = NoError();
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to calculate exo layer density") };
    }
  }

}  // namespace musica
