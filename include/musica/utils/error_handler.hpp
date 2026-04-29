// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/utils/string.hpp>
#include <musica/utils/error.hpp>

#ifdef __cplusplus
  #include <musica/utils/error_code.hpp>
  #include <exception>
  #ifdef MUSICA_USE_MICM
    #include <micm/util/micm_exception.hpp>
  #endif

namespace musica
{

  extern "C"
  {
#endif

  /// @brief The C/Fortran-compatible Error struct
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

  /// @brief Creates an Error from musica::Exception
  /// @param e The musica::Exception to convert [input]
  /// @param severity The severity of the Error (MUSICA_SEVERITY_WARNING/ERROR/CRITICAL) [input]
  /// @param error The Error [output]
  void ToError(const Exception& e, int severity, Error* error);

  /// @brief Creates an Error from std::exception
  /// @param e The std::exception to convert [input]
  /// @param severity The severity of the Error (MUSICA_SEVERITY_WARNING/ERROR/CRITICAL) [input]
  /// @param error The Error [output]
  void ToError(const std::exception& e, int severity, Error* error);

#ifdef MUSICA_USE_MICM
  /// @brief Creates an Error from a micm::MicmException
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

  /// Catches all C++ exceptions and converts them to Error* for C/Fortran callers.
  template<typename Func>
  auto HandleErrors(Func func, Error* error) -> decltype(func())
  {
    DeleteError(error);
    try
    {
      return func();
    }
    catch (const micm::MicmException& e) { ToError(e, error); }
    catch (const musica::Exception& e)   { ToError(e, MUSICA_SEVERITY_ERROR, error); }
    catch (const std::exception& e)      { ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_UNKNOWN, e.what(), MUSICA_SEVERITY_CRITICAL, error); }
    catch (...)                          { ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_UNKNOWN, "Unknown error", MUSICA_SEVERITY_CRITICAL, error); }
    return decltype(func())();
  }

}  // namespace musica
#endif