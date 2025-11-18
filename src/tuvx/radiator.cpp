// Copyright (C) 2023-2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/tuvx/radiator.hpp>

#include <cstring>
#include <filesystem>
#include <iostream>

namespace
{
  constexpr int ERROR_NONE = 0;
  constexpr int ERROR_RADIATOR_DIM_MISMATCH = 3201;
  constexpr int ERROR_UNALLOCATED_RADIATOR = 3202;
  constexpr int ERROR_UNALLOCATED_RADIATOR_UPDATER = 3203;
  constexpr const char *GetErrorMessage(int error_code)
  {
    switch (error_code)
    {
      case ERROR_NONE: return "No error";
      case ERROR_RADIATOR_DIM_MISMATCH: return "Radiator dimension mismatch";
      case ERROR_UNALLOCATED_RADIATOR: return "Radiator is unallocated";
      case ERROR_UNALLOCATED_RADIATOR_UPDATER: return "Radiator updater is unallocated";
      default: return "Unknown error";
    }
  }
}  // namespace

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

  const char *GetRadiatorName(Radiator *radiator, Error *error)
  {
    DeleteError(error);
    try
    {
      std::string name = radiator->GetName(error);
      if (error->code_ != ERROR_NONE)
        return nullptr;

      char *c_name = new char[name.size() + 1];
      std::strcpy(c_name, name.c_str());
      return c_name;
    }
    catch (const std::system_error &e)
    {
      ToError(e, error);
      return nullptr;
    }
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

  std::size_t GetRadiatorNumberOfHeightSections(Radiator *radiator, Error *error)
  {
    DeleteError(error);
    return radiator->GetNumberOfHeightSections(error);
  }

  std::size_t GetRadiatorNumberOfWavelengthSections(Radiator *radiator, Error *error)
  {
    DeleteError(error);
    return radiator->GetNumberOfWavelengthSections(error);
  }

  // Radiation class functions

  Radiator::Radiator(const char *radiator_name, Grid *height_grid, Grid *wavelength_grid, Error *error)
  {
    DeleteError(error);
    int error_code = ERROR_NONE;
    radiator_ = InternalCreateRadiator(
        radiator_name, strlen(radiator_name), height_grid->updater_, wavelength_grid->updater_, &error_code);
    if (error_code != ERROR_NONE)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    updater_ = InternalGetRadiatorUpdater(radiator_, &error_code);
    if (error_code != ERROR_NONE)
    {
      InternalDeleteRadiator(radiator_, &error_code);
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
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

  std::string Radiator::GetName(Error *error)
  {
    DeleteError(error);
    int error_code = ERROR_NONE;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_RADIATOR_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return "";
    }
    String name;
    InternalGetRadiatorName(updater_, &name, &error_code);
    if (error_code != ERROR_NONE)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return "";
    }
    std::string result(name.value_, name.size_);
    delete[] name.value_;
    NoError(error);
    return result;
  }

  void Radiator::SetOpticalDepths(
      double *optical_depths,
      std::size_t num_vertical_layers,
      std::size_t num_wavelength_bins,
      Error *error)
  {
    int error_code = ERROR_NONE;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_RADIATOR_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    InternalSetOpticalDepths(updater_, optical_depths, num_vertical_layers, num_wavelength_bins, &error_code);
    if (error_code != ERROR_NONE)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
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
    int error_code = ERROR_NONE;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_RADIATOR_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    InternalGetOpticalDepths(updater_, optical_depths, num_vertical_layers, num_wavelength_bins, &error_code);
    if (error_code != ERROR_NONE)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
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
    int error_code = ERROR_NONE;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_RADIATOR_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    InternalSetSingleScatteringAlbedos(
        updater_, single_scattering_albedos, num_vertical_layers, num_wavelength_bins, &error_code);
    if (error_code != ERROR_NONE)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
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
    int error_code = ERROR_NONE;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_RADIATOR_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    InternalGetSingleScatteringAlbedos(
        updater_, single_scattering_albedos, num_vertical_layers, num_wavelength_bins, &error_code);
    if (error_code != ERROR_NONE)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
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
    int error_code = ERROR_NONE;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_RADIATOR_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    InternalSetAsymmetryFactors(
        updater_, asymmetry_factors, num_vertical_layers, num_wavelength_bins, num_streams, &error_code);
    if (error_code != ERROR_NONE)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
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
    int error_code = ERROR_NONE;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_RADIATOR_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    InternalGetAsymmetryFactors(
        updater_, asymmetry_factors, num_vertical_layers, num_wavelength_bins, num_streams, &error_code);
    if (error_code != ERROR_NONE)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return;
    }
    NoError(error);
  }

  std::size_t Radiator::GetNumberOfHeightSections(Error *error)
  {
    DeleteError(error);
    int error_code = ERROR_NONE;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_RADIATOR_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return 0;
    }
    std::size_t num_height_sections = InternalGetRadiatorNumberOfHeightSections(updater_, &error_code);
    if (error_code != ERROR_NONE)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return 0;
    }
    NoError(error);
    return num_height_sections;
  }

  std::size_t Radiator::GetNumberOfWavelengthSections(Error *error)
  {
    DeleteError(error);
    int error_code = ERROR_NONE;
    if (updater_ == nullptr)
    {
      error_code = ERROR_UNALLOCATED_RADIATOR_UPDATER;
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return 0;
    }
    std::size_t num_wavelength_sections = InternalGetRadiatorNumberOfWavelengthSections(updater_, &error_code);
    if (error_code != ERROR_NONE)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), error);
      return 0;
    }
    NoError(error);
    return num_wavelength_sections;
  }

}  // namespace musica