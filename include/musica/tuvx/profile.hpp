// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/tuvx/grid.hpp>
#include <musica/tuvx/profile.hpp>
#include <musica/util.hpp>

#include <memory>
#include <string>
#include <vector>

namespace musica
{
  class ProfileMap;

  /// @brief A class used to interact with TUV-x profiles (properties with values on a grid)
  class Profile
  {
   public:
    /// @brief Creates a profile instance
    /// @param profile_name The name of the profile
    /// @param units The units of the profile
    /// @param grid The grid to use for the profile
    /// @param error The error struct to indicate success or failure
    Profile(const char *profile_name, const char *units, Grid *grid, Error *error);

    ~Profile();

    /// @brief Sets the profile values at the edges of the grid
    /// @param edge_values The values at the edges of the grid
    /// @param num_values The number of values
    /// @param error The error struct to indicate success or failure
    void SetEdgeValues(double edge_values[], std::size_t num_values, Error *error);

    /// @brief Gets the profile values at the edges of the grid
    /// @param edge_values The values at the edges of the grid
    /// @param num_values The number of values
    /// @param error The error struct to indicate success or failure
    void GetEdgeValues(double edge_values[], std::size_t num_values, Error *error);

    /// @brief Sets the profile values at the midpoints of the grid
    /// @param midpoint_values The values at the midpoints of the grid
    /// @param num_values The number of values
    /// @param error The error struct to indicate success or failure
    void SetMidpointValues(double midpoint_values[], std::size_t num_values, Error *error);

    /// @brief Gets the profile values at the midpoints of the grid
    /// @param midpoint_values The values at the midpoints of the grid
    /// @param num_values The number of values
    /// @param error The error struct to indicate success or failure
    void GetMidpointValues(double midpoint_values[], std::size_t num_values, Error *error);

    /// @brief Sets the layer densities for each grid section
    /// @param layer_densities The layer densities
    /// @param num_values The number of values
    /// @param error The error struct to indicate success or failure
    void SetLayerDensities(double layer_densities[], std::size_t num_values, Error *error);

    /// @brief Gets the layer densities for each grid section
    /// @param layer_densities The layer densities
    /// @param num_values The number of values
    /// @param error The error struct to indicate success or failure
    void GetLayerDensities(double layer_densities[], std::size_t num_values, Error *error);

    /// @brief Sets the layer density above the top of the grid
    /// @param exo_layer_density The layer density above the top of the grid
    /// @param error The error struct to indicate success or failure
    void SetExoLayerDensity(double exo_layer_density, Error *error);

    /// @brief Calculates an exo layer density based on a provided scale height
    /// @param scale_height The scale height to use in the calculation
    /// @param error The error struct to indicate success or failure
    void CalculateExoLayerDensity(double scale_height, Error *error);

    /// @brief Gets the layer density above the top of the grid
    /// @param error The error struct to indicate success or failure
    /// @return The layer density above the top of the grid
    double GetExoLayerDensity(Error *error);

   private:
    void *profile_;  // A valid pointer to a profile instance indicates ownership by this wrapper
    void *updater_;

    friend class ProfileMap;

    /// @brief Wraps an existing profile instance
    /// @param updater The updater for the profile
    Profile(void *updater)
        : profile_(nullptr),
          updater_(updater)
    {
    }
  };

#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for TUVX
    // callable by wrappers in other languages

    /// @brief Creates a new profile instance
    /// @param profile_name The name of the profile
    /// @param units The units of the profile
    /// @param grid The grid to use for the profile
    /// @param error The error struct to indicate success or failure
    Profile *CreateProfile(const char *profile_name, const char *units, Grid *grid, Error *error);

    /// @brief Deletes a profile instance
    /// @param profile The profile to delete
    /// @param error The error struct to indicate success or failure
    void DeleteProfile(Profile *profile, Error *error);

    /// @brief Sets the values at edges of the profile grid
    /// @param profile The profile to set the edge values of
    /// @param edge_values The edge values to set for the profile
    /// @param num_values The number of values
    /// @param error The error struct to indicate success or failure
    void SetProfileEdgeValues(Profile *profile, double edge_values[], std::size_t num_values, Error *error);

