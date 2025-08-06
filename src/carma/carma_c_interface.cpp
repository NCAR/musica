// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
#include <musica/carma/carma.hpp>
#include <musica/carma/carma_c_interface.hpp>

#include <cstring>
#include <iostream>
#include <vector>

namespace musica
{
#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for CARMA
    // callable by wrappers in other languages

    char* GetCarmaVersion()
    {
      char* version_ptr = nullptr;
      int version_length = 0;
      char* return_value = nullptr;

      InternalGetCarmaVersion(&version_ptr, &version_length);

      return_value = new char[version_length + 1];
      std::memcpy(return_value, version_ptr, version_length);
      return_value[version_length] = '\0';

      InternalFreeCarmaVersion(version_ptr, version_length);
      return return_value;
    }

#ifdef __cplusplus
  }  // extern "C"
#endif
}  // namespace musica