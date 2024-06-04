/* Copyright (C) 2023-2024 National Center for Atmospheric Research
 *
 * SPDX-License-Identifier: Apache-2.0* creating solvers, and solving the model.
 *
 * This file contains the implementation of the TUVX class, which represents a multi-component
 * reactive transport model. It also includes functions for creating and deleting TUVX instances.
 */

#include <musica/tuvx.hpp>

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace musica
{

  TUVX *CreateTuvx(const char *config_path, int *error_code)
  {
    if (!fs::exists(config_path))
    {
      std::cerr << "File doesn't exist: " << config_path << std::endl;
      *error_code = 2;
      return nullptr;
    }

    TUVX *tuvx = new TUVX();
    *error_code = tuvx->Create(std::string(config_path));

    return (*error_code == 0) ? tuvx : nullptr;
  }

  void DeleteTuvx(const TUVX *tuvx)
  {
    delete tuvx;
  }

  TUVX::~TUVX()
  {
    int error_code = 0;
    if (tuvx_ != nullptr)
    {
      InternalDeleteTuvx(tuvx_.get(), &error_code);
    }
  }

  int TUVX::Create(const std::string &config_path)
  {
    int parsing_status = 0;  // 0 on success, else on failure
    String config_path_str = CreateString(const_cast<char *>(config_path.c_str()));

    try
    {
      tuvx_ = std::make_unique<void *>(InternalCreateTuvx(config_path_str, &parsing_status));
      DeleteString(&config_path_str);
    }
    catch (const std::bad_alloc &e)
    {
      parsing_status = 1;
      std::cerr << e.what() << std::endl;
      DeleteString(&config_path_str);
    }
    catch (const std::exception &e)
    {
      parsing_status = 2;
      std::cerr << e.what() << std::endl;
      DeleteString(&config_path_str);
    }

    return parsing_status;
  }
}  // namespace musica