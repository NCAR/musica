// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/utils/error_code.hpp>

namespace
{
  class GeneralErrorCategory : public std::error_category
  {
   public:
    const char* name() const noexcept override
    {
      return MUSICA_ERROR_CATEGORY;
    }
    std::string message(int ev) const override
    {
      switch (static_cast<musica::ErrorCode>(ev))
      {
        case musica::ErrorCode::Unknown:                 return "Unknown error";
        case musica::ErrorCode::MappingNotFound:         return "Mapping not found";
        case musica::ErrorCode::MappingOptionsUndefined: return "Mapping options undefined";
        default:                                         return "Unknown error";
      }
    }
  };

  class MicmErrorCategory : public std::error_category
  {
   public:
    const char* name() const noexcept override
    {
      return MUSICA_MICM_ERROR_CATEGORY;
    }
    std::string message(int ev) const override
    {
      switch (static_cast<musica::MicmErrorCode>(ev))
      {
        case musica::MicmErrorCode::SpeciesNotFound:            return "Species not found";
        case musica::MicmErrorCode::SolverTypeNotFound:         return "Solver type not found";
        case musica::MicmErrorCode::UnsupportedSolverStatePair: return "Unsupported solver/state combination";
        default:                                                return "Unknown error";
      }
    }
  };

  class ParseErrorCategory : public std::error_category
  {
   public:
    const char* name() const noexcept override
    {
      return MUSICA_PARSE_ERROR_CATEGORY;
    }
    std::string message(int ev) const override
    {
      switch (static_cast<musica::ParseErrorCode>(ev))
      {
        case musica::ParseErrorCode::ParsingFailed:         return "Parsing failed";
        case musica::ParseErrorCode::InvalidConfigFile:     return "Invalid configuration file";
        case musica::ParseErrorCode::UnsupportedVersion:    return "Unsupported version";
        case musica::ParseErrorCode::FailedToCastToVersion: return "Failed to cast to version";
        default:                                            return "Unknown error";
      }
    }
  };
}  // namespace

namespace musica
{
  const std::error_category& musica_category()
  {
    static GeneralErrorCategory instance;
    return instance;
  }

  const std::error_category& micm_category()
  {
    static MicmErrorCategory instance;
    return instance;
  }

  const std::error_category& parse_category()
  {
    static ParseErrorCategory instance;
    return instance;
  }
}  // namespace musica

