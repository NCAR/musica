// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/utils/error.hpp>

#include <stdexcept>
#include <system_error>

namespace musica
{
  // General
  enum class ErrorCode
  {
    Unknown                 = MUSICA_ERROR_CODE_UNKNOWN,
    MappingNotFound         = MUSICA_ERROR_CODE_MAPPING_NOT_FOUND,
    MappingOptionsUndefined = MUSICA_ERROR_CODE_MAPPING_OPTIONS_UNDEFINED,
  };

  // MICM-specific
  enum class MicmErrorCode
  {
    SpeciesNotFound            = MUSICA_MICM_ERROR_CODE_SPECIES_NOT_FOUND,
    SolverTypeNotFound         = MUSICA_MICM_ERROR_CODE_SOLVER_TYPE_NOT_FOUND,
    UnsupportedSolverStatePair = MUSICA_MICM_ERROR_CODE_UNSUPPORTED_SOLVER_STATE_PAIR,
  };

  // Parse-specific
  enum class ParseErrorCode
  {
    ParsingFailed         = MUSICA_PARSE_ERROR_CODE_PARSING_FAILED,
    InvalidConfigFile     = MUSICA_PARSE_ERROR_CODE_INVALID_CONFIG_FILE,
    UnsupportedVersion    = MUSICA_PARSE_ERROR_CODE_UNSUPPORTED_VERSION,
    FailedToCastToVersion = MUSICA_PARSE_ERROR_CODE_FAILED_TO_CAST_TO_VERSION,
  };

  /// These are provided solely to support the short-form Exception constructor
  template<typename T> const char* error_category_for();
  template<> inline const char* error_category_for<ErrorCode>()      { return MUSICA_ERROR_CATEGORY; }
  template<> inline const char* error_category_for<MicmErrorCode>()  { return MUSICA_MICM_ERROR_CATEGORY; }
  template<> inline const char* error_category_for<ParseErrorCode>() { return MUSICA_PARSE_ERROR_CATEGORY; }

  /// @brief Unified exception type used to represent error codes from any subsystem.
  struct Exception : public std::runtime_error
  {
    int code_;
    const char* category_;

    Exception(int code, const char* category, const std::string& msg)
        : std::runtime_error(msg), code_(code), category_(category)
    {
    }

    /// @brief For typed enum callers — no cast needed.
    template<typename ErrorCodeEnum>
    Exception(ErrorCodeEnum code, const char* category, const std::string& msg)
        : std::runtime_error(msg), code_(static_cast<int>(code)), category_(category)
    {
    }

    /// @brief Category is determined implicitly from the caller’s enum type.
    template<typename ErrorCodeEnum>
    Exception(ErrorCodeEnum code, const std::string& msg)
        : std::runtime_error(msg), code_(static_cast<int>(code)), category_(error_category_for<ErrorCodeEnum>())
    {
    }
  };

  /// @brief Singleton category declarations
  const std::error_category& musica_category();
  const std::error_category& micm_category();
  const std::error_category& parse_category();

}  // namespace musica

inline std::error_code make_error_code(musica::ErrorCode e)
{
  return { static_cast<int>(e), musica::musica_category() };
}
inline std::error_code make_error_code(musica::MicmErrorCode e)
{
  return { static_cast<int>(e), musica::micm_category() };
}
inline std::error_code make_error_code(musica::ParseErrorCode e)
{
  return { static_cast<int>(e), musica::parse_category() };
}