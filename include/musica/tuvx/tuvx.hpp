// Copyright (C) 2023-2025 National Center for Atmospheric Research
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
    /// @param grids Grid map from host application
    /// @param profiles Profile map from host application
    /// @param radiators Radiator map from host application
    /// @param error Error struct to indicate success or failure
    void Create(const char *config_path, GridMap *grids, ProfileMap *profiles, RadiatorMap *radiators, Error *error);

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

    /// @brief Returns the ordering of photolysis rate constants
    /// @param error Error struct to indicate success or failure
    /// @return Array of photolysis rate constant name-index pairs
    Mappings GetPhotolysisRateConstantsOrdering(Error *error);

    /// @brief Returns the ordering of heating rates
    /// @param error Error struct to indicate success or failure
    /// @return Array of heating rate name-index pairs
    Mappings GetHeatingRatesOrdering(Error *error);

    /// @brief Run the TUV-x photolysis calculator
    /// @param solar_zenith_angle Solar zenith angle [radians]
    /// @param earth_sun_distance Earth-Sun distance [AU]
    /// @param photolysis_rate_constants Photolysis rate constant for each layer and reaction [s^-1]
    /// @param heating_rates Heating rates for each layer and reaction [K/s]
    /// @param error Error struct to indicate success or failure
    void Run(
        const double solar_zenith_angle,
        const double earth_sun_distance,
        double *const photolysis_rate_constants,
        double *const heating_rates,
        Error *const error);

    ~TUVX();

   private:
    void *tuvx_;
    int number_of_layers_;
  };

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

#ifdef __cplusplus
  }
#endif

}  // namespace musica
