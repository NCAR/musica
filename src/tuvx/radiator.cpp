// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/tuvx/radiator.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

namespace musica
{

  // Radiator external C API functions

  Radiator *CreateRadiator(const char *radiator_name, Grid *height_grid, Grid *wavelength_grid, Error *error)
  {
    DeleteError(error);
    return new Radiator(radiator_name, height_grid, wavelength_grid, error);
  }

  void DeleteRadiator(Radiator *radiator, Error *error)
  {
    DeleteError(error);
    try
    {
      delete radiator;
    }
    catch (const std::system_error &e)
    {
      ToError(e, error);
      return;
    }
    NoError(error);
  }

  void SetRadiatorOpticalDepths(
      Radiator *radiator,
      double *optical_depths,
      std::size_t num_vertical_layers,
      std::size_t num_wavelength_bins,
      Error *error)
  {
    DeleteError(error);
    radiator->SetOpticalDepths(optical_depths, num_vertical_layers, num_wavelength_bins, error);
  }

  void GetRadiatorOpticalDepths(
      Radiator *radiator,
      double *optical_depths,
      std::size_t num_vertical_layers,
      std::size_t num_wavelength_bins,
      Error *error)
  {
    DeleteError(error);
    radiator->GetOpticalDepths(optical_depths, num_vertical_layers, num_wavelength_bins, error);
  }

  void SetRadiatorSingleScatteringAlbedos(
      Radiator *radiator,
      double *single_scattering_albedos,
      std::size_t num_vertical_layers,
      std::size_t num_wavelength_bins,
      Error *error)
  {
    DeleteError(error);
    radiator->SetSingleScatteringAlbedos(single_scattering_albedos, num_vertical_layers, num_wavelength_bins, error);
  }

  void GetRadiatorSingleScatteringAlbedos(
      Radiator *radiator,
      double *single_scattering_albedos,
      std::size_t num_vertical_layers,
      std::size_t num_wavelength_bins,
      Error *error)
  {
    DeleteError(error);
    radiator->GetSingleScatteringAlbedos(single_scattering_albedos, num_vertical_layers, num_wavelength_bins, error);
  }

  void SetRadiatorAsymmetryFactors(
      Radiator *radiator,
      double *asymmetry_factors,
      std::size_t num_vertical_layers,
      std::size_t num_wavelength_bins,
      std::size_t num_streams,
      Error *error)
  {
    DeleteError(error);
    radiator->SetAsymmetryFactors(asymmetry_factors, num_vertical_layers, num_wavelength_bins, num_streams, error);
  }

  void GetRadiatorAsymmetryFactors(
      Radiator *radiator,
      double *asymmetry_factors,
      std::size_t num_vertical_layers,
      std::size_t num_wavelength_bins,
      std::size_t num_streams,
      Error *error)
  {
    DeleteError(error);
    radiator->GetAsymmetryFactors(asymmetry_factors, num_vertical_layers, num_wavelength_bins, num_streams, error);
  }

  // Radiation class functions

  Radiator::Radiator(const char *radiator_name, Grid *height_grid, Grid *wavelength_grid, Error *error)
  {
    int error_code = 0;
    radiator_ = InternalCreateRadiator(
        radiator_name, strlen(radiator_name), height_grid->updater_, wavelength_grid->updater_, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to create radiator", error);
      return;
    }
    updater_ = InternalGetRadiatorUpdater(radiator_, &error_code);
    if (error_code != 0)
    {
      InternalDeleteRadiator(radiator_, &error_code);
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to get radiator updater", error);
      return;
    }
    NoError(error);
  }

  Radiator::~Radiator()
  {
    int error_code = 0;
    if (radiator_ != nullptr)
      InternalDeleteRadiator(radiator_, &error_code);
    if (updater_ != nullptr)
      InternalDeleteRadiatorUpdater(updater_, &error_code);
    radiator_ = nullptr;
    updater_ = nullptr;
  }

  void Radiator::SetOpticalDepths(
      double *optical_depths,
      std::size_t num_vertical_layers,
      std::size_t num_wavelength_bins,
      Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Radiator is not updatable", error);
      return;
    }
    InternalSetOpticalDepths(updater_, optical_depths, num_vertical_layers, num_wavelength_bins, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to set optical depths", error);
      return;
    }
    NoError(error);
  }

  void Radiator::GetOpticalDepths(
      double *optical_depths,
      std::size_t num_vertical_layers,
      std::size_t num_wavelength_bins,
      Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Radiator is not updatable", error);
      return;
    }
    InternalGetOpticalDepths(updater_, optical_depths, num_vertical_layers, num_wavelength_bins, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to get optical depths", error);
      return;
    }
    NoError(error);
  }

  void Radiator::SetSingleScatteringAlbedos(
      double *single_scattering_albedos,
      std::size_t num_vertical_layers,
      std::size_t num_wavelength_bins,
      Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Radiator is not updatable", error);
      return;
    }
    InternalSetSingleScatteringAlbedos(
        updater_, single_scattering_albedos, num_vertical_layers, num_wavelength_bins, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to set single scattering albedos", error);
      return;
    }
    NoError(error);
  }

  void Radiator::GetSingleScatteringAlbedos(
      double *single_scattering_albedos,
      std::size_t num_vertical_layers,
      std::size_t num_wavelength_bins,
      Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Radiator is not updatable", error);
      return;
    }
    InternalGetSingleScatteringAlbedos(
        updater_, single_scattering_albedos, num_vertical_layers, num_wavelength_bins, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to get single scattering albedos", error);
      return;
    }
    NoError(error);
  }

  void Radiator::SetAsymmetryFactors(
      double *asymmetry_factors,
      std::size_t num_vertical_layers,
      std::size_t num_wavelength_bins,
      std::size_t num_streams,
      Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Radiator is not updatable", error);
      return;
    }
    InternalSetAsymmetryFactors(
        updater_, asymmetry_factors, num_vertical_layers, num_wavelength_bins, num_streams, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to set asymmetry factors", error);
      return;
    }
    NoError(error);
  }

  void Radiator::GetAsymmetryFactors(
      double *asymmetry_factors,
      std::size_t num_vertical_layers,
      std::size_t num_wavelength_bins,
      std::size_t num_streams,
      Error *error)
  {
    int error_code = 0;
    if (updater_ == nullptr)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Radiator is not updatable", error);
      return;
    }
    InternalGetAsymmetryFactors(
        updater_, asymmetry_factors, num_vertical_layers, num_wavelength_bins, num_streams, &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, 1, "Failed to get asymmetry factors", error);
      return;
    }
    NoError(error);
  }

}  // namespace musica