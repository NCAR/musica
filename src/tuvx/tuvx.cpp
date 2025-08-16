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
      : tuvx_(nullptr),
        is_config_only_mode_(false)
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
        is_config_only_mode_ = false;
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

  void TUVX::CreateFromConfigOnly(const char *config_path)
  {
    int error_code = 0;

    // check that the file exists
    if (!std::filesystem::exists(config_path))
    {
      throw std::runtime_error("Config file does not exist: " + std::string(config_path));
    }

    tuvx_ = InternalCreateTuvxFromConfig(config_path, strlen(config_path), &error_code);
    if (error_code != 0 || tuvx_ == nullptr)
    {
      throw std::runtime_error("Failed to create tuvx instance from config");
    }

    is_config_only_mode_ = true;
    // Get number of layers for this mode
    this->number_of_layers_ = InternalGetNumberOfLayers(tuvx_, &error_code);
    if (error_code != 0)
    {
      throw std::runtime_error("Failed to get number of layers");
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

  void TUVX::RunFromConfig(double *const photolysis_rate_constants, double *const heating_rates)
  {
    if (!is_config_only_mode_)
    {
      throw std::runtime_error("RunFromConfig can only be used with CreateFromConfigOnly");
    }

    int error_code = 0;
    InternalRunTuvxFromConfig(tuvx_, photolysis_rate_constants, heating_rates, &error_code);
    if (error_code != 0)
    {
      throw std::runtime_error("Failed to run TUV-x from config");
    }
  }

  int TUVX::GetPhotolysisRateCount()
  {
    if (!is_config_only_mode_)
    {
      throw std::runtime_error("GetPhotolysisRateCount requires config-only mode");
    }

    int error_code = 0;
    int count = InternalGetPhotolysisRateCount(tuvx_, &error_code);
    if (error_code != 0)
    {
      throw std::runtime_error("Failed to get photolysis rate count");
    }
    return count;
  }

  int TUVX::GetHeatingRateCount()
  {
    if (!is_config_only_mode_)
    {
      throw std::runtime_error("GetHeatingRateCount requires config-only mode");
    }

    int error_code = 0;
    int count = InternalGetHeatingRateCount(tuvx_, &error_code);
    if (error_code != 0)
    {
      throw std::runtime_error("Failed to get heating rate count");
    }
    return count;
  }

  int TUVX::GetNumberOfLayers()
  {
    if (!is_config_only_mode_)
    {
      throw std::runtime_error("GetNumberOfLayers requires config-only mode");
    }

    int error_code = 0;
    int count = InternalGetNumberOfLayers(tuvx_, &error_code);
    if (error_code != 0)
    {
      throw std::runtime_error("Failed to get number of layers");
    }
    return count;
  }

  int TUVX::GetNumberOfSzaSteps()
  {
    if (!is_config_only_mode_)
    {
      throw std::runtime_error("GetNumberOfSzaSteps requires config-only mode");
    }

    int error_code = 0;
    int count = InternalGetNumberOfSzaSteps(tuvx_, &error_code);
    if (error_code != 0)
    {
      throw std::runtime_error("Failed to get number of SZA steps");
    }
    return count;
  }

  std::vector<std::string> TUVX::GetPhotolysisRateNames()
  {
    std::vector<std::string> names;

    if (!is_config_only_mode_)
    {
      throw std::runtime_error("GetPhotolysisRateNames requires config-only mode");
    }

    // For now, return placeholder names since we'd need to implement
    // GetPhotolysisRateNames in Fortran to get actual names
    int error_code = 0;
    int count = InternalGetPhotolysisRateCount(tuvx_, &error_code);
    if (error_code != 0)
    {
      throw std::runtime_error("Failed to get photolysis rate count");
    }

    for (int i = 0; i < count; ++i)
    {
      names.push_back("photolysis_" + std::to_string(i));
    }
    return names;
  }

  std::vector<std::string> TUVX::GetHeatingRateNames()
  {
    std::vector<std::string> names;

    if (!is_config_only_mode_)
    {
      throw std::runtime_error("GetHeatingRateNames requires config-only mode");
    }

    // For now, return placeholder names since we'd need to implement
    // GetHeatingRateNames in Fortran to get actual names
    int error_code = 0;
    int count = InternalGetHeatingRateCount(tuvx_, &error_code);
    if (error_code != 0)
    {
      throw std::runtime_error("Failed to get heating rate count");
    }

    for (int i = 0; i < count; ++i)
    {
      names.push_back("heating_" + std::to_string(i));
    }
    return names;
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