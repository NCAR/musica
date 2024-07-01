// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the defintion of the TUVX class, which represents a photolysis calculator.
// It also includes functions for creating and deleting TUVX instances with c binding.
#pragma once

#include <musica/util.hpp>
#include <musica/tuvx/profile_map.hpp>
#include <musica/tuvx/profile.hpp>

#include <memory>
#include <string>
#include <vector>

namespace musica
{

  /// @brief A struct used to store a collection of profiles
  struct ProfileMap
  {
    ProfileMap(void *profile_map)
        : profile_map_(profile_map)
    {
    }
    ~ProfileMap();

    /// @brief Returns a profile. For now, this calls the interal tuvx fortran api, but will allow the change to c++ later on to
    /// be transparent to downstream projects
    /// @param profile_name The name of the profile we want
    /// @param profile_units The units of the profile we want
    /// @param error The error struct to indicate success or failure
    /// @return a profile pointer
    Profile *GetProfile(const char *profile_name, const char *profile_units, Error *error);

   private:
    void *profile_map_;
    std::vector<std::unique_ptr<Profile>> profiles_;
  };

#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for TUVX
    // callable by wrappers in other languages
    Profile *GetProfile(ProfileMap *profile_map, const char *profile_name, const char *profile_units, Error *error);

    // for use by musica interanlly. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void *InternalGetProfile(
        void *profile_map,
        const char *profile_name,
        std::size_t profile_name_length,
        const char *profile_units,
        std::size_t profile_units_length,
        int *error_code);

#ifdef __cplusplus
  }
#endif

}  // namespace musica
