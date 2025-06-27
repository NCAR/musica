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

    /// @brief Create an instance of tuvx from a configuration file only (simple interface)
    /// All parameters (solar zenith angle, Earth-Sun distance, atmospheric profiles, etc.)
    /// are read from the JSON configuration file, similar to the Fortran tuvx.F90 driver
    /// @param config_path Path to configuration file
    /// @param error Error struct to indicate success or failure
    void CreateFromConfigOnly(const char *config_path, Error *error);

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

    /// @brief Run the TUV-x photolysis calculator (simple interface)
    /// All parameters come from the JSON configuration file. Returns the computed
    /// photolysis rates and heating rates directly.
    /// @param photolysis_rate_constants Output array for photolysis rates [s^-1] (layer, reaction)
    /// @param heating_rates Output array for heating rates [K/s] (layer, reaction)
    /// @param error Error struct to indicate success or failure
    void RunFromConfig(double *const photolysis_rate_constants, double *const heating_rates, Error *const error);

    /// @brief Get the number of photolysis reactions
    /// @param error Error struct to indicate success or failure
    /// @return Number of photolysis reactions
    int GetPhotolysisRateCount(Error *error);

    /// @brief Get the number of heating rate types
    /// @param error Error struct to indicate success or failure
    /// @return Number of heating rate types
    int GetHeatingRateCount(Error *error);

    /// @brief Get the number of vertical layers
    /// @param error Error struct to indicate success or failure
    /// @return Number of vertical layers
    int GetNumberOfLayers(Error *error);

    /// @brief Get the number of solar zenith angle steps
    /// @param error Error struct to indicate success or failure
    /// @return Number of solar zenith angle steps
    int GetNumberOfSzaSteps(Error *error);

    /// @brief Get photolysis rate names (simple interface)
    /// @param error Error struct to indicate success or failure
    /// @return Vector of photolysis rate names
    std::vector<std::string> GetPhotolysisRateNames(Error *error);

    /// @brief Get heating rate names (simple interface)
    /// @param error Error struct to indicate success or failure
    /// @return Vector of heating rate names
    std::vector<std::string> GetHeatingRateNames(Error *error);

    ~TUVX();

   private:
    void *tuvx_;
    int number_of_layers_;
    bool is_config_only_mode_;
  };

}  // namespace musica