    /// @brief Gets the values at edges of the profile grid
    /// @param profile The profile to get the edge values of
    /// @param edge_values The edge values to get for the profile
    /// @param num_values The number of values
    /// @param error The error struct to indicate success or failure
    void GetProfileEdgeValues(Profile *profile, double edge_values[], std::size_t num_values, Error *error);

    /// @brief Sets the values at midpoints of the profile grid
    /// @param profile The profile to set the midpoint values of
    /// @param midpoint_values The midpoint values to set for the profile
    /// @param num_values The number of values
    /// @param error The error struct to indicate success or failure
    void SetProfileMidpointValues(Profile *profile, double midpoint_values[], std::size_t num_values, Error *error);

    /// @brief Gets the values at midpoints of the profile grid
    /// @param profile The profile to get the midpoint values of
    /// @param midpoint_values The midpoint values to get for the profile
    /// @param num_values The number of values
    /// @param error The error struct to indicate success or failure
    void GetProfileMidpointValues(Profile *profile, double midpoint_values[], std::size_t num_values, Error *error);

    /// @brief Sets the layer densities for each grid section of the profile
    /// @param profile The profile to set the layer densities of
    /// @param layer_densities The layer densities to set for the profile
    /// @param num_values The number of values
    /// @param error The error struct to indicate success or failure
    void SetProfileLayerDensities(Profile *profile, double layer_densities[], std::size_t num_values, Error *error);

    /// @brief Gets the layer densities for each grid section of the profile
    /// @param profile The profile to get the layer densities of
    /// @param layer_densities The layer densities to get for the profile
    /// @param num_values The number of values
    /// @param error The error struct to indicate success or failure
    void GetProfileLayerDensities(Profile *profile, double layer_densities[], std::size_t num_values, Error *error);

    /// @brief Sets the layer density above the top of the profile grid
    /// @param profile The profile to set the exo layer density of
    /// @param exo_layer_density The exo layer density to set for the profile
    /// @param error The error struct to indicate success or failure
    void SetProfileExoLayerDensity(Profile *profile, double exo_layer_density, Error *error);

    /// @brief Calculates an exo layer density based on a provided scale height
    /// @param profile The profile to calculate the exo layer density of
    /// @param scale_height The scale height to use in the calculation
    /// @param error The error struct to indicate success or failure
    void CalculateProfileExoLayerDensity(Profile *profile, double scale_height, Error *error);

    /// @brief Gets the density above the top of the profile grid
    /// @param profile The profile to get the exo layer density of
    /// @param error The error struct to indicate success or failure
    /// @return The exo layer density
    double GetProfileExoLayerDensity(Profile *profile, Error *error);

    // INTERNAL USE. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void *InternalCreateProfile(
        const char *profile_name,
        std::size_t profile_name_length,
        const char *units,
        std::size_t units_length,
        void *grid,
        int *error_code);
    void InternalDeleteProfile(void *profile, int *error_code);
    void *InternalGetProfileUpdater(void *profile, int *error_code);
    void InternalDeleteProfileUpdater(void *updater, int *error_code);
    std::string InternalGetProfileName(void *profile, int *error_code);
    std::string InternalGetProfileUnits(void *profile, int *error_code);
    void InternalSetEdgeValues(void *profile, double edge_values[], std::size_t num_values, int *error_code);
    void InternalGetEdgeValues(void *profile, double edge_values[], std::size_t num_values, int *error_code);
    void InternalSetMidpointValues(void *profile, double midpoint_values[], std::size_t num_values, int *error_code);
    void InternalGetMidpointValues(void *profile, double midpoint_values[], std::size_t num_values, int *error_code);
    void InternalSetLayerDensities(void *profile, double layer_densities[], std::size_t num_values, int *error_code);
    void InternalGetLayerDensities(void *profile, double layer_densities[], std::size_t num_values, int *error_code);
    void InternalSetExoLayerDensity(void *profile, double exo_layer_density, int *error_code);
    void InternalCalculateExoLayerDensity(void *profile, double scale_height, int *error_code);
    double InternalGetExoLayerDensity(void *profile, int *error_code);

#ifdef __cplusplus
  }
#endif

}  // namespace musica
