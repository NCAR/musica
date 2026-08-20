// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/tuvx/radiation_field_updater.hpp>

namespace
{
  constexpr int ERROR_NONE = 0;
  constexpr const char* GetErrorMessage(int error_code)
  {
    switch (error_code)
    {
      case ERROR_NONE: return "No error";
      default: return "Unknown error";
    }
  }
}  // namespace

namespace musica
{

  // RadiationFieldUpdater external C API functions

  void DeleteRadiationFieldUpdater(RadiationFieldUpdater* updater, Error* error)
  {
    DeleteError(error);
    try
    {
      delete updater;
    }
    catch (const std::exception& e)
    {
      ToError(e, MUSICA_SEVERITY_ERROR, error);
      return;
    }
    NoError(error);
  }

  void UpdateRadiationField(
      RadiationFieldUpdater* updater,
      double* direct_actinic_flux,
      double* upward_actinic_flux,
      double* downward_actinic_flux,
      double* direct_irradiance,
      double* upward_irradiance,
      double* downward_irradiance,
      std::size_t num_vertical_interfaces,
      std::size_t num_wavelength_bins,
      Error* error)
  {
    DeleteError(error);
    updater->Update(
        direct_actinic_flux,
        upward_actinic_flux,
        downward_actinic_flux,
        direct_irradiance,
        upward_irradiance,
        downward_irradiance,
        num_vertical_interfaces,
        num_wavelength_bins,
        error);
  }

  // RadiationFieldUpdater class functions

  RadiationFieldUpdater::~RadiationFieldUpdater()
  {
    int error_code = 0;
    if (updater_ != nullptr)
      InternalDeleteRadiationFieldUpdater(updater_, &error_code);
    updater_ = nullptr;
  }

  void RadiationFieldUpdater::Update(
      double* direct_actinic_flux,
      double* upward_actinic_flux,
      double* downward_actinic_flux,
      double* direct_irradiance,
      double* upward_irradiance,
      double* downward_irradiance,
      std::size_t num_vertical_interfaces,
      std::size_t num_wavelength_bins,
      Error* error)
  {
    DeleteError(error);
    int error_code = 0;
    InternalUpdateRadiationField(
        updater_,
        direct_actinic_flux,
        upward_actinic_flux,
        downward_actinic_flux,
        direct_irradiance,
        upward_irradiance,
        downward_irradiance,
        num_vertical_interfaces,
        num_wavelength_bins,
        &error_code);
    if (error_code != 0)
    {
      ToError(MUSICA_ERROR_CATEGORY, error_code, GetErrorMessage(error_code), MUSICA_SEVERITY_ERROR, error);
      return;
    }
    NoError(error);
  }

}  // namespace musica
