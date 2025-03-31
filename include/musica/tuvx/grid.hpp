// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/util.hpp>

#include <memory>
#include <string>
#include <vector>

namespace musica
{
  class GridMap;
  class Profile;
  class Radiator;

  /// @brief A grid class used to access grid information in tuvx
  class Grid
  {
   public:
    /// @brief Creates a grid instance
    /// @param grid_name The name of the grid
    /// @param units The units of the grid
    /// @param num_sections The number of sections in the grid
    /// @param error The error struct to indicate success or failure
    Grid(const char *grid_name, const char *units, std::size_t num_sections, Error *error);

    ~Grid();

    /// @brief Return the number of sections in the grid
    /// @param error The error struct to indicate success or failure
    /// @return The number of sections in the grid
    std::size_t GetNumSections(Error *error);

    /// @brief Set the edges of the grid
    /// @param edges The edges of the grid
    /// @param num_edges the number of edges
    /// @param error the error struct to indicate success or failure
    void SetEdges(double edges[], std::size_t num_edges, Error *error);

    /// @brief Get the edges of the grid
    /// @param edges The edges of the grid
    /// @param num_edges the number of edges
    /// @param error the error struct to indicate success or failure
    void GetEdges(double edges[], std::size_t num_edges, Error *error);

    /// @brief Set the midpoints of the grid
    /// @param midpoints The midpoints of the grid
    /// @param num_midpoints the number of midpoints
    /// @param error the error struct to indicate success or failure
    void SetMidpoints(double midpoints[], std::size_t num_midpoints, Error *error);

    /// @brief Get the midpoints of the grid
    /// @param midpoints The midpoints of the grid
    /// @param num_midpoints the number of midpoints
    /// @param error the error struct to indicate success or failure
    void GetMidpoints(double midpoints[], std::size_t num_midpoints, Error *error);

   private:
    void *grid_;  // A valid pointer to a grid instance indicates ownership by this wrapper
    void *updater_;

    friend class GridMap;
    friend class Profile;
    friend class Radiator;

    /// @brief Wraps an existing grid instance. Used by GridMap
    /// @param updater The updater for the grid
    Grid(void *updater)
        : grid_(nullptr),
          updater_(updater)
    {
    }
  };

#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for TUVX
    // callable by wrappers in other languages

    /// @brief Creates a TUV-x grid instance
    /// @param grid_name The name of the grid
    /// @param units The units of the grid
    /// @param num_sections The number of sections in the grid
    /// @param error The error struct to indicate success or failure
    Grid *CreateGrid(const char *grid_name, const char *units, std::size_t num_sections, Error *error);

    /// @brief Gets the number of sections in the grid
    /// @param grid The grid to get the number of sections from
    /// @param error The error struct to indicate success or failure
    /// @return The number of sections in the grid
    std::size_t GetGridNumSections(Grid *grid, Error *error);

    /// @brief Deletes a TUV-x grid instance
    /// @param grid The grid to delete
    /// @param error The error struct to indicate success or failure
    void DeleteGrid(Grid *grid, Error *error);

    /// @brief Sets the values of the edges of the grid
    /// @param grid The grid to set the edges of
    /// @param edges The edge values to set for the grid
    /// @param num_edges The number of edges
    /// @param error The error struct to indicate success or failure
    void SetGridEdges(Grid *grid, double edges[], std::size_t num_edges, Error *error);

    /// @brief Gets the values of the edges of the grid
    /// @param grid The grid to get the edges of
    /// @param edges The edge values to get for the grid
    /// @param num_edges The number of edges
    /// @param error The error struct to indicate success or failure
    void GetGridEdges(Grid *grid, double edges[], std::size_t num_edges, Error *error);

    /// @brief Sets the values of the midpoints of the grid
    /// @param grid The grid to set the midpoints of
    /// @param midpoints The midpoint values to set for the grid
    /// @param num_midpoints The number of midpoints
    /// @param error The error struct to indicate success or failure
    void SetGridMidpoints(Grid *grid, double midpoints[], std::size_t num_midpoints, Error *error);

    /// @brief Gets the values of the midpoints of the grid
    /// @param grid The grid to get the midpoints of
    /// @param midpoints The midpoint values to get for the grid
    /// @param num_midpoints The number of midpoints
    /// @param error The error struct to indicate success or failure
    void GetGridMidpoints(Grid *grid, double midpoints[], std::size_t num_midpoints, Error *error);

    // INTERNAL USE. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void *InternalCreateGrid(
        const char *grid_name,
        std::size_t grid_name_length,
        const char *units,
        std::size_t units_length,
        std::size_t num_sections,
        int *error_code);
    void InternalDeleteGrid(void *grid, int *error_code);
    void *InternalGetGridUpdater(void *grid, int *error_code);
    void InternalDeleteGridUpdater(void *updater, int *error_code);
    std::string InternalGetGridName(void *grid, int *error_code);
    std::string InternalGetGridUnits(void *grid, int *error_code);
    std::size_t InternalGetNumSections(void *grid, int *error_code);
    void InternalSetEdges(void *grid, double edges[], std::size_t num_edges, int *error_code);
    void InternalGetEdges(void *grid, double edges[], std::size_t num_edges, int *error_code);
    void InternalSetMidpoints(void *grid, double midpoints[], std::size_t num_midpoints, int *error_code);
    void InternalGetMidpoints(void *grid, double midpoints[], std::size_t num_midpoints, int *error_code);

#ifdef __cplusplus
  }
#endif

}  // namespace musica
