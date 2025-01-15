// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/tuvx/grid.hpp>
#include <musica/tuvx/profile.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

namespace musica
{

  // Profile external C API functions

  Profile *CreateProfile(const char *profile_name, const char *units, Grid *grid, Error *error)
  {
    DeleteError(error);
    return new Profile(profile_name, units, grid, error);
  }

  void DeleteProfile(Profile *profile, Error *error)
  {
    DeleteError(error);
    try
    {
      delete profile;
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
      return;
    }
    *error = NoError();
  }

  void SetProfileEdgeValues(Profile *profile, double edge_values[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    profile->SetEdgeValues(edge_values, num_values, error);
  }

  void GetProfileEdgeValues(Profile *profile, double edge_values[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    profile->GetEdgeValues(edge_values, num_values, error);
  }

  void SetProfileMidpointValues(Profile *profile, double midpoint_values[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    profile->SetMidpointValues(midpoint_values, num_values, error);
  }

  void GetProfileMidpointValues(Profile *profile, double midpoint_values[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    profile->GetMidpointValues(midpoint_values, num_values, error);
  }

  void SetProfileLayerDensities(Profile *profile, double layer_densities[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    profile->SetLayerDensities(layer_densities, num_values, error);
  }

  void GetProfileLayerDensities(Profile *profile, double layer_densities[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    profile->GetLayerDensities(layer_densities, num_values, error);
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

  double GetProfileExoLayerDensity(Profile *profile, Error *error)
  {
    DeleteError(error);
    return profile->GetExoLayerDensity(error);
  }

  // Profile class functions

  Profile::Profile(const char *profile_name, const char *units, Grid *grid, Error *error)
  {
    int error_code = 0;
    profile_ = InternalCreateProfile(profile_name, strlen(profile_name), units, strlen(units), grid->updater_, &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create profile") };
      return;
    }
    updater_ = InternalGetProfileUpdater(profile_, &error_code);
    if (error_code != 0)
    {
      InternalDeleteProfile(profile_, &error_code);
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to get updater") };
      return;
    }
    *error = NoError();
  }

  Profile::~Profile()
  {
    int error_code = 0;
    if (profile_ != nullptr)
      InternalDeleteProfile(profile_, &error_code);
    if (updater_ != nullptr)
      InternalDeleteProfileUpdater(updater_, &error_code);
    profile_ = nullptr;
    updater_ = nullptr;
  }

  void Profile::SetEdgeValues(double edge_values[], std::size_t num_values, Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Profile is not updatable") };
      return;
    }
    InternalSetEdgeValues(updater_, edge_values, num_values, &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set edge values") };
      return;
    }
    *error = NoError();
  }

  void Profile::GetEdgeValues(double edge_values[], std::size_t num_values, Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Profile is not accessible") };
      return;
    }
    InternalGetEdgeValues(updater_, edge_values, num_values, &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to get edge values") };
      return;
    }
    *error = NoError();
  }

  void Profile::SetMidpointValues(double midpoint_values[], std::size_t num_values, Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Profile is not updatable") };
      return;
    }
    InternalSetMidpointValues(updater_, midpoint_values, num_values, &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set midpoint values") };
      return;
    }
    *error = NoError();
  }

  void Profile::GetMidpointValues(double midpoint_values[], std::size_t num_values, Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Profile is not accessible") };
      return;
    }
    InternalGetMidpointValues(updater_, midpoint_values, num_values, &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to get midpoint values") };
      return;
    }
    *error = NoError();
  }

  void Profile::SetLayerDensities(double layer_densities[], std::size_t num_values, Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Profile is not updatable") };
      return;
    }
    InternalSetLayerDensities(updater_, layer_densities, num_values, &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set layer densities") };
      return;
    }
    *error = NoError();
  }

  void Profile::GetLayerDensities(double layer_densities[], std::size_t num_values, Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Profile is not accessible") };
      return;
    }
    InternalGetLayerDensities(updater_, layer_densities, num_values, &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to get layer densities") };
      return;
    }
    *error = NoError();
  }

  void Profile::SetExoLayerDensity(double exo_layer_density, Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Profile is not updatable") };
      return;
    }
    InternalSetExoLayerDensity(updater_, exo_layer_density, &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set exo layer density") };
      return;
    }
    *error = NoError();
  }

  void Profile::CalculateExoLayerDensity(double scale_height, Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Profile is not accessible") };
      return;
    }
    InternalCalculateExoLayerDensity(updater_, scale_height, &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to calculate exo layer density") };
      return;
    }
    *error = NoError();
  }

  double Profile::GetExoLayerDensity(Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Profile is not accessible") };
      return 0.0;
    }
    double exo_layer_density = InternalGetExoLayerDensity(updater_, &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to get exo layer density") };
      return 0.0;
    }
    *error = NoError();
    return exo_layer_density;
  }

}  // namespace musica
