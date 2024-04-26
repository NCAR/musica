/// Copyright (C) 2023-2024 National Center for Atmospheric Research
/// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstddef>

#define MUSICA_ERROR_CATEGORY "MUSICA Error"
#define MUSICA_ERROR_CODE_SPECIES_NOT_FOUND 1

#ifdef __cplusplus
#include <system_error>

extern "C"
{
#endif

/// @brief A struct to represent a string
struct String
{
  char* value_;
  size_t size_;
};

/// @brief A struct to represent a const string
struct ConstString
{
  const char* value_;
  size_t size_;
};

/// @brief A struct to describe failure conditions
struct Error
{
  int code_;
  ConstString category_;
  ConstString message_;
};

/// @brief A struct to represent a mapping between a string and an index
struct Mapping
{
    ConstString name_;
    size_t index_;
};

/// @brief Casts a char* to a String
/// @param value The char* to cast
/// @return The casted String
String ToString(char* value);

/// @brief Casts a const char* to a String
ConstString ToConstString(const char* value);

/// @brief Deletes a String
/// @param str The String to delete
void DeleteString(String str);

#ifdef __cplusplus
}

/// @brief Creates an Error indicating no error
/// @return The Error
Error NoError();

/// @brief Creates an Error from a category and code
/// @param category The category of the Error
/// @param code The code of the Error
/// @return The Error
Error ToError(const char* category, int code);

/// @brief Creates an Error from a category, code, and message
/// @param category The category of the Error
/// @param code The code of the Error
/// @param message The message of the Error
/// @return The Error
Error ToError(const char* category, int code, const char* message);

/// @brief Creates an Error from syd::system_error
/// @param e The std::system_error to convert
/// @return The Error
Error ToError(const std::system_error& e);

/// @brief Overloads the equality operator for Error types
/// @param lhs The left-hand side Error
/// @param rhs The right-hand side Error
/// @return True if the Errors are equal, false otherwise
bool operator==(const Error& lhs, const Error& rhs);

/// @brief Overloads the inequality operator for Error types
/// @param lhs The left-hand side Error
/// @param rhs The right-hand side Error
/// @return True if the Errors are not equal, false otherwise
bool operator!=(const Error& lhs, const Error& rhs);

/// @brief Creates a Mapping from a name and index
/// @param name The name of the Mapping
/// @param index The index of the Mapping
/// @return The Mapping
Mapping ToMapping(const char* name, size_t index);

#endif