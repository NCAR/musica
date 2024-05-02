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
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        *error_code = 2;
        return nullptr;
    }
}

void delete_tuvx(const TUVX *tuvx)
{
    delete tuvx;
}

TUVX::~TUVX()
{
    int error_code = 0;
    internal_delete_tuvx(tuvx_.get(), &error_code);
}

int TUVX::create(const std::string &config_path)
{
    int parsing_status = 0; // 0 on success, 1 on failure
    String config_path_str = CreateString(const_cast<char *>(config_path.c_str()));
    tuvx_ = std::make_unique<void*>(internal_create_tuvx(config_path_str, &parsing_status));
    DeleteString(&config_path_str);
    return parsing_status;
}
}