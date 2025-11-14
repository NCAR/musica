// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/tuvx/radiator_map.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

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
      ToError(e, error);
      return;
    }
    NoError(error);
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

  // RadiatordMap class functions

  RadiatorMap::RadiatorMap(Error *error)
  {
    int error_code = 0;
    radiator_map_ = InternalCreateRadiatorMap(&error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to create radiator map", error);
    }
    owns_radiator_map_ = true;
    NoError(error);
  }

  RadiatorMap::~RadiatorMap()
  {
    int error_code = 0;
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
      ToError(MUSICA_ERROR_CATEGORY, 1, "Radiator map is null", error);
      return;
    }
    if (radiator->radiator_ == nullptr)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Cannot add unallocated radiator to radiator map", error);
      return;
    }
    if (radiator->updater_ == nullptr)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Cannot add radiator in invalid state", error);
      return;
    }

    int error_code = 0;

    try
    {
      InternalAddRadiator(radiator_map_, radiator->radiator_, &error_code);
      if (error_code != 0)
      {
        ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to add radiator to radiator map", error);
        return;
      }
      InternalDeleteRadiatorUpdater(radiator->updater_, &error_code);
      if (error_code != 0)
      {
        ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to delete updater after transfer of ownership to radiator map", error);
        return;
      }
      radiator->updater_ = InternalGetRadiatorUpdaterFromMap(radiator_map_, radiator->radiator_, &error_code);
      if (error_code != 0)
      {
        ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to get updater after transfer of ownership to radiator map", error);
        return;
      }
      InternalDeleteRadiator(radiator->radiator_, &error_code);
      if (error_code != 0)
      {
        ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to delete radiator during transfer of ownership to radiator map", error);
        return;
      }
      radiator->radiator_ = nullptr;
    }
    catch (const std::system_error &e)
    {
      ToError(e, error);
    }
    catch (...)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Internal error adding radiator", error);
    }
    NoError(error);
  }

  Radiator *RadiatorMap::GetRadiator(const char *radiator_name, Error *error)
  {
    if (radiator_map_ == nullptr)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Radiator map is null", error);
      return nullptr;
    }

    Radiator *radiator = nullptr;

    try
    {
      int error_code = 0;
      void *radiator_ptr = InternalGetRadiator(radiator_map_, radiator_name, strlen(radiator_name), &error_code);
      if (error_code != 0)
      {
        ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to get radiator from radiator map", error);
        return nullptr;
      }
      void *updater_ptr = InternalGetRadiatorUpdaterFromMap(radiator_map_, radiator_ptr, &error_code);
      if (error_code != 0)
      {
        ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to get radiator updater", error);
        InternalDeleteRadiator(radiator_ptr, &error_code);
        return nullptr;
      }
      InternalDeleteRadiator(radiator_ptr, &error_code);
      if (error_code != 0)
      {
        ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to delete radiator after getting updater", error);
        InternalDeleteRadiatorUpdater(updater_ptr, &error_code);
        return nullptr;
      }
      radiator = new Radiator(updater_ptr);
    }
    catch (const std::system_error &e)
    {
      ToError(e, error);
    }
    catch (...)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Internal error getting radiator", error);
    }
    NoError(error);
    return radiator;
  }

}  // namespace musica