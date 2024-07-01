// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the defintion of the TUVX class, which represents a photolysis calculator.
// It also includes functions for creating and deleting TUVX instances with c binding.
#pragma once

#include <musica/util.hpp>
#include <musica/tuvx/profile.hpp>

#include <memory>
#include <string>
#include <vector>

namespace musica
{

  /// @brief A struct used to interact with TUV-x profiles (properties with values on a grid)
  struct Profile
  {
    Profile(void *profile)
        : profile_(profile)
    {
    }
    ~Profile();

    /// @brief Sets the profile values at the edges of the grid
    /// @param edge_values The values at the edges of the grid
    /// @param num_values The number of values
    /// @param error The error struct to indicate success or failure
    void SetEdgeValues(double edge_values[], std::size_t num_values, Error *error);

    /// @brief Sets the profile values at the midpoints of the grid
    /// @param midpoint_values The values at the midpoints of the grid
    /// @param num_values The number of values
    /// @param error The error struct to indicate success or failure
    void SetMidpointValues(double midpoint_values[], std::size_t num_values, Error *error);

    /// @brief Sets the layer densities for each grid section
    /// @param layer_densities The layer densities
    /// @param num_values The number of values
    /// @param error The error struct to indicate success or failure
    void SetLayerDensities(double layer_densities[], std::size_t num_values, Error *error);

    /// @brief Sets the layer density above the top of the grid
    /// @param exo_layer_density The layer density above the top of the grid
    /// @param error The error struct to indicate success or failure
    void SetExoLayerDensity(double exo_layer_density, Error *error);

    /// @brief Calculates an exo layer density based on a provided scale height
    /// @param scale_height The scale height to use in the calculation
    /// @param error The error struct to indicate success or failure
    void CalculateExoLayerDensity(double scale_height, Error *error);

   private:
    void *profile_;
  };

#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for TUVX
    // callable by wrappers in other languages
    void SetProfileEdgeValues(Profile *profile, double edge_values[], std::size_t num_values, Error *error);
    void SetProfileMidpointValues(Profile *profile, double midpoint_values[], std::size_t num_values, Error *error);
    void SetProfileLayerDensities(Profile *profile, double layer_densities[], std::size_t num_values, Error *error);
    void SetProfileExoLayerDensity(Profile *profile, double exo_layer_density, Error *error);
    void CalculateProfileExoLayerDensity(Profile *profile, double scale_height, Error *error);

    // for use by musica interanlly. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void InternalDeleteProfile(void *profile, int *error_code);
    void InternalSetEdgeValues(void *profile, double edge_values[], std::size_t num_values, int *error_code);
    void InternalSetMidpointValues(void *profile, double midpoint_values[], std::size_t num_values, int *error_code);
    void InternalSetLayerDensities(void *profile, double layer_densities[], std::size_t num_values, int *error_code);
    void InternalSetExoLayerDensity(void *profile, double exo_layer_density, int *error_code);
    void InternalCalculateExoLayerDensity(void *profile, double scale_height, int *error_code);    

#ifdef __cplusplus
  }
#endif

}  // namespace musica
