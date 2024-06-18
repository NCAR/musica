// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the defintion of the TUVX class, which represents a photolysis calculator.
// It also includes functions for creating and deleting TUVX instances with c binding.
#pragma once

#include <musica/util.hpp>

#include <memory>
#include <string>
#include <vector>

namespace musica
{

  /// @brief A grid struct used to access grid information in tuvx
  struct Grid
  {
    Grid(void *grid)
        : grid_(grid)
    {
    }
    ~Grid();

    /// @brief Set the edges of the grid
    /// @param edges The edges of the grid
    /// @param num_edges the number of edges
    /// @param error the error struct to indicate success or failure
    void SetEdges(double edges[], std::size_t num_edges, Error *error);

    /// @brief Set the midpoints of the grid
    /// @param midpoints The midpoints of the grid
    /// @param num_midpoints the number of midpoints
    /// @param error the error struct to indicate success or failure
    void SetMidpoints(double midpoints[], std::size_t num_midpoints, Error *error);

   private:
    void *grid_;
  };

  /// @brief A grid map struct used to access grid information in tuvx
  struct GridMap
  {
    GridMap(void *grid_map)
        : grid_map_(grid_map)
    {
    }
    ~GridMap();

    /// @brief Returns a grid. For now, this calls the interal tuvx fortran api, but will allow the change to c++ later on to
    /// be transparent to downstream projects
    /// @param grid_name The name of the grid we want
    /// @param grid_units The units of the grid we want
    /// @param error The error struct to indicate success or failure
    /// @return a grid pointer
    Grid *GetGrid(const char *grid_name, const char *grid_units, Error *error);

   private:
    void *grid_map_;
    std::vector<std::unique_ptr<Grid>> grids_;
  };

  class TUVX;

#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for TUVX
    // callable by external Fortran models
    TUVX *CreateTuvx(const char *config_path, Error *error);
    void DeleteTuvx(const TUVX *tuvx, Error *error);
    GridMap *GetGridMap(TUVX *tuvx, Error *error);
    Grid *GetGrid(GridMap *grid_map, const char *grid_name, const char *grid_units, Error *error);
    void SetEdges(Grid *grid, double edges[], std::size_t num_edges, Error *error);
    void SetMidpoints(Grid *grid, double midpoints[], std::size_t num_midpoints, Error *error);

    // for use by musica interanlly. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void *InternalCreateTuvx(const char *config_path, std::size_t config_path_length, int *error_code);
    void InternalDeleteTuvx(void *tuvx, int *error_code);
    void *InternalGetGridMap(void *tuvx, int *error_code);
    void *InternalGetGrid(
        void *grid_map,
        const char *grid_name,
        std::size_t grid_name_length,
        const char *grid_units,
        std::size_t grid_units_length,
        int *error_code);
    void InternalDeleteGrid(void *grid, int *error_code);
    void InternalSetEdges(void *grid, double edges[], std::size_t num_edges, int *error_code);
    void InternalSetMidpoints(void *grid, double midpoints[], std::size_t num_midpoints, int *error_code);

#ifdef __cplusplus
  }
#endif

  class TUVX
  {
   public:
    TUVX();

    /// @brief Create an instance of tuvx from a configuration file
    /// @param config_path Path to configuration file or directory containing configuration file
    /// @param error Error struct to indicate success or failure
    void Create(const char *config_path, Error *error);

    /// @brief Create a grid map. For now, this calls the interal tuvx fortran api, but will allow the change to c++ later on
    /// to be transparent to downstream projects
    /// @param error The error struct to indicate success or failure
    /// @return a grid map pointer
    GridMap *CreateGridMap(Error *error);

    ~TUVX();

   private:
    void *tuvx_;
    std::unique_ptr<GridMap> grid_map_;
  };
}  // namespace musica
