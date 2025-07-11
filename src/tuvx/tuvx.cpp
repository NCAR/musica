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
      InternalDeleteTuvx(tuvx_, &error_code);
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
        *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Config file does not exist") };
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
        *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create tuvx instance") };
      }
      else
      {
        *error = NoError();
      }
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
    catch (...)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create tuvx instance") };
    }
  }

  GridMap *TUVX::CreateGridMap(Error *error)
  {
    *error = NoError();
    int error_code = 0;

    GridMap *grid_map = new GridMap(InternalGetGridMap(tuvx_, &error_code));
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create grid map") };
      return nullptr;
    }
    return grid_map;
  }

  ProfileMap *TUVX::CreateProfileMap(Error *error)
  {
    *error = NoError();
    int error_code = 0;
    ProfileMap *profile_map = new ProfileMap(InternalGetProfileMap(tuvx_, &error_code));
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create profile map") };
      return nullptr;
    }
    return profile_map;
  }

  RadiatorMap *TUVX::CreateRadiatorMap(Error *error)
  {
    *error = NoError();
    int error_code = 0;
    RadiatorMap *radiator_map = new RadiatorMap(InternalGetRadiatorMap(tuvx_, &error_code));
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create radiator map") };
      return nullptr;
    }
    return radiator_map;
  }

  Mappings TUVX::GetPhotolysisRateConstantsOrdering(Error *error)
  {
    *error = NoError();
    int error_code = 0;
    Mappings mappings = InternalGetPhotolysisRateConstantsOrdering(tuvx_, &error_code);
    if (error_code != 0)
    {
      *error =
          Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to get photolysis rate constants ordering") };
    }
    return mappings;
  }

  Mappings TUVX::GetHeatingRatesOrdering(Error *error)
  {
    *error = NoError();
    int error_code = 0;
    Mappings mappings = InternalGetHeatingRatesOrdering(tuvx_, &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to get heating rates ordering") };
    }
    return mappings;
  }

  void TUVX::Run(
      const double solar_zenith_angle,
      const double earth_sun_distance,
      double *const photolysis_rate_constants,
      double *const heating_rates,
      Error *const error)
  {
    *error = NoError();
    int error_code = 0;
    double sza_degrees = solar_zenith_angle * 180.0 / std::numbers::pi;
    InternalRunTuvx(
        tuvx_,
        this->number_of_layers_,
        sza_degrees,
        earth_sun_distance,
        photolysis_rate_constants,
        heating_rates,
        &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to run TUV-x") };
    }
  }

  std::string TUVX::GetVersion()
  {
    char *version_ptr;
    int version_length;
    InternalGetTuvxVersion(&version_ptr, &version_length);

    std::string version_str(version_ptr, version_length);

    // Free the memory allocated by Fortran
    InternalFreeTuvxVersion(version_ptr);

    return version_str;
  }

}  // namespace musica
