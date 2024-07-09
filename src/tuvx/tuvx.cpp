// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the TUVX class, which represents a multi-component
// reactive transport model. It also includes functions for creating and deleting TUVX instances.
#include <musica/tuvx/tuvx.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

namespace musica
{

  // TUVX external C API functions

  TUVX *CreateTuvx(const char *config_path, Error *error)
  {
    DeleteError(error);
    TUVX *tuvx = new TUVX();
    tuvx->Create(config_path, error);
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

  // TUVX class functions

  TUVX::TUVX()
      : tuvx_()
  {
  }

  TUVX::~TUVX()
  {
    int error_code = 0;
    if (tuvx_ != nullptr)
      InternalDeleteTuvx(tuvx_, &error_code);
    tuvx_ = nullptr;
  }

  void TUVX::Create(const char *config_path, Error *error)
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

      tuvx_ = InternalCreateTuvx(config_path, strlen(config_path), &parsing_status);
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
    GridMap* grid_map = new GridMap(InternalGetGridMap(tuvx_, &error_code));
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
    ProfileMap* profile_map = new ProfileMap(InternalGetProfileMap(tuvx_, &error_code));
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create profile map") };
      return nullptr;
    }
    return profile_map;
  }

}  // namespace musica
