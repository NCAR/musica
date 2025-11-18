// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/tuvx/grid.hpp>
#include <musica/tuvx/profile.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

namespace
{
  constexpr int ERROR_NONE = 0;
  constexpr int ERROR_UNALLOCATED_PROFILE_UPDATER = 1;
  constexpr int ERROR_PROFILE_SIZE_MISMATCH = 2;
  constexpr const char *GetErrorMessage(int error_code)
  {
    switch (error_code)
    {
      case ERROR_NONE: return "No error";
      case ERROR_UNALLOCATED_PROFILE_UPDATER: return "Profile updater is unallocated";
      case ERROR_PROFILE_SIZE_MISMATCH: return "Profile size mismatch";
      default: return "Unknown error";
    }
  }
}  // namespace

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
      ToError(e, error);
      return;
    }
    NoError(error);
  }

  const char *GetProfileName(Profile *profile, Error *error)
  {
    DeleteError(error);
    try
    {
      std::string const name = profile->GetName(error);
      if (error->code_ != 0)
        return nullptr;
      char *name_cstr = new char[name.size() + 1];
      std::strcpy(name_cstr, name.c_str());
      return name_cstr;
    }
    catch (const std::system_error &e)
    {
      ToError(e, error);
      return nullptr;
    }
  }

  const char *GetProfileUnits(Profile *profile, Error *error)
  {
    DeleteError(error);
    try
    {
      std::string const units = profile->GetUnits(error);
      if (error->code_ != 0)
        return nullptr;
      char *units_cstr = new char[units.size() + 1];
      std::strcpy(units_cstr, units.c_str());
      return units_cstr;
    }
    catch (const std::system_error &e)
    {
      ToError(e, error);
      return nullptr;
    }
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

  std::size_t GetProfileNumberOfSections(Profile *profile, Error *error)
  {
    DeleteError(error);
    return profile->GetNumberOfSections(error);
  }

  // Profile class functions

  Profile::Profile(const char *profile_name, const char *units, Grid *grid, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    profile_ = InternalCreateProfile(profile_name, strlen(profile_name), units, strlen(units), grid->updater_, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    updater_ = InternalGetProfileUpdater(profile_, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      int cleanup_error = 0;
      InternalDeleteProfile(profile_, &cleanup_error);
      return;
    }
    NoError(error);
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

  std::string Profile::GetName(Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(
          MUSICA_ERROR_CATEGORY,
          ERROR_UNALLOCATED_PROFILE_UPDATER,
          GetErrorMessage(ERROR_UNALLOCATED_PROFILE_UPDATER),
          error);
      return "";
    }
    String name;
    InternalGetProfileName(updater_, &name, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return "";
    }
    std::string result(name.value_, name.size_);
    delete[] name.value_;
    NoError(error);
    return result;
  }

  std::string Profile::GetUnits(Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(
          MUSICA_ERROR_CATEGORY,
          ERROR_UNALLOCATED_PROFILE_UPDATER,
          GetErrorMessage(ERROR_UNALLOCATED_PROFILE_UPDATER),
          error);
      return "";
    }
    String units;
    InternalGetProfileUnits(updater_, &units, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return "";
    }
    std::string result(units.value_, units.size_);
    delete[] units.value_;
    NoError(error);
    return result;
  }

  void Profile::SetEdgeValues(double edge_values[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(
          MUSICA_ERROR_CATEGORY,
          ERROR_UNALLOCATED_PROFILE_UPDATER,
          GetErrorMessage(ERROR_UNALLOCATED_PROFILE_UPDATER),
          error);
      return;
    }
    InternalSetEdgeValues(updater_, edge_values, num_values, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    NoError(error);
  }

  void Profile::GetEdgeValues(double edge_values[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(
          MUSICA_ERROR_CATEGORY,
          ERROR_UNALLOCATED_PROFILE_UPDATER,
          GetErrorMessage(ERROR_UNALLOCATED_PROFILE_UPDATER),
          error);
      return;
    }
    InternalGetEdgeValues(updater_, edge_values, num_values, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    NoError(error);
  }

  void Profile::SetMidpointValues(double midpoint_values[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(
          MUSICA_ERROR_CATEGORY,
          ERROR_UNALLOCATED_PROFILE_UPDATER,
          GetErrorMessage(ERROR_UNALLOCATED_PROFILE_UPDATER),
          error);
      return;
    }
    InternalSetMidpointValues(updater_, midpoint_values, num_values, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    NoError(error);
  }

  void Profile::GetMidpointValues(double midpoint_values[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(
          MUSICA_ERROR_CATEGORY,
          ERROR_UNALLOCATED_PROFILE_UPDATER,
          GetErrorMessage(ERROR_UNALLOCATED_PROFILE_UPDATER),
          error);
      return;
    }
    InternalGetMidpointValues(updater_, midpoint_values, num_values, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    NoError(error);
  }

  void Profile::SetLayerDensities(double layer_densities[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(
          MUSICA_ERROR_CATEGORY,
          ERROR_UNALLOCATED_PROFILE_UPDATER,
          GetErrorMessage(ERROR_UNALLOCATED_PROFILE_UPDATER),
          error);
      return;
    }
    InternalSetLayerDensities(updater_, layer_densities, num_values, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    NoError(error);
  }

  void Profile::GetLayerDensities(double layer_densities[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(
          MUSICA_ERROR_CATEGORY,
          ERROR_UNALLOCATED_PROFILE_UPDATER,
          GetErrorMessage(ERROR_UNALLOCATED_PROFILE_UPDATER),
          error);
      return;
    }
    InternalGetLayerDensities(updater_, layer_densities, num_values, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    NoError(error);
  }

  void Profile::SetExoLayerDensity(double exo_layer_density, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(
          MUSICA_ERROR_CATEGORY,
          ERROR_UNALLOCATED_PROFILE_UPDATER,
          GetErrorMessage(ERROR_UNALLOCATED_PROFILE_UPDATER),
          error);
      return;
    }
    InternalSetExoLayerDensity(updater_, exo_layer_density, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    NoError(error);
  }

  void Profile::CalculateExoLayerDensity(double scale_height, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(
          MUSICA_ERROR_CATEGORY,
          ERROR_UNALLOCATED_PROFILE_UPDATER,
          GetErrorMessage(ERROR_UNALLOCATED_PROFILE_UPDATER),
          error);
      return;
    }
    InternalCalculateExoLayerDensity(updater_, scale_height, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    NoError(error);
  }

  double Profile::GetExoLayerDensity(Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(
          MUSICA_ERROR_CATEGORY,
          ERROR_UNALLOCATED_PROFILE_UPDATER,
          GetErrorMessage(ERROR_UNALLOCATED_PROFILE_UPDATER),
          error);
      return 0.0;
    }
    double const exo_layer_density = InternalGetExoLayerDensity(updater_, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return 0.0;
    }
    NoError(error);
    return exo_layer_density;
  }

  std::size_t Profile::GetNumberOfSections(Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(
          MUSICA_ERROR_CATEGORY,
          ERROR_UNALLOCATED_PROFILE_UPDATER,
          GetErrorMessage(ERROR_UNALLOCATED_PROFILE_UPDATER),
          error);
      return 0;
    }
    std::size_t const num_sections = InternalProfileGetNumberOfSections(updater_, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return 0;
    }
    NoError(error);
    return num_sections;
  }

}  // namespace musica
