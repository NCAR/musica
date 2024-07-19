// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/tuvx/grid.hpp>
#include <musica/util.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace musica
{
  class RadiatorMap;
  class Profile;

  /// @brief A radiator struct used to access radiator information in tuvx
  struct Radiator
  {
    /// @brief Creates a radiator instance
    /// @param radiator_name The name of the radiator
    /// @param height_grid The height grid
    /// @param wavelength_grid The wavelength grid
    /// @param error The error struct to indicate success or failure
    Radiator(const char *radiator_name, Grid *height_grid, Grid *wavelength_grid, Error *error);

    ~Radiator();

    /// @brief Sets the optical_depths
    /// @param optical_depths The 2 dimensional optical_depths
    /// @param num_vertical_layers The number of vertical layers
    /// @param num_wavelength_bins The number of wavelength layers
    /// @param error The error struct to indicate success or failure
    void
    SetOpticalDepths(double *optical_depths, std::size_t num_vertical_layers, std::size_t num_wavelength_bins, Error *error);

    /// @brief Gets the optical_depths
    /// @param optical_depths The 2 dimensional optical_depths
    /// @param num_vertical_layers The number of vertical layers
    /// @param num_wavelength_bins The number of wavelength layers
    /// @param error The error struct to indicate success or failure
    void
    GetOpticalDepths(double *optical_depths, std::size_t num_vertical_layers, std::size_t num_wavelength_bins, Error *error);

    /// @brief Sets the values of the single scattering albedos
    /// @param single_scattering_albedos The 2 dimensional single scattering albedos values
    /// @param num_vertical_layers The number of vertical layers
    /// @param num_wavelength_bins The number of wavelength bins
    /// @param error The error struct to indicate success or failure
    void SetSingleScatteringAlbedos(
        double *single_scattering_albedos,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        Error *error);

    /// @brief Gets the values of the single scattering albedos
    /// @param single_scattering_albedos The 2 dimensional single scattering albedos values
    /// @param num_vertical_layers The number of vertical layers
    /// @param num_wavelength_bins The number of wavelength bins
    /// @param error The error struct to indicate success or failure
    void GetSingleScatteringAlbedos(
        double *single_scattering_albedos,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        Error *error);

    /// @brief Sets the values of the asymmetry factors
    /// @param asymmetry_factor The asymmetery factors values to set for the radiator
    /// @param num_vertical_layers The number of vertical layers
    /// @param num_wavelength_bins The number of wavelength bins
    /// @param num_streams The number of streams
    /// @param error The error struct to indicate success or failure
    void SetAsymmetryFactors(
        double *asymmetry_factor,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        std::size_t num_streams,
        Error *error);

    /// @brief Gets the values of the asymmetry factors
    /// @param asymmetry_factor The asymmetery factors values to set for the radiator
    /// @param num_vertical_layers The number of vertical layers
    /// @param num_wavelength_bins The number of wavelength bins
    /// @param num_streams The number of streams
    /// @param error The error struct to indicate success or failure
    void GetAsymmetryFactors(
        double *asymmetry_factor,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        std::size_t num_streams,
        Error *error);

   private:
    void *radiator_;  // A valid pointer to a radiator instance indicates ownership by this wrapper
    void *updater_;

    // friend class RadiatorMap;
    // friend class Profile;

    /// @brief Wraps an existing radiator instance. Used by RadiatorMap
    /// @param updater The updater for the radiator
    Radiator(void *updater)
        : radiator_(nullptr),
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

    /// @brief Creates a TUV-x radiator instance
    /// @param radiator_name The name of the radiator
    /// @param height_grid The height grid
    /// @param wavelength_grid The wavelength grid
    /// @param error The error struct to indicate success or failure
    Radiator *CreateRadiator(const char *radiator_name, Grid *height_grid, Grid *wavelength_grid, Error *error);

    /// @brief Deletes a TUV-x radiator instance
    /// @param radiator The radiator to delete
    /// @param error The error struct to indicate success or failure
    void DeleteRadiator(Radiator *radiator, Error *error);

    /// @brief Sets the values of the optical depths of the radiator
    /// @param radiator The radiator to get the optical depths
    /// @param optical_depths The optical depths values to get for the radiator
    /// @param num_vertical_layers The number of vertical layers
    /// @param num_wavelength_bins The number of wavelength bins
    /// @param error The error struct to indicate success or failure
    void SetRadiatorOpticalDepths(
        Radiator *radiator,
        double *optical_depths,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        Error *error);

    /// @brief Gets the values of the optical depths of the radiator
    /// @param radiator The radiator to set the optical depths
    /// @param optical_depths The optical depths values to set for the radiator
    /// @param num_vertical_layers The number of vertical layers
    /// @param num_wavelength_bins The number of wavelength bins
    /// @param error The error struct to indicate success or failure
    void GetRadiatorOpticalDepths(
        Radiator *radiator,
        double *optical_depths,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        Error *error);

    /// @brief Sets the values of the single scattering albedos
    /// @param radiator The radiator to set the single scattering albedos of
    /// @param single_scattering_albedos The single scattering albedos values to set for the radiator
    /// @param num_vertical_layers The number of vertical layers
    /// @param num_wavelength_bins The number of wavelength bins
    /// @param error The error struct to indicate success or failure
    void SetRadiatorSingleScatteringAlbedos(
        Radiator *radiator,
        double *single_scattering_albedos,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        Error *error);

    /// @brief Gets the values of the single scattering albedos
    /// @param radiator The radiator to get the single scattering albedos of
    /// @param single_scattering_albedos The single scattering albedos values to get for the radiator
    /// @param num_vertical_layers The number of vertical layers
    /// @param num_wavelength_bins The number of wavelength bins
    /// @param error The error struct to indicate success or failure
    void GetRadiatorSingleScatteringAlbedos(
        Radiator *radiator,
        double *single_scattering_albedos,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        Error *error);

    /// @brief Sets the values of the asymmetry factors
    /// @param asymmetry_factor The 3 dimensional asymmetery factors values to set for the radiator
    /// @param num_vertical_layers The number of vertical layers
    /// @param num_wavelength_bins The number of wavelength bins
    /// @param num_streams The number of streams
    /// @param error The error struct to indicate success or failure
    void SetRadiatorAsymmetryFactors(
        Radiator *radiator,
        double *asymmetry_factor,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        std::size_t num_streams,
        Error *error);

    /// @brief Gets the values of the asymmetry factors
    /// @param asymmetry_factor The 3 dimensional asymmetery factors values to get for the radiator
    /// @param num_vertical_layers The number of vertical layers
    /// @param num_wavelength_bins The number of wavelength bins
    /// @param num_streams The number of streams
    /// @param error The error struct to indicate success or failure
    void GetRadiatorAsymmetryFactors(
        double *asymmetry_factor,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        std::size_t num_streams,
        Error *error);

    // INTERNAL USE. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void *InternalCreateRadiator(
        const char *radiator_name,
        std::size_t radiator_name_length,
        Grid *hegiht_grid,
        Grid *wavelength_grid,
        int *error_code);
    void InternalDeleteRadiator(void *radiator, int *error_code);
    void *InternalGetRadiatorUpdater(void *radiator, int *error_code);
    void InternalDeleteRadiatorUpdater(void *updater, int *error_code);
    void InternalSetOpticalDepths(
        void *radiator,
        double *optical_depths,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        int *error_code);
    void InternalGetOpticalDepths(
        void *radiator,
        double *optical_depths,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        int *error_code);
    void InternalSetSingleScatteringAlbedos(
        void *radiator,
        double *single_scattering_albedos,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        int *error_code);
    void InternalGetsingleScatteringAlbedos(
        void *radiator,
        double *single_scattering_albedos,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        int *error_code);
    void InternalSetAsymmetryFactors(
        void *radiator,
        double *asymmetry_factors,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        std::size_t num_streams,
        int *error_code);
    void InternalGetAsymmetryFactors(
        void *radiator,
        double *asymmetry_factors,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        std::size_t num_streams,
        int *error_code);

#ifdef __cplusplus
  }
#endif

}  // namespace musica
