// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the TUVX class, which represents a multi-component
// reactive transport model. It also includes functions for creating and deleting TUVX instances.
#include <musica/tuvx/grid_map.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

namespace musica
{

  // GridMap external C API functions

  void DeleteGridMap(GridMap *grid_map, Error *error)
  {
    *error = NoError();
    try
    {
      delete grid_map;
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
  }

  Grid *GetGrid(GridMap *grid_map, const char *grid_name, const char *grid_units, Error *error)
  {
    DeleteError(error);
    return grid_map->GetGrid(grid_name, grid_units, error);
  }

  // GridMap class functions

  GridMap::~GridMap()
  {
    // At the time of writing, the grid map pointer is owned by fortran memory
    // in the tuvx core and should not be deleted here. It will be deleted when
    // the tuvx instance is deleted
    int error_code = 0;
    grid_map_ = nullptr;
  }

  Grid *GridMap::GetGrid(const char *grid_name, const char *grid_units, Error *error)
  {
    if (grid_map_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Grid map is null") };
      return nullptr;
    }

    int error_code = 0;
    Grid *grid = nullptr;

    try
    {
      *error = NoError();

      grid = new Grid(InternalGetGrid(grid_map_, grid_name, strlen(grid_name), grid_units, strlen(grid_units), &error_code));

      if (error_code != 0)
      {
        delete grid;
        grid = nullptr;
        DeleteError(error);
        *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create grid map") };
      }
      else
      {
        grids_.push_back(std::unique_ptr<Grid>(grid));
      }
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
    catch (...)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create grid") };
    }

    return grid;
  }

}  // namespace musica
