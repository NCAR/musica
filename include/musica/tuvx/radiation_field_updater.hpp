// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/utils/util.hpp>

#include <cstddef>

namespace musica
{
  class TUVX;

  /// @brief Lets a host application push a radiation field into TUV-x's "from host" solver
  ///
  /// Obtained only via TUVX::GetRadiationFieldUpdater; never constructed directly. The
  /// values passed to Update are dimensionless and must NOT include the extraterrestrial
  /// flux profile or the Earth-Sun distance factor -- TUV-x applies both downstream. Call
  /// Update before every TUVX::Run: TUV-x does not raise an error if a Run is missed, it
  /// silently reuses the last field set (or an all-zero field, if Update was never called).
  class RadiationFieldUpdater
  {
   public:
    ~RadiationFieldUpdater();

    /// @brief Sets the radiation field TUV-x will use for the next Run
    /// @param direct_actinic_flux Direct component of the actinic flux (required), shape
    /// (num_vertical_interfaces, num_wavelength_bins), interface 0 is the lowest altitude
    /// @param upward_actinic_flux Diffuse upwelling component of the actinic flux (required), same shape
    /// @param downward_actinic_flux Diffuse downwelling component of the actinic flux (required), same shape
    /// @param direct_irradiance Direct component of the irradiance (optional; pass nullptr to omit).
    /// Used only for dose rates; an omitted component is treated as all zeros for this call.
    /// @param upward_irradiance Diffuse upwelling component of the irradiance (optional; see above)
    /// @param downward_irradiance Diffuse downwelling component of the irradiance (optional; see above)
    /// @param num_vertical_interfaces Number of height grid cells + 1
    /// @param num_wavelength_bins Number of wavelength grid cells
    /// @param error Error struct to indicate success or failure
    void Update(
        double* direct_actinic_flux,
        double* upward_actinic_flux,
        double* downward_actinic_flux,
        double* direct_irradiance,
        double* upward_irradiance,
        double* downward_irradiance,
        std::size_t num_vertical_interfaces,
        std::size_t num_wavelength_bins,
        Error* error);

   private:
    void* updater_;

    friend class TUVX;

    /// @brief Wraps an existing radiation field updater instance
    /// @param updater The updater, owned by this wrapper from this point on
    explicit RadiationFieldUpdater(void* updater)
        : updater_(updater)
    {
    }
  };

#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for the radiation field updater
    // callable by wrappers in other languages

    /// @brief Deletes a radiation field updater instance
    /// @param updater The updater to delete
    /// @param error Error struct to indicate success or failure
    void DeleteRadiationFieldUpdater(RadiationFieldUpdater* updater, Error* error);

    /// @brief Sets the radiation field TUV-x will use for the next Run
    /// @param updater The updater to set the field on
    /// @param direct_actinic_flux Direct component of the actinic flux (required)
    /// @param upward_actinic_flux Diffuse upwelling component of the actinic flux (required)
    /// @param downward_actinic_flux Diffuse downwelling component of the actinic flux (required)
    /// @param direct_irradiance Direct component of the irradiance (optional; pass nullptr to omit)
    /// @param upward_irradiance Diffuse upwelling component of the irradiance (optional; pass nullptr to omit)
    /// @param downward_irradiance Diffuse downwelling component of the irradiance (optional; pass nullptr to omit)
    /// @param num_vertical_interfaces Number of height grid cells + 1
    /// @param num_wavelength_bins Number of wavelength grid cells
    /// @param error Error struct to indicate success or failure
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
        Error* error);

    // INTERNAL USE. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    //
    // A null pointer for one of the three optional irradiance arrays is passed straight
    // through as a disassociated C pointer. The Fortran bridge checks each with
    // c_associated and passes it to update()'s matching optional, non-pointer,
    // assumed-shape dummy argument only when associated -- Fortran treats a disassociated
    // pointer actual argument matched to such a dummy as an absent optional argument, the
    // same way InternalRunTuvx already does for its optional dose_rates argument.
    void InternalDeleteRadiationFieldUpdater(void* updater, int* error_code);
    void InternalUpdateRadiationField(
        void* updater,
        double* direct_actinic_flux,
        double* upward_actinic_flux,
        double* downward_actinic_flux,
        double* direct_irradiance,
        double* upward_irradiance,
        double* downward_irradiance,
        std::size_t num_vertical_interfaces,
        std::size_t num_wavelength_bins,
        int* error_code);

#ifdef __cplusplus
  }
#endif

}  // namespace musica
