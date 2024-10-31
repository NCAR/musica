// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains hepler functions for the internal TUV-x interface.
#pragma once

#include <cstring>
#include <musica/util.hpp>

namespace musica
{

#ifdef __cplusplus
  extern "C"
  {
#endif

  /// @brief Create a string from a C-style string
  /// @param str C-style string
  /// @return String struct
  String InternalCreateString(const char *str);

  /// @brief Allocate an array of Mappings
  /// @param size Size of the array
  /// @return Pointer to the array of Mappings
  Mapping *InternalAllocateMappings(const std::size_t size);

#ifdef __cplusplus
  } // extern "C"
#endif
}