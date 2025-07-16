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

    char *GetCarmaVersion();

    // for use by musica internally.
    void InternalGetCarmaVersion(char **version_ptr, int *version_length);
    void InternalFreeCarmaVersion(char *version_ptr, int version_length);

    // CARMA driver interface functions
    void InternalRunCarmaWithParameters(const CARMAParameters &params, int *rc);

#ifdef __cplusplus
  }  // extern "C"
#endif
}  // namespace musica