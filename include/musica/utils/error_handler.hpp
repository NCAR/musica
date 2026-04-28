// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/utils/string.hpp>
#include <musica/utils/error.hpp>
#ifdef MUSICA_USE_MICM
  #include <micm/util/micm_exception.hpp>
#endif

#ifdef __cplusplus
  #include <system_error>

namespace musica
{

  extern "C"
  {
#endif

  /// @brief A struct to describe failure conditions
  struct Error
  {
    int code_ = MUSICA_STATUS_SUCCESS;
    int severity_ = MUSICA_SEVERITY_INFO;
    String category_;
    String message_;
  };


  /// @brief Deletes an Error
  /// @param error The Error to delete
  void DeleteError(Error* error);

  /// @brief Creates an Error indicating no error
  /// @param error The Error [output]
  void NoError(Error* error);

  /// @brief Creates an Error from a category, code, message, and severity
  /// @param category The category of the Error [input]
  /// @param code The code of the Error [input]
  /// @param message The message of the Error [input]
  /// @param severity The severity of the Error (MUSICA_SEVERITY_WARNING/ERROR/CRITICAL) [input]
  /// @param error The Error [output]
  void ToError(const char* category, int code, const char* message, int severity, Error* error);

#ifdef __cplusplus
  }

  /// @brief Creates an Error from std::system_error
  /// @param e The std::system_error to convert [input]
  /// @param severity The severity of the Error (MUSICA_SEVERITY_WARNING/ERROR/CRITICAL) [input]
  /// @param error The Error [output]
  void ToError(const std::system_error& e, int severity, Error* error);

#ifdef MUSICA_USE_MICM
  /// @brief Creates an Error from a micm::MicmException, mapping MicmSeverity to musica severity
  /// @param e The micm::MicmException to convert [input]
  /// @param error The Error [output]
  void ToError(const micm::MicmException& e, Error* error);
#endif

  /// @brief Checks for success
  /// @param error The Error to check
  /// @return True if the Error is successful, false otherwise
  bool IsSuccess(const Error& error);

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

}  // namespace musica
#endif
