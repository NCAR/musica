// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the TUVX class, which creates C connections
// to the TUV-x photolysis calculator, allowing it to be used in a C++ context.
#include <musica/tuvx/tuvx.hpp>
#include <musica/tuvx/tuvx_c_interface.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>
#include <numbers>

namespace musica
{
  TUVX::TUVX()
      : tuvx_(nullptr)
  {
  }

  TUVX::~TUVX()
  {
    int error_code = 0;
    if (tuvx_ != nullptr)
    {
      InternalDeleteTuvx(tuvx_, &error_code);
    }
    tuvx_ = nullptr;
  }

  void TUVX::Create(const char *config_path, GridMap *grids, ProfileMap *profiles, RadiatorMap *radiators, Error *error)
  {
    int parsing_status = 0;  // 0 on success, 1 on failure
    try
    {
      // check that the file exists
      if (!std::filesystem::exists(config_path))
      {
        ToError(MUSICA_ERROR_CATEGORY, 1, "Config file does not exist", error);
        return;
      }

      tuvx_ = InternalCreateTuvx(
          config_path,
          strlen(config_path),
          grids->grid_map_,
          profiles->profile_map_,
          radiators->radiator_map_,
          &(this->number_of_layers_),
          &parsing_status);
      if (parsing_status == 1)
      {
        ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to create tuvx instance", error);
      }
      else
      {
        NoError(error);
      }
    }
    catch (const std::system_error &e)
    {
      ToError(e, error);
    }
    catch (...)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to create tuvx instance", error);
    }
  }

  void TUVX::CreateFromConfigString(
      const char *config_string,
      GridMap *grids,
      ProfileMap *profiles,
      RadiatorMap *radiators,
      Error *error)
  {
    int error_code = 0;
    if (config_string == nullptr || strlen(config_string) == 0)
    {
      throw std::runtime_error("Configuration string is empty");
    }

    try
    {
      tuvx_ = InternalCreateTuvxFromConfigString(
          config_string,
          strlen(config_string),
          grids->grid_map_,
          profiles->profile_map_,
          radiators->radiator_map_,
          &(this->number_of_layers_),
          &error_code);
      if (error_code != 0 || tuvx_ == nullptr)
      {
        ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to create TUV-x instance from config string", error);
      }
      else
      {
        NoError(error);
      }
    }
    catch (const std::system_error &e)
    {
      ToError(e, error);
    }
    catch (...)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to create TUV-x instance from config string", error);
    }
  }

  GridMap *TUVX::CreateGridMap(Error *error)
  {
    int error_code = 0;

    GridMap *grid_map = new GridMap(InternalGetGridMap(tuvx_, &error_code));
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to create grid map", error);
    }
    else
    {
      NoError(error);
    }
    return grid_map;
  }

  ProfileMap *TUVX::CreateProfileMap(Error *error)
  {
    int error_code = 0;
    ProfileMap *profile_map = new ProfileMap(InternalGetProfileMap(tuvx_, &error_code));
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to create profile map", error);
    }
    else
    {
      NoError(error);
    }
    return profile_map;
  }

  RadiatorMap *TUVX::CreateRadiatorMap(Error *error)
  {
    int error_code = 0;
    RadiatorMap *radiator_map = new RadiatorMap(InternalGetRadiatorMap(tuvx_, &error_code));
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to create radiator map", error);
    }
    else
    {
      NoError(error);
    }
    return radiator_map;
  }

  void TUVX::GetPhotolysisRateConstantsOrdering(Mappings *mappings, Error *error)
  {
    int error_code = 0;
    InternalGetPhotolysisRateConstantsOrdering(tuvx_, mappings, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to get photolysis rate constants ordering", error);
    }
    else
    {
      NoError(error);
    }
  }

  void TUVX::GetHeatingRatesOrdering(Mappings *mappings, Error *error)
  {
    int error_code = 0;
    InternalGetHeatingRatesOrdering(tuvx_, mappings, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to get heating rates ordering", error);
    }
    else
    {
      NoError(error);
    }
  }

  void TUVX::GetDoseRatesOrdering(Mappings *mappings, Error *error)
  {
    int error_code = 0;
    InternalGetDoseRatesOrdering(tuvx_, mappings, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to get dose rates ordering", error);
    }
    else
    {
      NoError(error);
    }
  }

  void TUVX::Run(
      const double solar_zenith_angle,
      const double earth_sun_distance,
      double *const photolysis_rate_constants,
      double *const heating_rates,
      double *const dose_rates,
      Error *const error)
  {
    int error_code = 0;
    const double sza_degrees = solar_zenith_angle * 180.0 / std::numbers::pi;
    try
    {
      InternalRunTuvx(
          tuvx_,
          this->number_of_layers_,
          sza_degrees,
          earth_sun_distance,
          photolysis_rate_constants,
          heating_rates,
          dose_rates,
          &error_code);
    }
    catch (const std::system_error &e)
    {
      ToError(e, error);
      return;
    }
    catch (...)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to run TUV-x", error);
      return;
    }
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to run TUV-x", error);
      return;
    }
  }

  int TUVX::GetPhotolysisRateConstantCount()
  {
    int error_code = 0;
    int const count = InternalGetPhotolysisRateConstantCount(tuvx_, &error_code);
    if (error_code != 0)
    {
      throw std::runtime_error("Failed to get photolysis rate constant count");
    }
    return count;
  }

  int TUVX::GetHeatingRateCount()
  {
    int error_code = 0;
    int const count = InternalGetHeatingRateCount(tuvx_, &error_code);
    if (error_code != 0)
    {
      throw std::runtime_error("Failed to get heating rate count");
    }
    return count;
  }

  int TUVX::GetDoseRateCount()
  {
    int error_code = 0;
    int const count = InternalGetDoseRateCount(tuvx_, &error_code);
    if (error_code != 0)
    {
      throw std::runtime_error("Failed to get dose rate count");
    }
    return count;
  }

  int TUVX::GetNumberOfLayers()
  {
    int error_code = 0;
    int const count = InternalGetNumberOfLayers(tuvx_, &error_code);
    if (error_code != 0)
    {
      throw std::runtime_error("Failed to get number of layers");
    }
    return count;
  }

  std::string TUVX::GetVersion()
  {
    char *version_ptr;
    int version_length;
    InternalGetTuvxVersion(&version_ptr, &version_length);

    std::string version_str(version_ptr, version_length);

    // Free the memory allocated by Fortran
    InternalFreeTuvxVersion(version_ptr, version_length);

    return version_str;
  }

}  // namespace musica