// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/utils/error.hpp>

#include <stdexcept>

namespace musica
{
  enum class ErrorCode
  {
    Unknown = MUSICA_ERROR_CODE_UNKNOWN,
    MappingNotFound = MUSICA_ERROR_CODE_MAPPING_NOT_FOUND,
    MappingOptionsUndefined = MUSICA_ERROR_CODE_MAPPING_OPTIONS_UNDEFINED,
  };

  enum class ParseErrorCode
  {
    ParsingFailed = MUSICA_PARSE_ERROR_CODE_PARSING_FAILED,
    InvalidConfigFile = MUSICA_PARSE_ERROR_CODE_INVALID_CONFIG_FILE,
    UnsupportedVersion = MUSICA_PARSE_ERROR_CODE_UNSUPPORTED_VERSION,
    FailedToCastToVersion = MUSICA_PARSE_ERROR_CODE_FAILED_TO_CAST_TO_VERSION,
  };

  enum class MicmErrorCode
  {
    SpeciesNotFound = MUSICA_MICM_ERROR_CODE_SPECIES_NOT_FOUND,
    SolverTypeNotFound = MUSICA_MICM_ERROR_CODE_SOLVER_TYPE_NOT_FOUND,
    UnsupportedSolverStatePair = MUSICA_MICM_ERROR_CODE_UNSUPPORTED_SOLVER_STATE_PAIR,
    NullPointer = MUSICA_MICM_ERROR_CODE_NULL_POINTER,
  };

  enum class MiamErrorCode
  {
    SpeciesNotFound = MUSICA_MIAM_ERROR_CODE_SPECIES_NOT_FOUND,
    PhaseNotFound = MUSICA_MIAM_ERROR_CODE_PHASE_NOT_FOUND,
    SolverTypeNotFound = MUSICA_MIAM_ERROR_CODE_SOLVER_TYPE_NOT_FOUND,
    MissingAerosolSection = MUSICA_MIAM_ERROR_CODE_MISSING_AEROSOL_SECTION,
    InvalidAerosolConfiguration = MUSICA_MIAM_ERROR_CODE_INVALID_AEROSOL_CONFIGURATION,
  };

  enum class MiemErrorCode
  {
    UnresolvedReference = MUSICA_MIEM_ERROR_CODE_UNRESOLVED_REFERENCE,
  };

  /// @note These are provided solely to support the short-form Exception constructor
  template<typename T>
  const char* error_category_for();
  template<>
  inline const char* error_category_for<ErrorCode>()
  {
    return MUSICA_ERROR_CATEGORY;
  }
  template<>
  inline const char* error_category_for<ParseErrorCode>()
  {
    return MUSICA_PARSE_ERROR_CATEGORY;
  }
  template<>
  inline const char* error_category_for<MicmErrorCode>()
  {
    return MUSICA_MICM_ERROR_CATEGORY;
  }
  template<>
  inline const char* error_category_for<MiamErrorCode>()
  {
    return MUSICA_MIAM_ERROR_CATEGORY;
  }
  template<>
  inline const char* error_category_for<MiemErrorCode>()
  {
    return MUSICA_MIEM_ERROR_CATEGORY;
  }

  /// @brief Unified exception type used to represent error codes from any subsystem.
  struct Exception : public std::runtime_error
  {
    int code_;
    const char* category_;

    /// @brief Constructs an exception from a typed error code enum and a category string.
    template<typename ErrorCodeEnum>
    Exception(ErrorCodeEnum code, const char* category, const std::string& msg)
        : std::runtime_error(msg),
          code_(static_cast<int>(code)),
          category_(category)
    {
    }

    /// @brief Constructs an exception from a typed error code enum, inferring the category from the enum type.
    template<typename ErrorCodeEnum>
    Exception(ErrorCodeEnum code, const std::string& msg)
        : std::runtime_error(msg),
          code_(static_cast<int>(code)),
          category_(error_category_for<ErrorCodeEnum>())
    {
    }
  };

}  // namespace musica
