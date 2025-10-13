// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/tuvx/grid_map.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

namespace
{
  constexpr int ERROR_NONE = 0;
  constexpr int ERROR_UNALLOCATED_GRID_MAP = 100;
  constexpr int ERROR_UNALLOCATED_GRID = 101;
  constexpr int ERROR_UNALLOCATED_GRID_UPDATER = 102;
  constexpr int ERROR_GRID_NAME_NOT_FOUND = 103;
  constexpr int ERROR_GRID_UNIT_MISMATCH = 104;
  constexpr int ERROR_GRID_TYPE_MISMATCH = 105;
  constexpr int ERROR_INDEX_OUT_OF_BOUNDS = 106;
  constexpr int INTERNAL_GRID_MAP_ERROR = 199;
  constexpr const char *GetErrorMessage(int code)
  {
    switch (code)
    {
      case ERROR_NONE: return "Success";
      case ERROR_UNALLOCATED_GRID_MAP: return "Unallocated grid map";
      case ERROR_UNALLOCATED_GRID: return "Unallocated grid";
      case ERROR_GRID_NAME_NOT_FOUND: return "Grid name not found";
      case ERROR_GRID_UNIT_MISMATCH: return "Grid unit mismatch";
      case ERROR_GRID_TYPE_MISMATCH: return "Grid type is not supported";
      case ERROR_INDEX_OUT_OF_BOUNDS: return "Index out of range";
      case INTERNAL_GRID_MAP_ERROR: return "Unknown internal error";
      default: return "Unknown error";
    }
  }
}  // namespace

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

  Grid *GetGridByIndex(GridMap *grid_map, std::size_t index, Error *error)
  {
    DeleteError(error);
    return grid_map->GetGridByIndex(index, error);
  }

  std::size_t GetNumberOfGrids(GridMap *grid_map, Error *error)
  {
    DeleteError(error);
    return grid_map->GetNumberOfGrids(error);
  }

  void RemoveGrid(GridMap *grid_map, const char *grid_name, const char *grid_units, Error *error)
  {
    DeleteError(error);
    grid_map->RemoveGrid(grid_name, grid_units, error);
  }

  void RemoveGridByIndex(GridMap *grid_map, std::size_t index, Error *error)
  {
    DeleteError(error);
    grid_map->RemoveGridByIndex(index, error);
  }

  // GridMap class functions

  GridMap::GridMap(Error *error)
  {
    int error_code = 0;
    grid_map_ = InternalCreateGridMap(&error_code);
    if (error_code != 0)
    {
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
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
      *error = Error{ ERROR_UNALLOCATED_GRID_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_GRID_MAP)) };
      return;
    }
    if (grid->grid_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_GRID,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_GRID)) };
      return;
    }
    if (grid->updater_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_GRID_UPDATER,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_GRID_UPDATER)) };
      return;
    }

    int error_code = 0;

    try
    {
      InternalAddGrid(grid_map_, grid->grid_, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        return;
      }
      InternalDeleteGridUpdater(grid->updater_, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        return;
      }
      grid->updater_ = InternalGetGridUpdaterFromMap(grid_map_, grid->grid_, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        return;
      }
      InternalDeleteGrid(grid->grid_, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        InternalDeleteGridUpdater(grid->updater_, &error_code);
        grid->updater_ = nullptr;
        return;
      }
      grid->grid_ = nullptr;
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
      return;
    }
    catch (...)
    {
      *error = Error{ INTERNAL_GRID_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_GRID_MAP_ERROR)) };
      return;
    }
    *error = NoError();
  }

  Grid *GridMap::GetGrid(const char *grid_name, const char *grid_units, Error *error)
  {
    if (grid_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_GRID_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_GRID_MAP)) };
      return nullptr;
    }

    Grid *grid = nullptr;

    try
    {
      int error_code = 0;
      void *grid_ptr = InternalGetGrid(grid_map_, grid_name, strlen(grid_name), grid_units, strlen(grid_units), &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        return nullptr;
      }
      void *updater_ptr = InternalGetGridUpdaterFromMap(grid_map_, grid_ptr, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        InternalDeleteGrid(grid_ptr, &error_code);
        return nullptr;
      }
      InternalDeleteGrid(grid_ptr, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        InternalDeleteGridUpdater(updater_ptr, &error_code);
        return nullptr;
      }
      grid = new Grid(updater_ptr);
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
      return nullptr;
    }
    catch (...)
    {
      *error = Error{ INTERNAL_GRID_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_GRID_MAP_ERROR)) };
      return nullptr;
    }
    *error = NoError();
    return grid;
  }

  Grid *GridMap::GetGridByIndex(std::size_t index, Error *error)
  {
    if (grid_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_GRID_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_GRID_MAP)) };
      return nullptr;
    }

    Grid *grid = nullptr;

    try
    {
      int error_code = 0;
      void *grid_ptr = InternalGetGridByIndex(grid_map_, index, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        return nullptr;
      }
      void *updater_ptr = InternalGetGridUpdaterFromMap(grid_map_, grid_ptr, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        InternalDeleteGrid(grid_ptr, &error_code);
        return nullptr;
      }
      InternalDeleteGrid(grid_ptr, &error_code);
      if (error_code != 0)
      {
        *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
        InternalDeleteGridUpdater(updater_ptr, &error_code);
        return nullptr;
      }
      grid = new Grid(updater_ptr);
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
      return nullptr;
    }
    catch (...)
    {
      *error = Error{ INTERNAL_GRID_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_GRID_MAP_ERROR)) };
      return nullptr;
    }
    *error = NoError();
    return grid;
  }

  std::size_t GridMap::GetNumberOfGrids(Error *error)
  {
    if (grid_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_GRID_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_GRID_MAP)) };
      return 0;
    }

    std::size_t n_grids = 0;

    try
    {
      int error_code = 0;
      n_grids = InternalGetNumberOfGrids(grid_map_, &error_code);
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
      *error = Error{ INTERNAL_GRID_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_GRID_MAP_ERROR)) };
      return 0;
    }
    *error = NoError();
    return n_grids;
  }

  void GridMap::RemoveGrid(const char *grid_name, const char *grid_units, Error *error)
  {
    if (grid_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_GRID_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_GRID_MAP)) };
      return;
    }

    try
    {
      int error_code = 0;
      InternalRemoveGrid(grid_map_, grid_name, strlen(grid_name), grid_units, strlen(grid_units), &error_code);
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
      *error = Error{ INTERNAL_GRID_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_GRID_MAP_ERROR)) };
      return;
    }
    *error = NoError();
  }

  void GridMap::RemoveGridByIndex(std::size_t index, Error *error)
  {
    if (grid_map_ == nullptr)
    {
      *error = Error{ ERROR_UNALLOCATED_GRID_MAP,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(ERROR_UNALLOCATED_GRID_MAP)) };
      return;
    }

    try
    {
      int error_code = 0;
      InternalRemoveGridByIndex(grid_map_, index, &error_code);
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
      *error = Error{ INTERNAL_GRID_MAP_ERROR,
                      CreateString(MUSICA_ERROR_CATEGORY),
                      CreateString(GetErrorMessage(INTERNAL_GRID_MAP_ERROR)) };
      return;
    }
    *error = NoError();
  }

}  // namespace musica
