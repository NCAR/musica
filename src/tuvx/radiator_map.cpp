// Copyright (C) 2023-2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/tuvx/radiator_map.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

namespace
{
  constexpr int ERROR_NONE = 0;
  constexpr int ERROR_UNALLOCATED_RADIATOR_MAP = 3301;
  constexpr int ERROR_RADIATOR_NOT_FOUND = 3302;
  constexpr int ERROR_RADIATOR_TYPE_MISMATCH = 3303;
  constexpr int ERROR_INDEX_OUT_OF_BOUNDS = 3304;
  constexpr int ERROR_UNALLOCATED_RADIATOR = 3305;
  constexpr int ERROR_UNALLOCATED_RADIATOR_UPDATER = 3306;
  constexpr int INTERNAL_RADIATOR_MAP_ERROR = 3399;
  constexpr const char *GetErrorMessage(int error_code)
  {
    switch (error_code)
    {
      case ERROR_NONE: return "No error";
      case ERROR_UNALLOCATED_RADIATOR_MAP: return "Radiator map is unallocated";
      case ERROR_RADIATOR_NOT_FOUND: return "Radiator not found in map";
      case ERROR_RADIATOR_TYPE_MISMATCH: return "Radiator type mismatch";
      case ERROR_INDEX_OUT_OF_BOUNDS: return "Index out of bounds";
      case ERROR_UNALLOCATED_RADIATOR: return "Radiator is unallocated";
      case ERROR_UNALLOCATED_RADIATOR_UPDATER: return "Radiator updater is unallocated";
      case INTERNAL_RADIATOR_MAP_ERROR: return "Internal radiator map error";
      default: return "Unknown error";
    }
  }
}  // namespace

namespace musica
{

  // RadiatordMap external C API functions

  RadiatorMap *CreateRadiatorMap(Error *error)
  {
    DeleteError(error);
    return new RadiatorMap(error);
  }

