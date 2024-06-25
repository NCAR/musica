// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstddef>

#define MUSICA_ERROR_CATEGORY               "MUSICA Error"
#define MUSICA_ERROR_CODE_SPECIES_NOT_FOUND 1

#ifdef __cplusplus
  #include <system_error>

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

    /// @brief A struct to describe failure conditions
    struct Error
    {
      int code_;
      String category_;
      String message_;
    };

    /// @brief A struct to represent a mapping between a string and an index
    struct Mapping
    {
      String name_;
      std::size_t index_;
    };

    /// @brief Casts a char* to a String
    /// @param value The char* to cast
    /// @return The casted String
    String CreateString(const char* value);

    /// @brief Deletes a String
    /// @param str The String to delete
    void DeleteString(String* str);

    /// @brief Creates an Error indicating no error
    /// @return The Error
    Error NoError();

    /// @brief Creates an Error from a category, code, and message
    /// @param category The category of the Error
    /// @param code The code of the Error
    /// @param message The message of the Error
    /// @return The Error
    Error ToError(const char* category, int code, const char* message);

    /// @brief Deletes an Error
    /// @param error The Error to delete
    void DeleteError(Error* error);

    /// @brief Deletes a Mapping
    /// @param mapping The Mapping to delete
    void DeleteMapping(Mapping* mapping);

    /// @brief Deletes an array of Mappings
    /// @param mappings The array of Mappings to delete
    /// @param size The size of the array
    void DeleteMappings(Mapping* mappings, std::size_t size);

#ifdef __cplusplus
  }
  /// @brief Creates an Error from a category and code
  /// @param category The category of the Error
  /// @param code The code of the Error
  /// @return The Error
  Error ToError(const char* category, int code);

  /// @brief Creates an Error from syd::system_error
  /// @param e The std::system_error to convert
  /// @return The Error
  Error ToError(const std::system_error& e);

  /// @brief Checks for success
  /// @param error The Error to check
  /// @return True if the Error is successful, false otherwise
  bool IsSuccess(const Error& error);

  /// @brief Checks for a specific error
  /// @param error The Error to check
  /// @param category The category of the Error
  /// @param code The code of the Error
  /// @return True if the Error matches the category and code, false otherwise
  bool IsError(const Error& error, const char* category, int code);

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
  Mapping ToMapping(const char* name, std::size_t index);

#endif

}  // namespace musica