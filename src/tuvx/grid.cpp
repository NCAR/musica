// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the TUVX class, which represents a multi-component
// reactive transport model. It also includes functions for creating and deleting TUVX instances.
#include <musica/tuvx/grid.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

namespace musica
{

  // Grid external C API functions

  Grid *CreateGrid(const char* grid_name, const char* units, std::size_t num_sections, Error *error)
  {
    DeleteError(error);
    return new Grid(grid_name, units, num_sections, error);
  }

  void DeleteGrid(Grid *grid, Error *error)
  {
    DeleteError(error);
    try
    {
      delete grid;
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
      return;
    }
    *error = NoError();
  }

  void SetGridEdges(Grid *grid, double edges[], std::size_t num_edges, Error *error)
  {
    DeleteError(error);
    grid->SetEdges(edges, num_edges, error);
  }

  void GetGridEdges(Grid *grid, double edges[], std::size_t num_edges, Error *error)
  {
    DeleteError(error);
    grid->GetEdges(edges, num_edges, error);
  }

  void SetGridEdgesAndMidpoints(Grid *grid, double edges[], std::size_t num_edges, double midpoints[], std::size_t num_midpoints, Error *error)
  {
    DeleteError(error);
    grid->SetEdgesAndMidpoints(edges, num_edges, midpoints, num_midpoints, error);
  }

  void GetGridMidpoints(Grid *grid, double midpoints[], std::size_t num_midpoints, Error *error)
  {
    DeleteError(error);
    grid->GetMidpoints(midpoints, num_midpoints, error);
  }

  // Grid class functions

  Grid::Grid(const char *grid_name, const char *units, std::size_t num_sections, Error *error)
  {
    int error_code = 0;
    grid_ = InternalCreateGrid(grid_name, strlen(grid_name), units, strlen(units), num_sections, &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create grid") };
      return;
    }
    updater_ = InternalGetUpdater(grid_, &error_code);
    if (error_code != 0)
    {
      InternalDeleteGrid(grid_, &error_code);
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to get updater") };
      return;
    }
    *error = NoError();
  }

  Grid::~Grid()
  {
    int error_code = 0;
    if (grid_ != nullptr)
      InternalDeleteGrid(grid_, &error_code);
    if (updater_ != nullptr)
      InternalDeleteUpdater(updater_, &error_code);
    grid_ = nullptr;
  }

  void Grid::SetEdges(double edges[], std::size_t num_edges, Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Grid is not updatable") };
      return;
    }
    InternalSetEdges(updater_, edges, num_edges, &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set edges") };
      return;
    }
    *error = NoError();
  }

  void Grid::GetEdges(double edges[], std::size_t num_edges, Error *error)
  {
    int error_code = 0;
    InternalGetEdges(grid_, edges, num_edges, &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to get edges") };
      return;
    }
    *error = NoError();
  }

  void Grid::SetEdgesAndMidpoints(double edges[], std::size_t num_edges, double midpoints[], std::size_t num_midpoints, Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Grid is not updatable") };
      return;
    }
    InternalSetEdgesAndMidpoints(updater_, edges, num_edges, midpoints, num_midpoints, &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set midpoints") };
      return;
    }
    *error = NoError();
  }

  void Grid::GetMidpoints(double midpoints[], std::size_t num_midpoints, Error *error)
  {
    int error_code = 0;
    InternalGetMidpoints(grid_, midpoints, num_midpoints, &error_code);
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to get midpoints") };
      return;
    }
    *error = NoError();
  }

}  // namespace musica
