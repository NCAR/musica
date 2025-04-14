#pragma once

#include <musica/micm/chemistry.hpp>
#include <musica/micm/micm.hpp>
#include <musica/error.hpp>

#include <mechanism_configuration/parser.hpp>

#include <stdexcept>
#include <system_error>

enum class MusicaParseErrc
{
  InvalidConfigFile = MUSICA_PARSE_INVALID_CONFIG_FILE,
  UnsupportedVersion = MUSICA_PARSE_UNSUPPORTED_VERSION
};

namespace std
{
  template<>
  struct is_error_condition_enum<MusicaParseErrc> : true_type
  {
  };
}  // namespace std

namespace
{
  class MusicaParseErrorCategory : public std::error_category
  {
   public:
    const char* name() const noexcept override
    {
      return MUSICA_ERROR_CATEGORY_PARSING;
    }
    std::string message(int ev) const override
    {
      switch (static_cast<MusicaParseErrc>(ev))
      {
        case MusicaParseErrc::InvalidConfigFile: return "Invalid configuration file";
        case MusicaParseErrc::UnsupportedVersion: return "Unsupported version";
        default: return "Unknown error";
      }
    }
  };

  const MusicaParseErrorCategory MUSICA_PARSE_ERROR{};
}  // namespace

inline std::error_code make_error_code(MusicaParseErrc e)
{
  return { static_cast<int>(e), MUSICA_PARSE_ERROR };
}

namespace musica
{
  Chemistry ReadConfiguration(const std::string& config_path);
  Chemistry ParserV0(const mechanism_configuration::ParserResult<>& result);
  Chemistry ParserV1(const mechanism_configuration::ParserResult<>& result);

  // Utility functions to check types and perform conversions
  bool IsBool(const std::string& value);
  bool IsInt(const std::string& value);
  bool IsFloatingPoint(const std::string& value);
}  // namespace musica