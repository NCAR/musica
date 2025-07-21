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
        double current_time,
        int current_step,
        const float* lat,
        const float* lon,
        const float* zc,
        const float* zl,
        const float* pressure,
        const float* temperature,
        const float* air_density,
        const float* number_density,
        const float* surface_area,
        const float* mass_density,
        const float* effective_radius,
        const float* mass_mixing_ratio,
        const float* bin_wet_radius,
        const float* bin_number_density,
        const float* bin_mass_mixing_ratio);

#ifdef __cplusplus
  }  // extern "C"
#endif
}  // namespace musica