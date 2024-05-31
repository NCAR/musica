/**
 * This file contains the defintion of the TUVX class, which represents a photolysis calculator
 * It also includes functions for creating and deleting TUVX instances with c binding
 * Copyright (C) 2023-2024 National Center for Atmospheric Research,
 *
 * SPDX-License-Identifier: Apache-2.0* creating solvers, and solving the model.
 */
#pragma once

#include <memory>
#include <string>
#include <vector>

#include <musica/util.hpp>
#include <musica/grid_map.hpp>

namespace musica {

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


    // for use by musica interanlly. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void *internal_create_tuvx(String config_path, int *error_code);
    void internal_delete_tuvx(void* tuvx, int *error_code);
    void *internal_get_grid_map(void* tuvx, int *error_code);

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
    GridMap* create_grid_map(Error *error);

    ~TUVX();
private:
    void* tuvx_;
    std::unique_ptr<GridMap> grid_map_;
};
}