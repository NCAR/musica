// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the defintion of the TUVX class, which represents a photolysis calculator.
// It also includes functions for creating and deleting TUVX instances with c binding.
#pragma once

#include <musica/util.hpp>
#include <musica/tuvx/grid.hpp>

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

#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for TUVX
    // callable by wrappers in other languages
    void SetGridEdges(Grid *grid, double edges[], std::size_t num_edges, Error *error);
    void SetGridMidpoints(Grid *grid, double midpoints[], std::size_t num_midpoints, Error *error);

    // for use by musica interanlly. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void InternalDeleteGrid(void *grid, int *error_code);
    void InternalSetEdges(void *grid, double edges[], std::size_t num_edges, int *error_code);
    void InternalSetMidpoints(void *grid, double midpoints[], std::size_t num_midpoints, int *error_code);

#ifdef __cplusplus
  }
#endif

}  // namespace musica
