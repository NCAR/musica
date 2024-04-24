/**
 * This file contains the implementation of the MICM class, which represents a multi-component
 * reactive transport model. It also includes functions for creating and deleting MICM instances,
 * Copyright (C) 2023-2024 National Center for Atmospheric Research,
 *
 * SPDX-License-Identifier: Apache-2.0* creating solvers, and solving the model.
 */

#include <musica/tuvx.hpp>

namespace musica {

TUVX *create_tuvx(const char *config_path, int *error_code)
{
    try
    {
        TUVX *tuvx = new TUVX();
        *error_code = tuvx->create(std::string(config_path));
        return tuvx;
    }
    catch (const std::bad_alloc &e)
    {
        *error_code = 1;
        return nullptr;
    }
}

void delete_tuvx(const TUVX *tuvx)
{
    delete tuvx;
}

int TUVX::create(const std::string &config_path)
{
    int parsing_status = 0; // 0 on success, 1 on failure
    return parsing_status;
}

}