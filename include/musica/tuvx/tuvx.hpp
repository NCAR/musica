// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the defintion of the TUVX class, which represents a photolysis calculator.
// It also includes functions for creating and deleting TUVX instances with c binding.
#pragma once

#include <musica/tuvx/grid_map.hpp>
#include <musica/tuvx/profile_map.hpp>
#include <musica/tuvx/radiator_map.hpp>
#include <musica/util.hpp>

#include <memory>
#include <string>
#include <vector>

namespace musica
{

  class TUVX
  {
   public:
    TUVX();

    /// @brief Create an instance of tuvx from a configuration file
    /// @param config_path Path to configuration file
    /// @param error Error struct to indicate success or failure
    void Create(const char *config_path, Error *error);

    /// @brief Create a grid map. For now, this calls the interal tuvx fortran api, but will allow the change to c++ later on
    /// to be transparent to downstream projects
    /// @param error The error struct to indicate success or failure
    /// @return a grid map pointer
    GridMap *CreateGridMap(Error *error);

    /// @brief Create a profile map. For now, this calls the interal tuvx fortran api, but will allow the change to c++ later
    /// on to be transparent to downstream projects
    /// @param error The error struct to indicate success or failure
    /// @return a profile map pointer
    ProfileMap *CreateProfileMap(Error *error);

    /// @brief Create a radiator map. For now, this calls the interal tuvx fortran api, but will allow the change to c++
    /// later on to be transparent to downstream projects
    /// @param error The error struct to indicate success or failure
    /// @return a radiator map pointer
    RadiatorMap *CreateRadiatorMap(Error *error);

    ~TUVX();

   private:
    void *tuvx_;
  };

#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for TUVX
    // callable by wrappers in other languages
    TUVX *CreateTuvx(const char *config_path, Error *error);
    void DeleteTuvx(const TUVX *tuvx, Error *error);
    GridMap *GetGridMap(TUVX *tuvx, Error *error);
    ProfileMap *GetProfileMap(TUVX *tuvx, Error *error);
    RadiatorMap *GetRadiatorMap(TUVX *tuvx, Error *error);

    // for use by musica interanlly. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void *InternalCreateTuvx(const char *config_path, std::size_t config_path_length, int *error_code);
    void InternalDeleteTuvx(void *tuvx, int *error_code);
    void *InternalGetGridMap(void *tuvx, int *error_code);
    void *InternalGetProfileMap(void *tuvx, int *error_code);
    void *InternalGetRadiatorMap(void *tuvx, int *error_code);

#ifdef __cplusplus
  }
#endif

}  // namespace musica
