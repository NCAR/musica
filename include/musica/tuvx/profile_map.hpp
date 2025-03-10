// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/tuvx/profile.hpp>
#include <musica/tuvx/profile_map.hpp>
#include <musica/util.hpp>

#include <memory>
#include <string>
#include <vector>

namespace musica
{

  /// @brief A class used to store a collection of profiles
  class ProfileMap
  {
   public:
    ProfileMap(void *profile_map)
        : profile_map_(profile_map),
          owns_profile_map_(false)
    {
    }

    /// @brief Creates a profile map instance
    /// @param error The error struct to indicate success or failure
    ProfileMap(Error *error);

    ~ProfileMap();

    /// @brief Adds a profile to the profile map
    /// @param profile The profile to add
    /// @param error The error struct to indicate success or failure
    void AddProfile(Profile *profile, Error *error);

    /// @brief Returns a profile. For now, this calls the interal tuvx fortran api, but will allow the change to c++ later on
    /// to be transparent to downstream projects
    /// @param profile_name The name of the profile we want
    /// @param profile_units The units of the profile we want
    /// @param error The error struct to indicate success or failure
    /// @return a profile pointer
    Profile *GetProfile(const char *profile_name, const char *profile_units, Error *error);

   private:
    void *profile_map_;
    bool owns_profile_map_;

    friend class TUVX;
  };

#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for TUVX
    // callable by wrappers in other languages

    /// @brief Creates a profile map instance
    /// @param error The error struct to indicate success or failure
    /// @return a pointer to the profile map
    ProfileMap *CreateProfileMap(Error *error);

    /// @brief Deletes a profile map instance
    /// @param profile_map The profile map to delete
    /// @param error The error struct to indicate success or failure
    void DeleteProfileMap(ProfileMap *profile_map, Error *error);

    /// @brief Adds a profile to the profile map
    /// @param profile_map The profile map to add the profile to
    /// @param profile The profile to add
    /// @param error The error struct to indicate success or failure
    void AddProfile(ProfileMap *profile_map, Profile *profile, Error *error);

    /// @brief Returns a profile from the profile map
    /// @param profile_map The profile map to get the profile from
    /// @param profile_name The name of the profile we want
    /// @param profile_units The units of the profile we want
    /// @param error The error struct to indicate success or failure
    /// @return a profile pointer, or nullptr if the profile is not found
    Profile *GetProfile(ProfileMap *profile_map, const char *profile_name, const char *profile_units, Error *error);

    // INTERNAL USE. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void *InternalCreateProfileMap(int *error_code);
    void InternalDeleteProfileMap(void *profile_map, int *error_code);
    void InternalAddProfile(void *profile_map, void *profile, int *error_code);
    void *InternalGetProfile(
        void *profile_map,
        const char *profile_name,
        std::size_t profile_name_length,
        const char *profile_units,
        std::size_t profile_units_length,
        int *error_code);
    void *InternalGetProfileUpdaterFromMap(void *profile_map, void *profile, int *error_code);

#ifdef __cplusplus
  }
#endif

}  // namespace musica
