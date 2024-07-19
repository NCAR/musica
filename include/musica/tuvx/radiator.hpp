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

  /// @brief A grid struct used to access grid information in tuvx
  struct Radiator
  {
    /// @brief Creates a grid instance
    /// @param grid_name The name of the grid
    /// @param units The units of the grid
    /// @param num_sections The number of sections in the grid
    /// @param error The error struct to indicate success or failure
    Radiator(const char *grid_name, const char *units, std::size_t num_sections, Error *error);

    ~Radiator();

    /// @brief Set the edges of the grid
    /// @param edges The edges of the grid
    /// @param num_edges the number of edges
    /// @param error the error struct to indicate success or failure
    void SetEdges(double edges[], std::size_t num_edges, Error *error);

    /// @brief Get the edges of the grid
    /// @param edges The edges of the grid
    /// @param num_edges the number of edges
    /// @param error the error struct to indicate success or failure
    void GetEdges(double edges[], std::size_t num_edges, Error *error);

    /// @brief Set the midpoints of the grid
    /// @param midpoints The midpoints of the grid
    /// @param num_midpoints the number of midpoints
    /// @param error the error struct to indicate success or failure
    void SetMidpoints(double midpoints[], std::size_t num_midpoints, Error *error);

    /// @brief Get the midpoints of the grid
    /// @param midpoints The midpoints of the grid
    /// @param num_midpoints the number of midpoints
    /// @param error the error struct to indicate success or failure
    void GetMidpoints(double midpoints[], std::size_t num_midpoints, Error *error);

   private:
    void *grid_;  // A valid pointer to a grid instance indicates ownership by this wrapper
    void *updater_;

    friend class RadiatorMap;
    friend class Profile;

    /// @brief Wraps an existing grid instance. Used by RadiatorMap
    /// @param updater The updater for the grid
    Radiator(void *updater)
        : grid_(nullptr),
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
    void SetOpticalDepths(
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
    void GetOpticalDepths(
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
    void SetSingleScatteringAlbedos(
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
    void GetSingleScatteringAlbedos(
        Radiator *radiator,
        double *single_scattering_albedos,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        Error *error);

    /// @brief Sets the values of the asymmetry factors
    /// @param radiator The radiator to set the asymmetery factors of
    /// @param single_scattering_albedos The asymmetery factors values to set for the radiator
    /// @param num_vertical_layers The number of vertical layers
    /// @param num_wavelength_bins The number of wavelength bins
    /// @param num_streams The number of streams
    /// @param error The error struct to indicate success or failure
    void SetAsymmetryFactors(
        Radiator *radiator,
        double *asymmetry_factor,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        std::size_t num_streams,
        Error *error);

    /// @brief Gets the values of the asymmetry factors
    /// @param radiator The radiator to get the asymmetery factors of
    /// @param single_scattering_albedos The asymmetery factors values to get for the radiator
    /// @param num_vertical_layers The number of vertical layers
    /// @param num_wavelength_bins The number of wavelength bins
    /// @param num_streams The number of streams
    /// @param error The error struct to indicate success or failure
    void GetAsymmetryFactors(
        Radiator *radiator,
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
