#pragma once

#include <musica/utils/error.hpp>
#include <musica/micm/chemistry.hpp>
#include <musica/micm/micm.hpp>

#include <mechanism_configuration/parser.hpp>
#include <mechanism_configuration/v0/types.hpp>
#include <mechanism_configuration/v1/mechanism.hpp>

#include <stdexcept>
#include <system_error>


namespace musica
{
  Chemistry ReadConfiguration(const std::string& config_path);
  Chemistry ReadConfigurationFromString(const std::string& json_or_yaml_string);
  Chemistry ParserV0(const mechanism_configuration::ParserResult<>& result);
  Chemistry ConvertV1Mechanism(const mechanism_configuration::v1::types::Mechanism& v1_mechanism);
  Chemistry ParserV1(const mechanism_configuration::ParserResult<>& result);

  mechanism_configuration::v1::types::Mechanism ConvertV0MechanismToV1(
      const std::string& config_path,
      bool convert_reaction_units = true);
  mechanism_configuration::v1::types::Mechanism ConvertV0MechanismToV1(
      const mechanism_configuration::v0::types::Mechanism& v0_mechanism,
      bool convert_reaction_units = true);

  // Utility functions to check types and perform conversions
  bool IsBool(const std::string& value);
  bool IsInt(const std::string& value);
  bool IsFloatingPoint(const std::string& value);
}  // namespace musica