/**
 * This file contains the implementation of the TUVX class, which represents a multi-component
 * reactive transport model. It also includes functions for creating and deleting TUVX instances,
 * Copyright (C) 2023-2024 National Center for Atmospheric Research,
 *
 * SPDX-License-Identifier: Apache-2.0* creating solvers, and solving the model.
 */

#include <iostream>

#include <musica/tuvx.hpp>

namespace musica {

TUVX *create_tuvx(const char *config_path, Error *error)
{
  DeleteError(error);
  TUVX *tuvx = new TUVX();
  tuvx->create(std::string(config_path), error);
  if (!IsSuccess(*error)) {
    delete tuvx;
    return nullptr;
  }
  return tuvx;
}

void delete_tuvx(const TUVX *tuvx, Error *error)
{
  DeleteError(error);
  if (tuvx == nullptr) {
    *error = NoError();
    return;
  }
  try {
    delete tuvx;
    *error = NoError();
  } catch (const std::system_error &e) {
    *error = ToError(e);
  }
}

void run_tuvx(const TUVX *tuvx, Error *error) {
    DeleteError(error);
    *error = NoError();
}

GridMap* get_grid_map(TUVX *tuvx, Error *error) {
    DeleteError(error);
    return tuvx->create_grid_map(error);
}

Grid* get_grid(GridMap* grid_map, const char* grid_name, const char* grid_units, Error *error) {
    DeleteError(error);
    return grid_map->get_grid(grid_name, grid_units, error);
}

void set_edges(Grid* grid, double edges[], std::size_t num_edges, Error *error) {
    grid->set_edges(edges, num_edges, error);
}

void set_midpoints(Grid* grid, double midpoints[], std::size_t num_midpoints, Error *error) {
    grid->set_midpoints(midpoints, num_midpoints, error);
}

TUVX::TUVX() : tuvx_(), grid_map_(nullptr) {}

TUVX::~TUVX()
{
    int error_code = 0;
    if (tuvx_ != nullptr) internal_delete_tuvx(tuvx_, &error_code);
    tuvx_ = nullptr;
}

void TUVX::create(const std::string &config_path, Error *error)
{
    int parsing_status = 0; // 0 on success, 1 on failure
    try {
        String config_path_str = CreateString(const_cast<char *>(config_path.c_str()));
        tuvx_ = internal_create_tuvx(config_path_str, &parsing_status);
        DeleteString(&config_path_str);
        if (parsing_status == 1) {
            *error = Error{1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create tuvx instance")};
        }
        else {
            *error = NoError();
        }
    }
    catch (const std::system_error &e) {
        *error = ToError(e);
    }
    catch(...) {
        *error = Error{1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create tuvx instance")};
    }
}

GridMap* TUVX::create_grid_map(Error *error)
{
    int error_code = 0;
    grid_map_ = std::make_unique<GridMap>(internal_get_grid_map(tuvx_, &error_code));
    *error = NoError();
    if (error_code != 0) {
        *error = Error{1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create grid map")};
        return nullptr;
    }
    return grid_map_.get();
}

GridMap::~GridMap()
{
    int error_code = 0;
    if (grid_map_ != nullptr) internal_delete_grid_map(grid_map_, &error_code);
    grid_map_ = nullptr;
}

Grid* GridMap::get_grid(const char* grid_name, const char* grid_units, Error *error) {
  int error_code = 0;
  auto name = CreateString(grid_name);
  auto units = CreateString(grid_units);

  Grid* grid = new Grid(internal_get_grid(grid_map_, name, units, &error_code));
  grids_.push_back(std::unique_ptr<Grid>(grid));

  *error = NoError();
  if (error_code != 0) {
      *error = Error{1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to create grid map")};
      return nullptr;
  }
  return grid;
}

Grid::~Grid()
{
    int error_code = 0;
    if (grid_ != nullptr) internal_delete_grid(grid_, &error_code);
    grid_ = nullptr;
}


void Grid::set_edges(double edges[], std::size_t num_edges, Error *error) {
    int error_code = 0;
    internal_set_edges(grid_, edges, num_edges, &error_code);
    *error = NoError();
    if (error_code != 0) {
        *error = Error{1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set edges")};
    }
}

void Grid::set_midpoints(double midpoints[], std::size_t num_midpoints, Error *error) {
    int error_code = 0;
    internal_set_midpoints(grid_, midpoints, num_midpoints, &error_code);
    *error = NoError();
    if (error_code != 0) {
        *error = Error{1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Failed to set midpoints")};
    }
}

} // namespace musica
