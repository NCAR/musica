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

namespace musica
{

  class TUVX;

#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for TUVX
    // callable by external Fortran models
    TUVX *CreateTuvx(const char *config_path, int *error_code);
    void DeleteTuvx(const TUVX *tuvx);

    // for use by musica interanlly. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void *InternalCreateTuvx(String config_path, int *error_code);
    void InternalDeleteTuvx(void *tuvx, int *error_code);

#ifdef __cplusplus
  }
#endif

  class TUVX
  {
   public:
    /// @brief Create an instance ove tuvx from a configuration file
    /// @param config_path Path to configuration file or directory containing configuration file
    /// @return 0 on success, else on failure in parsing configuration file
    int Create(const std::string &config_path);

    ~TUVX();

   private:
    std::unique_ptr<void *> tuvx_;
  };
}  // namespace musica