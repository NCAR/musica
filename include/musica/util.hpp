/// Copyright (C) 2023-2024 National Center for Atmospheric Research
/// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstddef>

#ifdef __cplusplus
namespace musica {
extern "C"
{
#endif

/// @brief A struct to represent a string
struct String
{
  char* value_;
  size_t size_;
};

/// @brief A struct to represent a mapping between a string and an index
struct Mapping
{
    char name[256];
    size_t index;
    size_t string_length;
};

/// @brief Casts a char* to a String
/// @param value The char* to cast
/// @return The casted String
String ToString(char* value);

/// @brief Deletes a String
/// @param str The String to delete
void DeleteString(String str);

#ifdef __cplusplus
}
}
#endif