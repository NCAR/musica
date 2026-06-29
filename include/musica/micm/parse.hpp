#pragma once

#include <musica/micm/chemistry.hpp>
#include <musica/micm/micm.hpp>
#include <musica/utils/error.hpp>

#include <mechanism_configuration/parse.hpp>
#include <mechanism_configuration/mechanism.hpp>

#include <stdexcept>

namespace musica
{
  Chemistry ReadConfiguration(const std::string& config_path);
  Chemistry ReadConfigurationFromString(const std::string& json_or_yaml_string);
  Chemistry ConvertMechanism(const mechanism_configuration::Mechanism& mechanism);

  mechanism_configuration::Mechanism ConvertV0MechanismToV1(
      const std::string& config_path,
      bool convert_reaction_units = true);

  // Utility functions to check types and perform conversions
  bool IsBool(const std::string& value);
  bool IsInt(const std::string& value);
  bool IsFloatingPoint(const std::string& value);
}  // namespace musica