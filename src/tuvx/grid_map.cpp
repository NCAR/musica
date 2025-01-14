// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/tuvx/grid_map.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

namespace musica
{

  // GridMap external C API functions

  GridMap *CreateGridMap(Error *error)
  {
    DeleteError(error);
    return new GridMap(error);
  }

  void DeleteGridMap(GridMap *grid_map, Error *error)
  {
    DeleteError(error);
    try
    {
      delete grid_map;
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
    *error = NoError();
  }

  void AddGrid(GridMap *grid_map, Grid *grid, Error *error)
  {
    DeleteError(error);
    grid_map->AddGrid(grid, error);
  }

  Grid *GetGrid(GridMap *grid_map, const char *grid_name, const char *grid_units, Error *error)
  {
    DeleteError(error);
    return grid_map->GetGrid(grid_name, grid_units, error);
  }

  // GridMap class functions

  GridMap::GridMap(Error *error)
  {
    int error_code = 0;
    grid_map_ = InternalCreateGridMap(&error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create grid map") };
    }
    owns_grid_map_ = true;
    *error = NoError();
  }

  GridMap::~GridMap()
  {
    int error_code = 0;
    if (grid_map_ != nullptr && owns_grid_map_)
    {
      InternalDeleteGridMap(grid_map_, &error_code);
    }
    grid_map_ = nullptr;
    owns_grid_map_ = false;
  }

  void GridMap::AddGrid(Grid *grid, Error *error)
  {
    if (grid_map_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Grid map is null") };
      return;
    }
    if (grid->grid_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Cannot add unowned grid to grid map") };
      return;
    }
    if (grid->updater_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Cannot add grid in invalid state") };
      return;
    }

    int error_code = 0;

    try
    {
      InternalAddGrid(grid_map_, grid->grid_, &error_code);
      if (error_code != 0)
      {
        *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to add grid to grid map") };
      }
      InternalDeleteGridUpdater(grid->updater_, &error_code);
      if (error_code != 0)
      {
        *error = Error{ 1,
                        CreateString(MUSICA_ERROR_CATEGORY),
                        CreateString("Failed to delete updater after transfer of ownership to grid map") };
      }
      grid->updater_ = InternalGetGridUpdaterFromMap(grid_map_, grid->grid_, &error_code);
      if (error_code != 0)
      {
        *error = Error{ 1,
                        CreateString(MUSICA_ERROR_CATEGORY),
                        CreateString("Failed to get updater after transfer of ownership to grid map") };
      }
      InternalDeleteGrid(grid->grid_, &error_code);
      if (error_code != 0)
      {
        *error = Error{ 1,
                        CreateString(MUSICA_ERROR_CATEGORY),
                        CreateString("Failed to delete grid during transfer of ownership to grid map") };
      }
      grid->grid_ = nullptr;
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
    catch (...)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Internal error adding grid") };
    }
    *error = NoError();
  }

  Grid *GridMap::GetGrid(const char *grid_name, const char *grid_units, Error *error)
  {
    if (grid_map_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Grid map is null") };
      return nullptr;
    }

    Grid *grid = nullptr;

    try
    {
      int error_code = 0;
      void *grid_ptr = InternalGetGrid(grid_map_, grid_name, strlen(grid_name), grid_units, strlen(grid_units), &error_code);
      if (error_code != 0)
      {
        *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to get grid from grid map") };
        return nullptr;
      }
      void *updater_ptr = InternalGetGridUpdaterFromMap(grid_map_, grid_ptr, &error_code);
      if (error_code != 0)
      {
        *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to get updater") };
        InternalDeleteGrid(grid_ptr, &error_code);
        return nullptr;
      }
      InternalDeleteGrid(grid_ptr, &error_code);
      if (error_code != 0)
      {
        *error =
            Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to delete grid after getting updater") };
        InternalDeleteGridUpdater(updater_ptr, &error_code);
        return nullptr;
      }
      grid = new Grid(updater_ptr);
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
    catch (...)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Internal error getting grid") };
    }
    *error = NoError();
    return grid;
  }

}  // namespace musica
