// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/tuvx/profile_map.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

namespace musica
{

  // ProfileMap external C API functions

  ProfileMap *CreateProfileMap(Error *error)
  {
    DeleteError(error);
    return new ProfileMap(error);
  }

  void DeleteProfileMap(ProfileMap *profile_map, Error *error)
  {
    DeleteError(error);
    try
    {
      delete profile_map;
    }
    catch (const std::system_error &e)
    {
      Error* temp = ToError(e);
      *error = *temp;
      DeleteError(temp);
    }
    Error* temp = NoError();
    *error = *temp;
    DeleteError(temp);
  }

  void AddProfile(ProfileMap *profile_map, Profile *profile, Error *error)
  {
    DeleteError(error);
    profile_map->AddProfile(profile, error);
  }

  Profile *GetProfile(ProfileMap *profile_map, const char *profile_name, const char *profile_units, Error *error)
  {
    DeleteError(error);
    return profile_map->GetProfile(profile_name, profile_units, error);
  }

  // ProfileMap class functions

  ProfileMap::ProfileMap(Error *error)
  {
    int error_code = 0;
    profile_map_ = InternalCreateProfileMap(&error_code);
    if (error_code != 0)
    {
      SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to create profile map");
    }
    owns_profile_map_ = true;
    Error* temp = NoError();
    *error = *temp;
    DeleteError(temp);
  }

  ProfileMap::~ProfileMap()
  {
    int error_code = 0;
    if (profile_map_ != nullptr && owns_profile_map_)
    {
      InternalDeleteProfileMap(profile_map_, &error_code);
    }
    profile_map_ = nullptr;
    owns_profile_map_ = false;
  }

  void ProfileMap::AddProfile(Profile *profile, Error *error)
  {
    if (profile_map_ == nullptr)
    {
      SetError(error, 1, MUSICA_ERROR_CATEGORY, "Profile map is null");
      return;
    }
    if (profile->profile_ == nullptr)
    {
      SetError(error, 1, MUSICA_ERROR_CATEGORY, "Cannot add unowned profile");
      return;
    }
    if (profile->updater_ == nullptr)
    {
      SetError(error, 1, MUSICA_ERROR_CATEGORY, "Cannot add profile in invalid state");
      return;
    }

    int error_code = 0;

    try
    {
      InternalAddProfile(profile_map_, profile->profile_, &error_code);
      if (error_code != 0)
      {
        SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to add profile");
      }
      InternalDeleteProfileUpdater(profile->updater_, &error_code);
      if (error_code != 0)
      {
        SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to delete profile updater");
      }
      profile->updater_ = InternalGetProfileUpdaterFromMap(profile_map_, profile->profile_, &error_code);
      if (error_code != 0)
      {
        SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to get profile updater from map");
      }
      InternalDeleteProfile(profile->profile_, &error_code);
      if (error_code != 0)
      {
       SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to delete profile after transfer of ownership to profile map");
      }
      profile->profile_ = nullptr;
    }
    catch (const std::system_error &e)
    {
      Error* temp = ToError(e);
      *error = *temp;
      DeleteError(temp);
    }
    catch (...)
    {
      SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to add profile");
    }
    Error* temp = NoError();
    *error = *temp;
    DeleteError(temp);
  }

  Profile *ProfileMap::GetProfile(const char *profile_name, const char *profile_units, Error *error)
  {
    if (profile_map_ == nullptr)
    {
      SetError(error, 1, MUSICA_ERROR_CATEGORY, "Profile map is null");
      return nullptr;
    }

    Profile *profile = nullptr;

    try
    {
      int error_code = 0;
      void *profile_ptr = InternalGetProfile(
          profile_map_, profile_name, strlen(profile_name), profile_units, strlen(profile_units), &error_code);
      if (error_code != 0)
      {
       SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to get profile");
        return nullptr;
      }
      void *updater_ptr = InternalGetProfileUpdaterFromMap(profile_map_, profile_ptr, &error_code);
      if (error_code != 0)
      {
        SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to get updater");
        InternalDeleteProfile(profile_ptr, &error_code);
        return nullptr;
      }
      InternalDeleteProfile(profile_ptr, &error_code);
      if (error_code != 0)
      {
        SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to delete profile during transfer of ownership to profile map");
        InternalDeleteProfileUpdater(updater_ptr, &error_code);
        return nullptr;
      }
      profile = new Profile(updater_ptr);
    }
    catch (const std::system_error &e)
    {
      Error* temp = ToError(e);
      *error = *temp;
      DeleteError(temp);
    }
    catch (...)
    {
      SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to create profile");
    }
    Error* temp = NoError();
    *error = *temp;
    DeleteError(temp);
    return profile;
  }

}  // namespace musica
