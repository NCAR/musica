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

TUVX *create_tuvx(const char *config_path, Error *error)
{
  DeleteError(error);
  TUVX *tuvx = new TUVX();
  tuvx->create(std::string(config_path), error);
  if (!IsSuccess(*error)) {
    delete tuvx;
    return nullptr;
  }
  return tuvx;
}

void delete_tuvx(const TUVX *tuvx, Error *error)
{
  DeleteError(error);
  if (tuvx == nullptr) {
    *error = NoError();
    return;
  }
  try {
    delete tuvx;
    *error = NoError();
  } catch (const std::system_error &e) {
    *error = ToError(e);
  }
}

void run_tuvx(const TUVX *tuvx, Error *error) {
    DeleteError(error);
    *error = NoError();
}

GridMap* get_grid_map(TUVX *tuvx, Error *error) {
    DeleteError(error);
    return tuvx->create_grid_map(error);
}

TUVX::~TUVX()
{
    int error_code = 0;
    internal_delete_tuvx(tuvx_.get(), &error_code);
}

int TUVX::create(const std::string &config_path, Error *error)
{
    int parsing_status = 0; // 0 on success, 1 on failure
    String config_path_str = CreateString(const_cast<char *>(config_path.c_str()));
    tuvx_ = std::make_unique<void*>(internal_create_tuvx(config_path_str, &parsing_status));
    DeleteString(&config_path_str);
    return parsing_status;
}

GridMap* TUVX::create_grid_map(Error *error)
{
    int error_code = 0;
    internal_get_grid_map(tuvx_.get(), &error_code);
}
}