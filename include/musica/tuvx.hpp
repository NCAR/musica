/**
 * This file contains the defintion of the MICM class, which represents a multi-component
 * reactive transport model. It also includes functions for creating and deleting MICM instances with c binding
 * Copyright (C) 2023-2024 National Center for Atmospheric Research,
 *
 * SPDX-License-Identifier: Apache-2.0* creating solvers, and solving the model.
 */
#pragma once

#include <memory>
#include <string>
#include <vector>

#include <musica/util.hpp>

namespace musica {

class TUVX;

#ifdef __cplusplus
extern "C"
{
#endif

    TUVX *create_tuvx(const char *config_path, int *error_code);
    void delete_tuvx(const TUVX *tuvx);

#ifdef __cplusplus
}
#endif

class TUVX
{
public:
    /// @brief Create a solver by reading and parsing configuration file
    /// @param config_path Path to configuration file or directory containing configuration file
    /// @return 0 on success, 1 on failure in parsing configuration file
    int create(const std::string &config_path);
};
}