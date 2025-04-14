#pragma once

#include <musica/micm/chemistry.hpp>
#include <musica/micm/micm.hpp>

#include <mechanism_configuration/parser.hpp>

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