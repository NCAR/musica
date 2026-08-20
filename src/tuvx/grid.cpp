// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/tuvx/grid.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

namespace
{
  constexpr int ERROR_NONE = 0;
  constexpr int ERROR_UNALLOCATED_GRID_UPDATER = 1;
  constexpr int ERROR_GRID_SIZE_MISMATCH = 2;
  constexpr const char *GetErrorMessage(int code)
  {
    switch (code)
    {
      case ERROR_NONE: return "Success";
      case ERROR_UNALLOCATED_GRID_UPDATER:
        return "No grid updater available (e.g. because the grid was not supplied by the host)";
      case ERROR_GRID_SIZE_MISMATCH: return "Grid size mismatch";
      default: return "Unknown error";
    }
  }
}  // namespace

namespace musica
{

  // Grid external C API functions

  Grid *CreateGrid(const char *grid_name, const char *units, std::size_t num_sections, Error *error)
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
    catch (const std::exception &e)
    {
      ToError(e, MUSICA_SEVERITY_ERROR, error);
      return;
    }
    NoError(error);
  }

  std::size_t GetGridNumberOfSections(Grid *grid, Error *error)
  {
    DeleteError(error);
    return grid->GetNumberOfSections(error);
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

  double *GetGridEdgesPointer(Grid *grid, Error *error)
  {
    DeleteError(error);
    return grid->GetEdgesPointer(error);
  }

  void SetGridMidpoints(Grid *grid, double midpoints[], std::size_t num_midpoints, Error *error)
  {
    DeleteError(error);
    grid->SetMidpoints(midpoints, num_midpoints, error);
  }

  void GetGridMidpoints(Grid *grid, double midpoints[], std::size_t num_midpoints, Error *error)
  {
    DeleteError(error);
    grid->GetMidpoints(midpoints, num_midpoints, error);
  }

  double *GetGridMidpointsPointer(Grid *grid, Error *error)
  {
    DeleteError(error);
    return grid->GetMidpointsPointer(error);
  }

  // Grid class functions

  Grid::Grid(const char *grid_name, const char *units, std::size_t num_sections, Error *error)
  {
    int error_code = 0;
    grid_ = InternalCreateGrid(grid_name, strlen(grid_name), units, strlen(units), num_sections, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return;
    }
    updater_ = InternalGetGridUpdater(grid_, &error_code);
    if (error_code != 0)
    {
      InternalDeleteGrid(grid_, &error_code);
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return;
    }
    NoError(error);
  }

  Grid::~Grid()
  {
    int error_code = 0;
    if (grid_ != nullptr)
      InternalDeleteGrid(grid_, &error_code);
    if (updater_ != nullptr)
      InternalDeleteGridUpdater(updater_, &error_code);
    grid_ = nullptr;
    updater_ = nullptr;
  }

  std::string Grid::GetName(Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    String name;
    if (updater_ != nullptr)
      InternalGetGridName(updater_, &name, &error_code);
    else if (grid_ != nullptr)
      InternalGetGridNameReadOnly(grid_, &name, &error_code);
    else
    {
      error_code = ERROR_UNALLOCATED_GRID_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return "";
    }
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return "";
    }
    NoError(error);
    std::string value(name.value_, name.size_);
    DeleteString(&name);
    return value;
  }

  std::string Grid::GetUnits(Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    String units;
    if (updater_ != nullptr)
      InternalGetGridUnits(updater_, &units, &error_code);
    else if (grid_ != nullptr)
      InternalGetGridUnitsReadOnly(grid_, &units, &error_code);
    else
    {
      error_code = ERROR_UNALLOCATED_GRID_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return "";
    }
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return "";
    }
    NoError(error);
    std::string value(units.value_, units.size_);
    DeleteString(&units);
    return value;
  }

  std::size_t Grid::GetNumberOfSections(Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    std::size_t n_sections = 0;
    if (updater_ != nullptr)
      n_sections = InternalGetNumberOfSections(updater_, &error_code);
    else if (grid_ != nullptr)
      n_sections = InternalGetNumberOfSectionsReadOnly(grid_, &error_code);
    else
    {
      error_code = ERROR_UNALLOCATED_GRID_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return 0;
    }
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return 0;
    }
    NoError(error);
    return n_sections;
  }

  void Grid::SetEdges(double edges[], std::size_t num_edges, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_GRID_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return;
    }
    InternalSetEdges(updater_, edges, num_edges, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return;
    }
    NoError(error);
  }

  void Grid::GetEdges(double edges[], std::size_t num_edges, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ != nullptr)
      InternalGetEdges(updater_, edges, num_edges, &error_code);
    else if (grid_ != nullptr)
      InternalGetEdgesReadOnly(grid_, edges, num_edges, &error_code);
    else
    {
      error_code = ERROR_UNALLOCATED_GRID_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return;
    }
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return;
    }
    NoError(error);
  }

  double *Grid::GetEdgesPointer(Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    double *edges_ptr = nullptr;
    if (updater_ != nullptr)
      edges_ptr = InternalGetEdgesPointer(updater_, &error_code);
    else if (grid_ != nullptr)
      edges_ptr = InternalGetEdgesPointerReadOnly(grid_, &error_code);
    else
    {
      error_code = ERROR_UNALLOCATED_GRID_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return nullptr;
    }
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return nullptr;
    }
    NoError(error);
    return edges_ptr;
  }

  void Grid::SetMidpoints(double midpoints[], std::size_t num_midpoints, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_GRID_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return;
    }
    InternalSetMidpoints(updater_, midpoints, num_midpoints, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return;
    }
    NoError(error);
  }

  void Grid::GetMidpoints(double midpoints[], std::size_t num_midpoints, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ != nullptr)
      InternalGetMidpoints(updater_, midpoints, num_midpoints, &error_code);
    else if (grid_ != nullptr)
      InternalGetMidpointsReadOnly(grid_, midpoints, num_midpoints, &error_code);
    else
    {
      error_code = ERROR_UNALLOCATED_GRID_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return;
    }
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return;
    }
    NoError(error);
  }

  double *Grid::GetMidpointsPointer(Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    double *midpoints_ptr = nullptr;
    if (updater_ != nullptr)
      midpoints_ptr = InternalGetMidpointsPointer(updater_, &error_code);
    else if (grid_ != nullptr)
      midpoints_ptr = InternalGetMidpointsPointerReadOnly(grid_, &error_code);
    else
    {
      error_code = ERROR_UNALLOCATED_GRID_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return nullptr;
    }
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return nullptr;
    }
    NoError(error);
    return midpoints_ptr;
  }

}  // namespace musica
