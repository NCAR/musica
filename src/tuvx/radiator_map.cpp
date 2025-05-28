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
      Error* temp = ToError(e);
      *error = *temp;
      DeleteError(temp);
    }
    Error* temp = NoError();
    *error = *temp;
    DeleteError(temp);
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
      SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to create radiator map");
    }
    owns_radiator_map_ = true;
    Error* temp = NoError();
    *error = *temp;
    DeleteError(temp);
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
      SetError(error, 1, MUSICA_ERROR_CATEGORY, "Radiator map is null");
      return;
    }
    if (radiator->radiator_ == nullptr)
    {
      SetError(error, 1, MUSICA_ERROR_CATEGORY, "Cannot add unowned radiator to radiator map");
      return;
    }
    if (radiator->updater_ == nullptr)
    {
      SetError(error, 1, MUSICA_ERROR_CATEGORY, "Cannot add radiator in invalid state");
      return;
    }

    int error_code = 0;

    try
    {
      InternalAddRadiator(radiator_map_, radiator->radiator_, &error_code);
      if (error_code != 0)
      {
        SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to add radiator to radiator map");
      }
      InternalDeleteRadiatorUpdater(radiator->updater_, &error_code);
      if (error_code != 0)
      {
        SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to delete updater after transfer of ownership to radiator map");
      }
      radiator->updater_ = InternalGetRadiatorUpdaterFromMap(radiator_map_, radiator->radiator_, &error_code);
      if (error_code != 0)
      {
        SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to get updater after transfer of ownership to radiator map");
      }
      InternalDeleteRadiator(radiator->radiator_, &error_code);
      if (error_code != 0)
      {
        SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to delete radiator during transfer of ownership to radiator map");
      }
      radiator->radiator_ = nullptr;
    }
    catch (const std::system_error &e)
    {
      Error* temp = ToError(e);
      *error = *temp;
      DeleteError(temp);
    }
    catch (...)
    {
      SetError(error, 1, MUSICA_ERROR_CATEGORY, "Internal error adding radiator");
    }
    Error* temp = NoError();
    *error = *temp;
    DeleteError(temp);
  }

  Radiator *RadiatorMap::GetRadiator(const char *radiator_name, Error *error)
  {
    if (radiator_map_ == nullptr)
    {
      SetError(error, 1, MUSICA_ERROR_CATEGORY, "Radiator map is null");
      return nullptr;
    }

    Radiator *radiator = nullptr;

    try
    {
      int error_code = 0;
      void *radiator_ptr = InternalGetRadiator(radiator_map_, radiator_name, strlen(radiator_name), &error_code);
      if (error_code != 0)
      {
        SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to get radiator from radiator map");
        return nullptr;
      }
      void *updater_ptr = InternalGetRadiatorUpdaterFromMap(radiator_map_, radiator_ptr, &error_code);
      if (error_code != 0)
      {
        SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to get radiator updater");
        InternalDeleteRadiator(radiator_ptr, &error_code);        
        return nullptr;
      }
      InternalDeleteRadiator(radiator_ptr, &error_code);
      if (error_code != 0)
      {
        SetError(error, 1, MUSICA_ERROR_CATEGORY, "Failed to delete radiator after getting updater");
        InternalDeleteRadiatorUpdater(updater_ptr, &error_code);
        return nullptr;
      }
      radiator = new Radiator(updater_ptr);
    }
    catch (const std::system_error &e)
    {
      Error* temp = ToError(e);
      *error = *temp;
      DeleteError(temp);
    }
    catch (...)
    {
      SetError(error, 1, MUSICA_ERROR_CATEGORY, "Internal error getting radiator");
    }
    Error* temp = NoError();
    *error = *temp;
    DeleteError(temp);
    return radiator;
  }

}  // namespace musica