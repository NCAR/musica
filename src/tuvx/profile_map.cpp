// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/tuvx/profile_map.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

namespace
{
  constexpr int ERROR_NONE = 0;
  constexpr int ERROR_UNALLOCATED_PROFILE_MAP = 201;
  constexpr int ERROR_UNALLOCATED_PROFILE = 202;
  constexpr int ERROR_UNALLOCATED_PROFILE_UPDATER = 203;
  constexpr int ERROR_PROFILE_NAME_NOT_FOUND = 204;
  constexpr int ERROR_PROFILE_UNITS_MISMATCH = 205;
  constexpr int ERROR_PROFILE_TYPE_MISMATCH = 206;
  constexpr int ERROR_INDEX_OUT_OF_BOUNDS = 207;
  constexpr int INTERNAL_PROFILE_MAP_ERROR = 299;
  constexpr const char *GetErrorMessage(int error_code)
  {
    switch (error_code)
    {
      case ERROR_NONE: return "No error";
      case ERROR_UNALLOCATED_PROFILE_MAP: return "Profile map is unallocated";
      case ERROR_UNALLOCATED_PROFILE: return "Profile is unallocated";
      case ERROR_UNALLOCATED_PROFILE_UPDATER: return "Profile updater is unallocated";
      case ERROR_PROFILE_NAME_NOT_FOUND: return "Profile name not found in map";
      case ERROR_PROFILE_UNITS_MISMATCH: return "Profile units mismatch";
      case ERROR_PROFILE_TYPE_MISMATCH: return "Profile type mismatch";
      case ERROR_INDEX_OUT_OF_BOUNDS: return "Index out of bounds";
      default: return "Unknown error";
    }
  }
}  // namespace

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
      *error = ToError(e);
    }
    *error = NoError();
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

  Profile *GetProfileByIndex(ProfileMap *profile_map, std::size_t index, Error *error)
  {
    DeleteError(error);
    return profile_map->GetProfileByIndex(index, error);
  }

  void RemoveProfile(ProfileMap *profile_map, const char *profile_name, const char *profile_units, Error *error)
  {
    DeleteError(error);
    profile_map->RemoveProfile(profile_name, profile_units, error);
  }

  void RemoveProfileByIndex(ProfileMap *profile_map, std::size_t index, Error *error)
  {
    DeleteError(error);
    profile_map->RemoveProfileByIndex(index, error);
  }

  std::size_t GetNumberOfProfiles(ProfileMap *profile_map, Error *error)
  {
    DeleteError(error);
    return profile_map->GetNumberOfProfiles(error);
  }

  // ProfileMap class functions

  ProfileMap::ProfileMap(Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    profile_map_ = InternalCreateProfileMap(&error_code);
    if (error_code != 0)
    {
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
    }
    owns_profile_map_ = true;
    *error = NoError();
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
    DeleteError(error);
    if (profile_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_PROFILE_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_PROFILE_MAP)) };
      return;
    }
    if (profile->profile_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_PROFILE,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_PROFILE)) };
      return;
    }
    if (profile->updater_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_PROFILE_UPDATER,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_PROFILE_UPDATER)) };
      return;
    }

    int error_code = 0;

    try
    {
      InternalAddProfile(profile_map_, profile->profile_, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      }
      InternalDeleteProfileUpdater(profile->updater_, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      }
      profile->updater_ = InternalGetProfileUpdaterFromMap(profile_map_, profile->profile_, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      }
      InternalDeleteProfile(profile->profile_, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      }
      profile->profile_ = nullptr;
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
      return;
    }
    catch (...)
    {
      *error = Error{ INTERNAL_PROFILE_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_PROFILE_MAP_ERROR)) };
      return;
    }
    *error = NoError();
  }

  Profile *ProfileMap::GetProfile(const char *profile_name, const char *profile_units, Error *error)
  {
    DeleteError(error);
    if (profile_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_PROFILE_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_PROFILE_MAP)) };
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
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        return nullptr;
      }
      void *updater_ptr = InternalGetProfileUpdaterFromMap(profile_map_, profile_ptr, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        InternalDeleteProfile(profile_ptr, &error_code);
        return nullptr;
      }
      InternalDeleteProfile(profile_ptr, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        InternalDeleteProfileUpdater(updater_ptr, &error_code);
        return nullptr;
      }
      profile = new Profile(updater_ptr);
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
      return nullptr;
    }
    catch (...)
    {
      *error = Error{ INTERNAL_PROFILE_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_PROFILE_MAP_ERROR)) };
      return nullptr;
    }
    *error = NoError();
    return profile;
  }

  Profile *ProfileMap::GetProfileByIndex(std::size_t index, Error *error)
  {
    DeleteError(error);
    if (profile_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_PROFILE_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_PROFILE_MAP)) };
      return nullptr;
    }

    Profile *profile = nullptr;

    try
    {
      int error_code = 0;
      void *profile_ptr = InternalGetProfileByIndex(profile_map_, index, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        return nullptr;
      }
      void *updater_ptr = InternalGetProfileUpdaterFromMap(profile_map_, profile_ptr, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        InternalDeleteProfile(profile_ptr, &error_code);
        return nullptr;
      }
      InternalDeleteProfile(profile_ptr, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        InternalDeleteProfileUpdater(updater_ptr, &error_code);
        return nullptr;
      }
      profile = new Profile(updater_ptr);
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
      return nullptr;
    }
    catch (...)
    {
      *error = Error{ INTERNAL_PROFILE_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_PROFILE_MAP_ERROR)) };
      return nullptr;
    }
    *error = NoError();
    return profile;
  }

  void ProfileMap::RemoveProfile(const char *profile_name, const char *profile_units, Error *error)
  {
    DeleteError(error);
    if (profile_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_PROFILE_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_PROFILE_MAP)) };
      return;
    }

    int error_code = 0;

    try
    {
      InternalRemoveProfile(
          profile_map_, profile_name, strlen(profile_name), profile_units, strlen(profile_units), &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        return;
      }
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
      return;
    }
    catch (...)
    {
      *error = Error{ INTERNAL_PROFILE_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_PROFILE_MAP_ERROR)) };
      return;
    }
    *error = NoError();
  }

  void ProfileMap::RemoveProfileByIndex(std::size_t index, Error *error)
  {
    DeleteError(error);
    if (profile_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_PROFILE_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_PROFILE_MAP)) };
      return;
    }

    int error_code = 0;

    try
    {
      InternalRemoveProfileByIndex(profile_map_, index, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        return;
      }
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
      return;
    }
    catch (...)
    {
      *error = Error{ INTERNAL_PROFILE_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_PROFILE_MAP_ERROR)) };
      return;
    }
    *error = NoError();
  }

  std::size_t ProfileMap::GetNumberOfProfiles(Error *error)
  {
    DeleteError(error);
    if (profile_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_PROFILE_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_PROFILE_MAP)) };
      return 0;
    }

    std::size_t num_profiles = 0;

    try
    {
      int error_code = 0;
      num_profiles = InternalGetNumberOfProfiles(profile_map_, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        return 0;
      }
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
      return 0;
    }
    catch (...)
    {
      *error = Error{ INTERNAL_PROFILE_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_PROFILE_MAP_ERROR)) };
      return 0;
    }
    *error = NoError();
    return num_profiles;
  }
}  // namespace musica
