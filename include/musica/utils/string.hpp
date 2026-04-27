// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstddef>

#ifdef __cplusplus
namespace musica
{
  extern "C"
  {
#endif

    /// @brief A struct to represent a string
    struct String
    {
      char* value_ = nullptr;
      std::size_t size_ = 0;
    };

    /// @brief Casts a char* to a String
    /// @param value The char* to cast [input]
    /// @param str The casted String [output]
    void CreateString(const char* value, String* str);

    /// @brief Deletes a String
    /// @param str The String to delete
    void DeleteString(String* str);

#ifdef __cplusplus
  }
}  // namespace musica
#endif