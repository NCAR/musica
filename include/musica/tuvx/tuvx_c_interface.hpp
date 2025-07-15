// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/tuvx/tuvx.hpp>

namespace musica
{
#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for TUVX
    // callable by wrappers in other languages

    /// @brief Creates a TUVX instance by passing a configuration file path and host-defined grids, profiles, and radiators
    /// @param config_path Path to configuration file
    /// @param grids Grid map from host application
    /// @param profiles Profile map from host application
    /// @param radiators Radiator map from host application
    /// @param error Error struct to indicate success or failure
    TUVX *CreateTuvx(const char *config_path, GridMap *grids, ProfileMap *profiles, RadiatorMap *radiators, Error *error);

    /// @brief Deletes a TUVX instance
    /// @param tuvx Pointer to TUVX instance
    /// @param error Error struct to indicate success or failure
    void DeleteTuvx(const TUVX *tuvx, Error *error);

    /// @brief Returns the set of grids used by TUVX
    /// @param tuvx Pointer to TUVX instance
    /// @param error Error struct to indicate success or failure
    /// @return Grid map
    GridMap *GetGridMap(TUVX *tuvx, Error *error);

    /// @brief Returns the set of profiles used by TUVX
    /// @param tuvx Pointer to TUVX instance
    /// @param error Error struct to indicate success or failure
    /// @return Profile map
    ProfileMap *GetProfileMap(TUVX *tuvx, Error *error);

    /// @brief Returns the set of radiators used by TUVX
    /// @param tuvx Pointer to TUVX instance
    /// @param error Error struct to indicate success or failure
    /// @return Radiator map
    RadiatorMap *GetRadiatorMap(TUVX *tuvx, Error *error);

    /// @brief Returns the ordering photolysis rate constants
    /// @param tuvx Pointer to TUVX instance
    /// @param error Error struct to indicate success or failure
    /// @return Array of photolysis rate constant name-index pairs
    Mappings GetPhotolysisRateConstantsOrdering(TUVX *tuvx, Error *error);

    /// @brief Returns the ordering of heating rates
    /// @param tuvx Pointer to TUVX instance
    /// @param error Error struct to indicate success or failure
    /// @return Array of heating rate name-index pairs
    Mappings GetHeatingRatesOrdering(TUVX *tuvx, Error *error);

    /// @brief Run the TUV-x photolysis calculator
    /// @param tuvx Pointer to TUVX instance
    /// @param solar_zenith_angle Solar zenith angle [radians]
    /// @param earth_sun_distance Earth-Sun distance [AU]
    /// @param photolysis_rate_constants Photolysis rate constant for each layer and reaction [s^-1]
    /// @param heating_rates Heating rates for each layer and reaction [K/s]
    /// @param error Error struct to indicate success or failure
    void RunTuvx(
        TUVX *tuvx,
        const double solar_zenith_angle,
        const double earth_sun_distance,
        double *const photolysis_rate_constants,
        double *const heating_rates,
        Error *const error);

    // for use by musica internally. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void *InternalCreateTuvx(
        const char *config_path,
        std::size_t config_path_length,
        void *grid_map,
        void *profile_map,
        void *radiator_map,
        int *number_of_layers,
        int *error_code);
    void InternalDeleteTuvx(void *tuvx, int *error_code);
    void *InternalGetGridMap(void *tuvx, int *error_code);
    void *InternalGetProfileMap(void *tuvx, int *error_code);
    void *InternalGetRadiatorMap(void *tuvx, int *error_code);
    Mappings InternalGetPhotolysisRateConstantsOrdering(void *tuvx, int *error_code);
    Mappings InternalGetHeatingRatesOrdering(void *tuvx, int *error_code);
    void InternalRunTuvx(
        void *tuvx,
        const int number_of_layers,
        const double solar_zenith_angle,
        const double earth_sun_distance,
        double *photolysis_rate_constants,
        double *heating_rates,
        int *error_code);

    void InternalGetTuvxVersion(char **version_ptr, int *version_length);
    void InternalFreeTuvxVersion(char *version_ptr, int version_length);

    void *InternalCreateTuvxFromConfig(const char *config_path, int config_path_length, int *error_code);
    void InternalRunTuvxFromConfig(void *tuvx, double *photolysis_rates, double *heating_rates, int *error_code);
    int InternalGetPhotolysisRateCount(void *tuvx, int *error_code);
    int InternalGetHeatingRateCount(void *tuvx, int *error_code);
    int InternalGetNumberOfLayers(void *tuvx, int *error_code);
    int InternalGetNumberOfSzaSteps(void *tuvx, int *error_code);
    void InternalGetPhotolysisRateNames(void *tuvx, char **names, int *error_code);
    void InternalGetHeatingRateNames(void *tuvx, char **names, int *error_code);

#ifdef __cplusplus
  }
#endif

}  // namespace musica