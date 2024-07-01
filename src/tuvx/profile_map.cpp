// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the TUVX class, which represents a multi-component
// reactive transport model. It also includes functions for creating and deleting TUVX instances.
#include <musica/tuvx/profile_map.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

namespace musica
{

  // ProfileMap external C API functions

  void DeleteProfileMap(ProfileMap *profile_map, Error *error)
  {
    *error = NoError();
    try
    {
      delete profile_map;
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
  }

  Profile *GetProfile(ProfileMap *profile_map, const char *profile_name, const char *profile_units, Error *error)
  {
    DeleteError(error);
    return profile_map->GetProfile(profile_name, profile_units, error);
  }

  // ProfileMap class functions

  ProfileMap::~ProfileMap()
  {
    // At the time of writing, the profile map pointer is owned by fortran memory
    // in the tuvx core and should not be deleted here. It will be deleted when
    // the tuvx instance is deleted
    int error_code = 0;
    profile_map_ = nullptr;
  }

  Profile *ProfileMap::GetProfile(const char *profile_name, const char *profile_units, Error *error)
  {
    if (profile_map_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Profile map is null") };
      return nullptr;
    }

    int error_code = 0;
    Profile *profile = nullptr;

    try
    {
      *error = NoError();

      profile = new Profile(InternalGetProfile(profile_map_, profile_name, strlen(profile_name), profile_units, strlen(profile_units), &error_code));

      if (error_code != 0)
      {
        delete profile;
        profile = nullptr;
        *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create profile map") };
      }
      else
      {
        profiles_.push_back(std::unique_ptr<Profile>(profile));
      }
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
    catch (...)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create profile") };
    }

    return profile;
  }

}  // namespace musica
