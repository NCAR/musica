// Copyright (C) 2023-2025 National Center for Atmospheric Research
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
      case ERROR_UNALLOCATED_GRID_UPDATER: return "Unallocated grid updater";
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
    catch (const std::system_error &e)
    {
      *error = ToError(e);
      return;
    }
    *error = NoError();
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

  // Grid class functions

  Grid::Grid(const char *grid_name, const char *units, std::size_t num_sections, Error *error)
  {
    int error_code = 0;
    grid_ = InternalCreateGrid(grid_name, strlen(grid_name), units, strlen(units), num_sections, &error_code);
    if (error_code != 0)
    {
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      return;
    }
    updater_ = InternalGetGridUpdater(grid_, &error_code);
    if (error_code != 0)
    {
      InternalDeleteGrid(grid_, &error_code);
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
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
      InternalDeleteGridUpdater(updater_, &error_code);
    grid_ = nullptr;
    updater_ = nullptr;
  }

  std::string Grid::GetName(Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_GRID_UPDATER;
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      return "";
    }
    String name = InternalGetGridName(updater_, &error_code);
    if (error_code != 0)
    {
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      return "";
    }
    *error = NoError();
    std::string value(name.value_, name.size_);
    DeleteString(&name);
    return value;
  }

  std::string Grid::GetUnits(Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_GRID_UPDATER;
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      return "";
    }
    String units = InternalGetGridUnits(updater_, &error_code);
    if (error_code != 0)
    {
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      return "";
    }
    *error = NoError();
    std::string value(units.value_, units.size_);
    DeleteString(&units);
    return value;
  }

  std::size_t Grid::GetNumberOfSections(Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_GRID_UPDATER;
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      return 0;
    }
    std::size_t n_sections = InternalGetNumberOfSections(updater_, &error_code);
    if (error_code != 0)
    {
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      return 0;
    }
    *error = NoError();
    return n_sections;
  }

  void Grid::SetEdges(double edges[], std::size_t num_edges, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_GRID_UPDATER;
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      return;
    }
    InternalSetEdges(updater_, edges, num_edges, &error_code);
    if (error_code != 0)
    {
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      return;
    }
    *error = NoError();
  }

  void Grid::GetEdges(double edges[], std::size_t num_edges, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_GRID_UPDATER;
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      return;
    }
    InternalGetEdges(updater_, edges, num_edges, &error_code);
    if (error_code != 0)
    {
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      return;
    }
    *error = NoError();
  }

  void Grid::SetMidpoints(double midpoints[], std::size_t num_midpoints, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_GRID_UPDATER;
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      return;
    }
    InternalSetMidpoints(updater_, midpoints, num_midpoints, &error_code);
    if (error_code != 0)
    {
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      return;
    }
    *error = NoError();
  }

  void Grid::GetMidpoints(double midpoints[], std::size_t num_midpoints, Error *error)
  {
    DeleteError(error);
    int error_code = 0;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_GRID_UPDATER;
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      return;
    }
    InternalGetMidpoints(updater_, midpoints, num_midpoints, &error_code);
    if (error_code != 0)
    {
      *error = Error{ error_code, CreateString(MUSICA_ERROR_CATEGORY), CreateString(GetErrorMessage(error_code)) };
      return;
    }
    *error = NoError();
  }

}  // namespace musica