  void DeleteRadiatorMap(RadiatorMap *radiator_map, Error *error)
  {
    DeleteError(error);
    try
    {
      delete radiator_map;
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
    *error = NoError();
  }

  void AddRadiator(RadiatorMap *radiator_map, Radiator *radiator, Error *error)
  {
    DeleteError(error);
    radiator_map->AddRadiator(radiator, error);
  }

  Radiator *GetRadiator(RadiatorMap *radiator_map, const char *radiator_name, Error *error)
  {
    DeleteError(error);
    return radiator_map->GetRadiator(radiator_name, error);
  }

  Radiator *GetRadiatorByIndex(RadiatorMap *radiator_map, std::size_t index, Error *error)
  {
    DeleteError(error);
    return radiator_map->GetRadiatorByIndex(index, error);
  }

  void RemoveRadiator(RadiatorMap *radiator_map, const char *radiator_name, Error *error)
  {
    DeleteError(error);
    radiator_map->RemoveRadiator(radiator_name, error);
  }

  void RemoveRadiatorByIndex(RadiatorMap *radiator_map, std::size_t index, Error *error)
  {
    DeleteError(error);
    radiator_map->RemoveRadiatorByIndex(index, error);
  }

  std::size_t GetNumberOfRadiators(RadiatorMap *radiator_map, Error *error)
  {
    DeleteError(error);
    return radiator_map->GetNumberOfRadiators(error);
  }

  // RadiatorMap class functions

  RadiatorMap::RadiatorMap(Error *error)
  {
    int error_code = ERROR_NONE;
    radiator_map_ = InternalCreateRadiatorMap(&error_code);
    if (error_code != ERROR_NONE)
    {
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
    }
    owns_radiator_map_ = true;
    *error = NoError();
  }

  RadiatorMap::~RadiatorMap()
  {
    int error_code = ERROR_NONE;
    if (radiator_map_ != nullptr && owns_radiator_map_)
    {
      InternalDeleteRadiatorMap(radiator_map_, &error_code);
    }
    radiator_map_ = nullptr;
    owns_radiator_map_ = false;
  }

  void RadiatorMap::AddRadiator(Radiator *radiator, Error *error)
  {
    if (radiator_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_RADIATOR_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_RADIATOR_MAP)) };
      return;
    }
    if (radiator->radiator_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_RADIATOR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_RADIATOR)) };
      return;
    }
    if (radiator->updater_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_RADIATOR_UPDATER,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_RADIATOR_UPDATER)) };
      return;
    }

    int error_code = ERROR_NONE;

    try
    {
      InternalAddRadiator(radiator_map_, radiator->radiator_, &error_code);
      if (error_code != ERROR_NONE)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        return;
      }
      InternalDeleteRadiatorUpdater(radiator->updater_, &error_code);
      if (error_code != ERROR_NONE)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        return;
      }
      radiator->updater_ = InternalGetRadiatorUpdaterFromMap(radiator_map_, radiator->radiator_, &error_code);
      if (error_code != ERROR_NONE)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        return;
      }
      InternalDeleteRadiator(radiator->radiator_, &error_code);
      if (error_code != ERROR_NONE)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        return;
      }
      radiator->radiator_ = nullptr;
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
      return;
    }
    catch (...)
    {
      *error = Error{ INTERNAL_RADIATOR_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_RADIATOR_MAP_ERROR)) };
      return;
    }
    *error = NoError();
  }

  Radiator *RadiatorMap::GetRadiator(const char *radiator_name, Error *error)
  {
    DeleteError(error);
    if (radiator_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_RADIATOR_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_RADIATOR_MAP)) };
      return nullptr;
    }

    Radiator *radiator = nullptr;

    try
    {
      int error_code = ERROR_NONE;
      void *radiator_ptr = InternalGetRadiator(radiator_map_, radiator_name, strlen(radiator_name), &error_code);
      if (error_code != ERROR_NONE)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        return nullptr;
      }
      void *updater_ptr = InternalGetRadiatorUpdaterFromMap(radiator_map_, radiator_ptr, &error_code);
      if (error_code != ERROR_NONE)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        InternalDeleteRadiator(radiator_ptr, &error_code);
        return nullptr;
      }
      InternalDeleteRadiator(radiator_ptr, &error_code);
      if (error_code != ERROR_NONE)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        InternalDeleteRadiatorUpdater(updater_ptr, &error_code);
        return nullptr;
      }
      radiator = new Radiator(updater_ptr);
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
      return nullptr;
    }
    catch (...)
    {
      *error = Error{ INTERNAL_RADIATOR_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_RADIATOR_MAP_ERROR)) };
      return nullptr;
    }
    *error = NoError();
    return radiator;
  }

  Radiator *RadiatorMap::GetRadiatorByIndex(std::size_t index, Error *error)
  {
    DeleteError(error);
    if (radiator_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_RADIATOR_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_RADIATOR_MAP)) };
      return nullptr;
    }

    Radiator *radiator = nullptr;

    try
    {
      int error_code = ERROR_NONE;
      void *radiator_ptr = InternalGetRadiatorByIndex(radiator_map_, index, &error_code);
      if (error_code != ERROR_NONE)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        return nullptr;
      }
      void *updater_ptr = InternalGetRadiatorUpdaterFromMap(radiator_map_, radiator_ptr, &error_code);
      if (error_code != ERROR_NONE)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        InternalDeleteRadiator(radiator_ptr, &error_code);
        return nullptr;
      }
      InternalDeleteRadiator(radiator_ptr, &error_code);
      if (error_code != ERROR_NONE)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        InternalDeleteRadiatorUpdater(updater_ptr, &error_code);
        return nullptr;
      }
      radiator = new Radiator(updater_ptr);
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
      return nullptr;
    }
    catch (...)
    {
      *error = Error{ INTERNAL_RADIATOR_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_RADIATOR_MAP_ERROR)) };
      return nullptr;
    }
    *error = NoError();
    return radiator;
  }

  void RadiatorMap::RemoveRadiator(const char *radiator_name, Error *error)
  {
    DeleteError(error);
    if (radiator_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_RADIATOR_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_RADIATOR_MAP)) };
      return;
    }

    int error_code = ERROR_NONE;

    try
    {
      InternalRemoveRadiator(radiator_map_, radiator_name, strlen(radiator_name), &error_code);
      if (error_code != ERROR_NONE)
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
      *error = Error{ INTERNAL_RADIATOR_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_RADIATOR_MAP_ERROR)) };
      return;
    }
    *error = NoError();
  }

  void RadiatorMap::RemoveRadiatorByIndex(std::size_t index, Error *error)
  {
    DeleteError(error);
    if (radiator_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_RADIATOR_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_RADIATOR_MAP)) };
      return;
    }

    int error_code = ERROR_NONE;

    try
    {
      InternalRemoveRadiatorByIndex(radiator_map_, index, &error_code);
      if (error_code != ERROR_NONE)
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
      *error = Error{ INTERNAL_RADIATOR_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_RADIATOR_MAP_ERROR)) };
      return;
    }
    *error = NoError();
  }

  std::size_t RadiatorMap::GetNumberOfRadiators(Error *error)
  {
    DeleteError(error);
    if (radiator_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_RADIATOR_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_RADIATOR_MAP)) };
      return 0;
    }

    std::size_t num_radiators = 0;

    try
    {
      int error_code = ERROR_NONE;
      num_radiators = InternalGetNumberOfRadiators(radiator_map_, &error_code);
      if (error_code != ERROR_NONE)
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
      *error = Error{ INTERNAL_RADIATOR_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_RADIATOR_MAP_ERROR)) };
      return 0;
    }
    *error = NoError();
    return num_radiators;
  }

}  // namespace musica