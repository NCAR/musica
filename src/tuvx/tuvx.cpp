// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the TUVX class, which represents a multi-component
// reactive transport model. It also includes functions for creating and deleting TUVX instances.
#include <musica/tuvx.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

namespace musica
{

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

  void DeleteGrid(Grid *grid, Error *error)
  {
    *error = NoError();
    try
    {
      delete grid;
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
  }

  void SetEdges(Grid *grid, double edges[], std::size_t num_edges, Error *error)
  {
    DeleteError(error);
    grid->SetEdges(edges, num_edges, error);
  }

  void SetMidpoints(Grid *grid, double midpoints[], std::size_t num_midpoints, Error *error)
  {
    DeleteError(error);
    grid->SetMidpoints(midpoints, num_midpoints, error);
  }

  TUVX::TUVX()
      : tuvx_(),
        grid_map_(nullptr)
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
    int error_code = 0;
    grid_map_ = std::make_unique<GridMap>(InternalGetGridMap(tuvx_, &error_code));
    *error = NoError();
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create grid map") };
      return nullptr;
    }
    return grid_map_.get();
  }

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

  Grid::~Grid()
  {
    int error_code = 0;
    if (grid_ != nullptr)
      InternalDeleteGrid(grid_, &error_code);
    grid_ = nullptr;
  }

  void Grid::SetEdges(double edges[], std::size_t num_edges, Error *error)
  {
    int error_code = 0;
    InternalSetEdges(grid_, edges, num_edges, &error_code);
    *error = NoError();
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set edges") };
    }
  }

  void Grid::SetMidpoints(double midpoints[], std::size_t num_midpoints, Error *error)
  {
    int error_code = 0;
    InternalSetMidpoints(grid_, midpoints, num_midpoints, &error_code);
    *error = NoError();
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set midpoints") };
    }
  }

}  // namespace musica
