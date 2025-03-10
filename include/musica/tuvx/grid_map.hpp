// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/tuvx/grid.hpp>
#include <musica/util.hpp>

#include <memory>
#include <string>
#include <vector>

namespace musica
{

  /// @brief A grid map class used to access grid information in tuvx
  class GridMap
  {
   public:
    GridMap(void *grid_map)
        : grid_map_(grid_map),
          owns_grid_map_(false)
    {
    }

    /// @brief  @brief Creates a grid map instance
    /// @param error The error struct to indicate success or failure
    GridMap(Error *error);

    ~GridMap();

    /// @brief Adds a grid to the grid map
    /// @param grid The grid to add
    /// @param error The error struct to indicate success or failure
    void AddGrid(Grid *grid, Error *error);

    /// @brief Returns a grid. For now, this calls the interal tuvx fortran api, but will allow the change to c++ later on to
    /// be transparent to downstream projects
    /// @param grid_name The name of the grid we want
    /// @param grid_units The units of the grid we want
    /// @param error The error struct to indicate success or failure
    /// @return a grid pointer
    Grid *GetGrid(const char *grid_name, const char *grid_units, Error *error);

   private:
    void *grid_map_;
    bool owns_grid_map_;

    friend class TUVX;
  };

#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for TUVX
    // callable by wrappers in other languages

    /// @brief Creates a grid map instance
    /// @param error The error struct to indicate success or failure
    /// @return a pointer to the grid map
    GridMap *CreateGridMap(Error *error);

    /// @brief Deletes a grid map instance
    /// @param grid_map The grid map to delete
    /// @param error The error struct to indicate success or failure
    void DeleteGridMap(GridMap *grid_map, Error *error);

    /// @brief Adds a grid to the grid map
    /// @param grid_map The grid map to add the grid to
    /// @param grid The grid to add
    /// @param error The error struct to indicate success or failure
    void AddGrid(GridMap *grid_map, Grid *grid, Error *error);

    /// @brief Returns a grid from the grid map
    /// @param grid_map The grid map to get the grid from
    /// @param grid_name The name of the grid we want
    /// @param grid_units The units of the grid we want
    /// @param error The error struct to indicate success or failure
    /// @return The grid pointer, or nullptr if the grid is not found
    Grid *GetGrid(GridMap *grid_map, const char *grid_name, const char *grid_units, Error *error);

    // INTERNAL USE. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void *InternalCreateGridMap(int *error_code);
    void InternalDeleteGridMap(void *grid_map, int *error_code);
    void InternalAddGrid(void *grid_map, void *grid, int *error_code);
    void *InternalGetGrid(
        void *grid_map,
        const char *grid_name,
        std::size_t grid_name_length,
        const char *grid_units,
        std::size_t grid_units_length,
        int *error_code);
    void *InternalGetGridUpdaterFromMap(void *grid_map, void *grid, int *error_code);

#ifdef __cplusplus
  }
#endif

}  // namespace musica
