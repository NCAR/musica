// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the TUVX class, which represents a multi-component
// reactive transport model. It also includes functions for creating and deleting TUVX instances.
#include <musica/tuvx/tuvx.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>
#include <numbers>

namespace musica
{

  // TUVX external C API functions
  TUVX *CreateTuvx(const char *config_path, GridMap *grids, ProfileMap *profiles, RadiatorMap *radiators, Error *error)
  {
    DeleteError(error);
    TUVX *tuvx = new TUVX();

    tuvx->Create(config_path, grids, profiles, radiators, error);
    if (!IsSuccess(*error))
    {
      delete tuvx;
      return nullptr;
    }
    return tuvx;
  }

  void DeleteTuvx(const TUVX *tuvx, Error *error)
  {
    DeleteError(error);
    if (tuvx == nullptr)
    {
      *error = NoError();
      return;
    }
    try
    {
      delete tuvx;
      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
  }

  GridMap *GetGridMap(TUVX *tuvx, Error *error)
  {
    DeleteError(error);

    return tuvx->CreateGridMap(error);
  }

  ProfileMap *GetProfileMap(TUVX *tuvx, Error *error)
  {
    DeleteError(error);
    return tuvx->CreateProfileMap(error);
  }

  RadiatorMap *GetRadiatorMap(TUVX *tuvx, Error *error)
  {
    DeleteError(error);
    return tuvx->CreateRadiatorMap(error);
  }

  Mappings GetPhotolysisRateConstantsOrdering(TUVX *tuvx, Error *error)
  {
    DeleteError(error);
    return tuvx->GetPhotolysisRateConstantsOrdering(error);
  }

  Mappings GetHeatingRatesOrdering(TUVX *tuvx, Error *error)
  {
    DeleteError(error);
    return tuvx->GetHeatingRatesOrdering(error);
  }

  void RunTuvx(
      TUVX *const tuvx,
      const double solar_zenith_angle,
      const double earth_sun_distance,
      double *const photolysis_rate_constants,
      double *const heating_rates,
      Error *const error)
  {
    DeleteError(error);
    tuvx->Run(solar_zenith_angle, earth_sun_distance, photolysis_rate_constants, heating_rates, error);
  }

  // TUVX class functions

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

}  // namespace musica
