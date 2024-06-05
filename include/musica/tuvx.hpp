/* Copyright (C) 2023-2024 National Center for Atmospheric Research
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * This file contains the defintion of the TUVX class, which represents a photolysis calculator.
 * It also includes functions for creating and deleting TUVX instances with c binding.
 */
#pragma once

#include <musica/util.hpp>

#include <memory>
#include <string>
#include <vector>

namespace musica {

    /// @brief A grid struct used to access grid information in tuvx
    struct Grid
    {
        Grid(void* grid) : grid_(grid) {}
        ~Grid();

        /// @brief Set the edges of the grid
        /// @param edges The edges of the grid
        /// @param num_edges the number of edges
        /// @param error the error struct to indicate success or failure
        void set_edges(double edges[], std::size_t num_edges, Error *error);

        /// @brief Set the midpoints of the grid
        /// @param midpoints The midpoints of the grid
        /// @param num_midpoints the number of midpoints
        /// @param error the error struct to indicate success or failure
        void set_midpoints(double midpoints[], std::size_t num_midpoints, Error *error);

        private:
            void* grid_;
    };

    /// @brief A grid map struct used to access grid information in tuvx
    struct GridMap
    {
        GridMap(void* grid_map) : grid_map_(grid_map) {}
        ~GridMap();


        /// @brief Returns a grid. For now, this calls the interal tuvx fortran api, but will allow the change to c++ later on to be transparent to downstream projects
        /// @param grid_name The name of the grid we want
        /// @param grid_units The units of the grid we want
        /// @param error The error struct to indicate success or failure
        /// @return a grid pointer
        Grid* get_grid(const char* grid_name, const char* grid_units, Error *error);

        private:
            void* grid_map_;
            std::vector<std::unique_ptr<Grid>> grids_;
    };


class TUVX;

#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for TUVX
    // callable by external Fortran models
    TUVX *create_tuvx(const char *config_path, Error *error);
    void delete_tuvx(const TUVX *tuvx, Error *error);
    void run_tuvx(const TUVX *tuvx, Error *error);
    GridMap* get_grid_map(TUVX *tuvx, Error *error);
    void delete_grid_map(GridMap* grid_map, Error *error);
    Grid* get_grid(GridMap* grid_map, const char* grid_name, const char* grid_units, Error *error);
    void delete_grid(Grid* grid, Error *error);
    void set_edges(Grid* grid, double edges[], std::size_t num_edges, Error *error);
    void set_midpoints(Grid* grid, double midpoints[], std::size_t num_midpoints, Error *error);
    

    // for use by musica interanlly. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void *internal_create_tuvx(String config_path, int *error_code);
    void internal_delete_tuvx(void* tuvx, int *error_code);
    void *internal_get_grid_map(void* tuvx, int *error_code);
    void internal_delete_grid_map(void* grid_map, int *error_code);
    void *internal_get_grid(void* grid_map, String grid_name, String grid_units, int *error_code);
    void internal_delete_grid(void* grid, int *error_code);
    void internal_set_edges(void* grid, double edges[], std::size_t num_edges, int *error_code);
    void internal_set_midpoints(void* grid, double midpoints[], std::size_t num_midpoints, int *error_code);

#ifdef __cplusplus
  }
#endif

class TUVX
{
public:
    TUVX();

    /// @brief Create an instance ove tuvx from a configuration file
    /// @param config_path Path to configuration file or directory containing configuration file
    /// @param error Error struct to indicate success or failure
    /// @return 0 on success, 1 on failure in parsing configuration file
    void create(const std::string &config_path, Error *error);

    /// @brief Create a grid map. For now, this calls the interal tuvx fortran api, but will allow the change to c++ later on to be transparent to downstream projects
    /// @param error The error struct to indicate success or failure
    /// @return a grid map pointer
    GridMap* create_grid_map(Error *error);

    ~TUVX();
private:
    void* tuvx_;
    std::unique_ptr<GridMap> grid_map_;
};
}
