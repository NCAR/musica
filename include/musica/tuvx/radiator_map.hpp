// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/tuvx/radiator.hpp>
#include <musica/util.hpp>

#include <memory>
#include <string>
#include <vector>

namespace musica
{

  /// @brief A radiator map struct used to access radiator information in tuvx
  struct RadiatorMap
  {
    RadiatorMap(void *radiator_map)
        : radiator_map_(radiator_map),
          owns_radiator_map_(false)
    {
    }

    /// @brief  @brief Creates a radiator map instance
    /// @param error The error struct to indicate success or failure
    RadiatorMap(Error *error);

    ~RadiatorMap();

    /// @brief Adds a radiator to the radiator map
    /// @param radiator The radiator to add
    /// @param error The error struct to indicate success or failure
    void AddRadiator(Radiator *radiator, Error *error);

    /// @brief Returns a radiator. For now, this calls the interal tuvx fortran api, but will allow the change to c++ later
    /// on to be transparent to downstream projects
    /// @param radiator_name The name of the radiator
    /// @param error The error struct to indicate success or failure
    /// @return a radiator pointer
    Radiator *GetRadiator(const char *radiator_name, Error *error);

   private:
    void *radiator_map_;
    bool owns_radiator_map_;
  };

#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for TUVX
    // callable by wrappers in other languages

    /// @brief Creates a radiator map instance
    /// @param error The error struct to indicate success or failure
    /// @return a pointer to the radiator map
    RadiatorMap *CreateRadiatorMap(Error *error);

    /// @brief Deletes a radiator map instance
    /// @param radiator_map The radiator map to delete
    /// @param error The error struct to indicate success or failure
    void DeleteRadiatorMap(RadiatorMap *radiator_map, Error *error);

    /// @brief Adds a radiator to the radiator map
    /// @param radiator_map The radiator map to add the radiator to
    /// @param radiator The radiator to add
    /// @param error The error struct to indicate success or failure
    void AddRadiator(RadiatorMap *radiator_map, Radiator *radiator, Error *error);

    /// @brief Returns a radiator from the radiator map
    /// @param radiator_map The radiator map to get the radiator from
    /// @param radiator_name The name of the radiator
    /// @param error The error struct to indicate success or failure
    /// @return The radiator pointer, or nullptr if the radiator is not found
    Radiator *GetRadiator(RadiatorMap *radiator_map, const char *radiator_name, Error *error);

    // INTERNAL USE. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void *InternalCreateRadiatorMap(int *error_code);
    void InternalDeleteRadiatordMap(void *radiator_map, int *error_code);
    void InternalAddRadiator(void *radiator_map, void *radiator, int *error_code);
    void *
    InternalGetRadiator(void *radiator_map, const char *radiator_name, std::size_t radiator_name_length, int *error_code);
    void *InternalGetRadiatorUpdaterFromMap(void *radiator_map, void *radiator, int *error_code);

#ifdef __cplusplus
  }
#endif

}  // namespace musica
