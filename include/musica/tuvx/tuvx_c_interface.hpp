// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
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
    TUVX* CreateTuvx(const char* config_path, GridMap* grids, ProfileMap* profiles, RadiatorMap* radiators, Error* error);

    /// @brief Creates a TUVX instance by passing a configuration string and host-defined grids, profiles, and radiators
    /// @param config_string JSON/YAML configuration string
    /// @param grids Grid map from host application
    /// @param profiles Profile map from host application
    /// @param radiators Radiator map from host application
    /// @param error Error struct to indicate success or failure
    TUVX* CreateTuvxFromConfigString(
        const char* config_string,
        GridMap* grids,
        ProfileMap* profiles,
        RadiatorMap* radiators,
        Error* error);

    /// @brief Deletes a TUVX instance
    /// @param tuvx Pointer to TUVX instance
    /// @param error Error struct to indicate success or failure
    void DeleteTuvx(const TUVX* tuvx, Error* error);

    /// @brief Returns the set of grids used by TUVX
    /// @param tuvx Pointer to TUVX instance
    /// @param error Error struct to indicate success or failure
    /// @return Grid map
    GridMap* GetGridMap(TUVX* tuvx, Error* error);

    /// @brief Returns the set of profiles used by TUVX
    /// @param tuvx Pointer to TUVX instance
    /// @param error Error struct to indicate success or failure
    /// @return Profile map
    ProfileMap* GetProfileMap(TUVX* tuvx, Error* error);

    /// @brief Returns the set of radiators used by TUVX
    /// @param tuvx Pointer to TUVX instance
    /// @param error Error struct to indicate success or failure
    /// @return Radiator map
    RadiatorMap* GetRadiatorMap(TUVX* tuvx, Error* error);

    /// @brief Returns the ordering photolysis rate constants
    /// @param tuvx Pointer to TUVX instance
    /// @param mappings Array of photolysis rate constant name-index pairs [output]
    /// @param error Error struct to indicate success or failure
    void GetPhotolysisRateConstantsOrdering(TUVX* tuvx, Mappings* mappings, Error* error);

    /// @brief Returns the ordering of heating rates
    /// @param tuvx Pointer to TUVX instance
    /// @param mappings Array of heating rate name-index pairs [output]
    /// @param error Error struct to indicate success or failure
    void GetHeatingRatesOrdering(TUVX* tuvx, Mappings* mappings, Error* error);

    /// @brief Returns the ordering of dose rates
    /// @param tuvx Pointer to TUVX instance
    /// @param mappings Array of dose rate name-index pairs [output]
    /// @param error Error struct to indicate success or failure
    void GetDoseRatesOrdering(TUVX* tuvx, Mappings* mappings, Error* error);

    /// @brief Run the TUV-x photolysis calculator
    /// @param tuvx Pointer to TUVX instance
    /// @param solar_zenith_angle Solar zenith angle [radians]
    /// @param earth_sun_distance Earth-Sun distance [AU]
    /// @param photolysis_rate_constants Photolysis rate constant [s^-1] (reaction, vertical edge)
    /// @param heating_rates Heating rates [K/s] (heating_reaction, vertical edge)
    /// @param dose_rates Dose rates [W/m^2] (dose_rate type, vertical edge)
    /// @param actinic_flux Actinic flux [photons cm^-2 s^-1 nm^-1] (wavelength, vertical edge, direct/upwelling/downwelling)
    /// @param spectral_irradiance Spectral irradiance [W m^-2 nm^-1] (wavelength, vertical edge,
    /// direct/upwelling/downwelling)
    /// @param error Error struct to indicate success or failure
    void RunTuvx(
        TUVX* tuvx,
        const double solar_zenith_angle,
        const double earth_sun_distance,
        double* const photolysis_rate_constants,
        double* const heating_rates,
        double* const dose_rates,
        double* const actinic_flux,
        double* const spectral_irradiance,
        Error* const error);

    /// @brief Get the TUVX version
    /// @param tuvx_version TUVX version [output]
    void TuvxVersion(String* tuvx_version);

    // for use by musica internally. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void* InternalCreateTuvx(
        const char* config_path,
        std::size_t config_path_length,
        void* grid_map,
        void* profile_map,
        void* radiator_map,
        int* number_of_height_midpoints,
        int* number_of_wavelength_midpoints,
        int* error_code);
    void* InternalCreateTuvxFromConfigString(
        const char* config_string,
        std::size_t config_string_length,
        void* grid_map,
        void* profile_map,
        void* radiator_map,
        int* number_of_height_midpoints,
        int* number_of_wavelength_midpoints,
        int* error_code);
    void InternalDeleteTuvx(void* tuvx, int* error_code);
    void* InternalGetGridMap(void* tuvx, int* error_code);
    void* InternalGetProfileMap(void* tuvx, int* error_code);
    void* InternalGetRadiatorMap(void* tuvx, int* error_code);
    void InternalGetPhotolysisRateConstantsOrdering(void* tuvx, Mappings* mappings, int* error_code);
    void InternalGetHeatingRatesOrdering(void* tuvx, Mappings* mappings, int* error_code);
    void InternalGetDoseRatesOrdering(void* tuvx, Mappings* mappings, int* error_code);
    void InternalRunTuvx(
        void* tuvx,
        const int number_of_height_midpoints,
        const int number_of_wavelength_midpoints,
        const double solar_zenith_angle,
        const double earth_sun_distance,
        double* photolysis_rate_constants,
        double* heating_rates,
        double* dose_rates,
        double* actinic_flux,
        double* spectral_irradiance,
        int* error_code);

    void InternalGetTuvxVersion(char** version_ptr, int* version_length);
    void InternalFreeTuvxVersion(char* version_ptr, int version_length);
    int InternalGetPhotolysisRateConstantCount(void* tuvx, int* error_code);
    int InternalGetHeatingRateCount(void* tuvx, int* error_code);
    int InternalGetDoseRateCount(void* tuvx, int* error_code);
    int InternalGetNumberOfHeightMidpoints(void* tuvx, int* error_code);
    int InternalGetNumberOfWavelengthMidpoints(void* tuvx, int* error_code);

#ifdef __cplusplus
  }
#endif

}  // namespace musica