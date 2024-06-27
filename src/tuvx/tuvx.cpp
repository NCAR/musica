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

  // Grid functions

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

  void SetGridEdges(Grid *grid, double edges[], std::size_t num_edges, Error *error)
  {
    DeleteError(error);
    grid->SetEdges(edges, num_edges, error);
  }

  void SetGridMidpoints(Grid *grid, double midpoints[], std::size_t num_midpoints, Error *error)
  {
    DeleteError(error);
    grid->SetMidpoints(midpoints, num_midpoints, error);
  }

  // Profile functions

  ProfileMap *GetProfileMap(TUVX *tuvx, Error *error)
  {
    DeleteError(error);
    return tuvx->CreateProfileMap(error);
  }

  void DeleteProfileMap(ProfileMap *profile_map, Error *error)
  {
    *error = NoError();
    try
    {
      delete profile_map;
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
  }

  Profile *GetProfile(ProfileMap *profile_map, const char *profile_name, const char *profile_units, Error *error)
  {
    DeleteError(error);
    return profile_map->GetProfile(profile_name, profile_units, error);
  }

  void DeleteProfile(Profile *profile, Error *error)
  {
    *error = NoError();
    try
    {
      delete profile;
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
  }

  void SetProfileEdgeValues(Profile *profile, double edge_values[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    profile->SetEdgeValues(edge_values, num_values, error);
  }

  void SetProfileMidpointValues(Profile *profile, double midpoint_values[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    profile->SetMidpointValues(midpoint_values, num_values, error);
  }

  void SetProfileLayerDensities(Profile *profile, double layer_densities[], std::size_t num_values, Error *error)
  {
    DeleteError(error);
    profile->SetLayerDensities(layer_densities, num_values, error);
  }

  void SetProfileExoLayerDensity(Profile *profile, double exo_layer_density, Error *error)
  {
    DeleteError(error);
    profile->SetExoLayerDensity(exo_layer_density, error);
  }

  void CalculateProfileExoLayerDensity(Profile *profile, double scale_height, Error *error)
  {
    DeleteError(error);
    profile->CalculateExoLayerDensity(scale_height, error);
  }

  // TUVX class functions

  TUVX::TUVX()
      : tuvx_(),
        grid_map_(nullptr),
        profile_map_(nullptr)
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
    *error = NoError();
    if (grid_map_ == nullptr)
    {
      int error_code = 0;
      grid_map_ = std::make_unique<GridMap>(InternalGetGridMap(tuvx_, &error_code));
      if (error_code != 0)
      {
        *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create grid map") };
        return nullptr;
      }
    }
    return grid_map_.get();
  }

  ProfileMap *TUVX::CreateProfileMap(Error *error)
  {
    *error = NoError();
    if (profile_map_ == nullptr)
    {
      int error_code = 0;
      profile_map_ = std::make_unique<ProfileMap>(InternalGetProfileMap(tuvx_, &error_code));
      if (error_code != 0)
      {
        *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create profile map") };
        return nullptr;
      }
    }
    return profile_map_.get();
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

  // Grid class functions

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

  // ProfileMap class functions

  ProfileMap::~ProfileMap()
  {
    // At the time of writing, the profile map pointer is owned by fortran memory
    // in the tuvx core and should not be deleted here. It will be deleted when
    // the tuvx instance is deleted
    int error_code = 0;
    profile_map_ = nullptr;
  }

  Profile *ProfileMap::GetProfile(const char *profile_name, const char *profile_units, Error *error)
  {
    if (profile_map_ == nullptr)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Profile map is null") };
      return nullptr;
    }

    int error_code = 0;
    Profile *profile = nullptr;

    try
    {
      *error = NoError();

      profile = new Profile(InternalGetProfile(profile_map_, profile_name, strlen(profile_name), profile_units, strlen(profile_units), &error_code));

      if (error_code != 0)
      {
        delete profile;
        profile = nullptr;
        *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create profile map") };
      }
      else
      {
        profiles_.push_back(std::unique_ptr<Profile>(profile));
      }
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
    catch (...)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create profile") };
    }

    return profile;
  }

  // Profile class functions

  Profile::~Profile()
  {
    int error_code = 0;
    if (profile_ != nullptr)
      InternalDeleteProfile(profile_, &error_code);
    profile_ = nullptr;
  }

  void Profile::SetEdgeValues(double edge_values[], std::size_t num_values, Error *error)
  {
    int error_code = 0;
    InternalSetEdgeValues(profile_, edge_values, num_values, &error_code);
    *error = NoError();
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set edge values") };
    }
  }

  void Profile::SetMidpointValues(double midpoint_values[], std::size_t num_values, Error *error)
  {
    int error_code = 0;
    InternalSetMidpointValues(profile_, midpoint_values, num_values, &error_code);
    *error = NoError();
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set midpoint values") };
    }
  }

  void Profile::SetLayerDensities(double layer_densities[], std::size_t num_values, Error *error)
  {
    int error_code = 0;
    InternalSetLayerDensities(profile_, layer_densities, num_values, &error_code);
    *error = NoError();
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set layer densities") };
    }
  }

  void Profile::SetExoLayerDensity(double exo_layer_density, Error *error)
  {
    int error_code = 0;
    InternalSetExoLayerDensity(profile_, exo_layer_density, &error_code);
    *error = NoError();
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set exo layer density") };
    }
  }

  void Profile::CalculateExoLayerDensity(double scale_height, Error *error)
  {
    int error_code = 0;
    InternalCalculateExoLayerDensity(profile_, scale_height, &error_code);
    *error = NoError();
    if (error_code != 0)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to calculate exo layer density") };
    }
  }

}  // namespace musica
