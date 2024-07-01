// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the defintion of the TUVX class, which represents a photolysis calculator.
// It also includes functions for creating and deleting TUVX instances with c binding.
#pragma once

#include <musica/util.hpp>
#include <musica/tuvx/grid_map.hpp>
#include <musica/tuvx/grid.hpp>

#include <memory>
#include <string>
#include <vector>

namespace musica
{

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

#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for TUVX
    // callable by wrappers in other languages
    Grid *GetGrid(GridMap *grid_map, const char *grid_name, const char *grid_units, Error *error);

    // for use by musica interanlly. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void *InternalGetGrid(
        void *grid_map,
        const char *grid_name,
        std::size_t grid_name_length,
        const char *grid_units,
        std::size_t grid_units_length,
        int *error_code);

#ifdef __cplusplus
  }
#endif

}  // namespace musica
