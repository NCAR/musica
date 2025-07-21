// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/carma/carma.hpp>

namespace musica
{
#ifdef __cplusplus
  extern "C"
  {
#endif
    // The external C API for CARMA
    // callable by wrappers in other languages

    char* GetCarmaVersion();

    // for use by musica internally.
    void InternalGetCarmaVersion(char** version_ptr, int* version_length);
    void InternalFreeCarmaVersion(char* version_ptr, int version_length);

    // CARMA driver interface functions
    void InternalRunCarma(const CARMAParameters& params, void* output, int* rc);

    // Transfer function called from Fortran
    void TransferCarmaOutputToCpp(
        void* c_output_ptr,
        int nz,
        int ny,
        int nx,
        int nelem,
        int ngroup,
        int nbin,
        int ngas,
        int nstep,
        const double* lat,
        const double* lon,
        const double* vertical_center,
        const double* vertical_levels,
        const double* pressure,
        const double* temperature,
        const double* air_density,
        const double* radiative_heating,
        const double* delta_temperature,
        const double* gas_mmr,
        const double* gas_saturation_liquid,
        const double* gas_saturation_ice,
        const double* gas_vapor_pressure_ice,
        const double* gas_vapor_pressure_liquid,
        const double* gas_weight_percent,
        const double* number_density,
        const double* surface_area,
        const double* mass_density,
        const double* effective_radius,
        const double* effective_radius_wet,
        const double* mean_radius,
        const double* nucleation_rate,
        const double* mass_mixing_ratio,
        const double* projected_area,
        const double* aspect_ratio,
        const double* vertical_mass_flux,
        const double* extinction,
        const double* optical_depth,
        const double* bin_wet_radius,
        const double* bin_number_density,
        const double* bin_density,
        const double* bin_mass_mixing_ratio,
        const double* bin_deposition_velocity,
        const double* group_radius,
        const double* group_mass,
        const double* group_volume,
        const double* group_radius_ratio,
        const double* group_aspect_ratio,
        const double* group_fractal_dimension);

#ifdef __cplusplus
  }  // extern "C"
#endif
}  // namespace musica