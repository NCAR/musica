// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/tuvx/grid.hpp>
#include <musica/util.hpp>

#include <cstddef>

namespace musica
{
  class RadiatorMap;

  /// @brief Radiator class used to access radiator information in tuvx
  class Radiator
  {
   public:
    /// @brief Creates radiator
    /// @param radiator_name Radiator name
    /// @param height_grid Height grid
    /// @param wavelength_grid Wavelength grid
    /// @param error Error to indicate success or failure
    Radiator(const char *radiator_name, Grid *height_grid, Grid *wavelength_grid, Error *error);

    ~Radiator();

    /// @brief Sets optical depth values
    /// @param optical_depths 2D array of optical depth values
    /// @param num_vertical_layers Number of vertical layers
    /// @param num_wavelength_bins Number of wavelength bins
    /// @param error Error to indicate success or failure
    void
    SetOpticalDepths(double *optical_depths, std::size_t num_vertical_layers, std::size_t num_wavelength_bins, Error *error);

    /// @brief Gets optical depth values
    /// @param optical_depths 2D array of optical depth values
    /// @param num_vertical_layers Number of vertical layers
    /// @param num_wavelength_bins Number of wavelength bins
    /// @param error Error to indicate success or failure
    void
    GetOpticalDepths(double *optical_depths, std::size_t num_vertical_layers, std::size_t num_wavelength_bins, Error *error);

    /// @brief Sets single scattering albedos values
    /// @param single_scattering_albedos 2D array of single scattering albedos values
    /// @param num_vertical_layers Number of vertical layers
    /// @param num_wavelength_bins Number of wavelength bins
    /// @param error Error to indicate success or failure
    void SetSingleScatteringAlbedos(
        double *single_scattering_albedos,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        Error *error);

    /// @brief Gets single scattering albedos values
    /// @param single_scattering_albedos 2D array of single scattering albedos values
    /// @param num_vertical_layers Number of vertical layers
    /// @param num_wavelength_bins Number of wavelength bins
    /// @param error Error to indicate success or failure
    void GetSingleScatteringAlbedos(
        double *single_scattering_albedos,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        Error *error);

    /// @brief Sets asymmetry factor values
    /// @param asymmetry_factor 3D array of asymmetery factor values
    /// @param num_vertical_layers Number of vertical layers
    /// @param num_wavelength_bins Number of wavelength bins
    /// @param num_streams Number of streams
    /// @param error Error to indicate success or failure
    void SetAsymmetryFactors(
        double *asymmetry_factor,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        std::size_t num_streams,
        Error *error);

    /// @brief Gets asymmetry factor values
    /// @param asymmetry_factor 3D array of asymmetery factor values
    /// @param num_vertical_layers Number of vertical layers
    /// @param num_wavelength_bins Number of wavelength bins
    /// @param num_streams Number of streams
    /// @param error Error to indicate success or failure
    void GetAsymmetryFactors(
        double *asymmetry_factor,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        std::size_t num_streams,
        Error *error);

   private:
    void *radiator_;  // A valid pointer to a radiator instance indicates ownership by this wrapper
    void *updater_;

    friend class RadiatorMap;

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

    /// @brief Creates radiator
    /// @param radiator_name Radiator name
    /// @param height_grid Height grid
    /// @param wavelength_grid Wavelength grid
    /// @param error Error to indicate success or failure
    Radiator *CreateRadiator(const char *radiator_name, Grid *height_grid, Grid *wavelength_grid, Error *error);

    /// @brief Deletes radiator
    /// @param radiator Radiator
    /// @param error Error to indicate success or failure
    void DeleteRadiator(Radiator *radiator, Error *error);

    /// @brief Sets optical depth values
    /// @param radiator Radiator
    /// @param optical_depths 2D array of optical depth values
    /// @param num_vertical_layers Number of vertical layers
    /// @param num_wavelength_bins Number of wavelength bins
    /// @param error Error to indicate success or failure
    void SetRadiatorOpticalDepths(
        Radiator *radiator,
        double *optical_depths,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        Error *error);

    /// @brief Gets optical depth values
    /// @param radiator Radiator
    /// @param optical_depths 2D array of optical depth values
    /// @param num_vertical_layers Number of vertical layers
    /// @param num_wavelength_bins Number of wavelength bins
    /// @param error Error to indicate success or failure
    void GetRadiatorOpticalDepths(
        Radiator *radiator,
        double *optical_depths,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        Error *error);

    /// @brief Sets single scattering albedos values
    /// @param radiator Radiator
    /// @param single_scattering_albedos 2D array of single scattering albedos values
    /// @param num_vertical_layers Number of vertical layers
    /// @param num_wavelength_bins Number of wavelength bins
    /// @param error Error to indicate success or failure
    void SetRadiatorSingleScatteringAlbedos(
        Radiator *radiator,
        double *single_scattering_albedos,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        Error *error);

    /// @brief Gets single scattering albedos values
    /// @param radiator Radiator
    /// @param single_scattering_albedos 2D array of single scattering albedos values
    /// @param num_vertical_layers Number of vertical layers
    /// @param num_wavelength_bins Number of wavelength bins
    /// @param error Error to indicate success or failure
    void GetRadiatorSingleScatteringAlbedos(
        Radiator *radiator,
        double *single_scattering_albedos,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        Error *error);

    /// @brief Sets asymmetry factor values
    /// @param radiator Radiator
    /// @param asymmetry_factor 3D array of asymmetery factor values
    /// @param num_vertical_layers Number of vertical layers
    /// @param num_wavelength_bins Number of wavelength bins
    /// @param num_streams Number of streams
    /// @param error Error to indicate success or failure
    void SetRadiatorAsymmetryFactors(
        Radiator *radiator,
        double *asymmetry_factor,
        std::size_t num_vertical_layers,
        std::size_t num_wavelength_bins,
        std::size_t num_streams,
        Error *error);

    /// @brief Gets asymmetry factor values
    /// @param radiator Radiator
    /// @param asymmetry_factor 3D array of asymmetery factor values
    /// @param num_vertical_layers Number of vertical layers
    /// @param num_wavelength_bins Number of wavelength bins
    /// @param num_streams Number of streams
    /// @param error Error to indicate success or failure
    void GetRadiatorAsymmetryFactors(
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
        void *height_grid,
        void *wavelength_grid,
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
    void InternalGetSingleScatteringAlbedos(
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