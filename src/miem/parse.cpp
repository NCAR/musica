// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/miem/parse.hpp>
#include <musica/utils/error_code.hpp>

#include <mechanism_configuration/parse.hpp>

namespace musica
{
  Emissions ReadEmissionsConfiguration(const std::string& config_path)
  {
    auto parsed = mechanism_configuration::Parse(config_path);
    if (!parsed)
    {
      std::string errors;
      for (const auto& [code, message] : parsed.error())
      {
        errors += message + "\n";
      }
      throw musica::Exception(musica::ParseErrorCode::InvalidConfigFile, errors);
    }

    return ConvertEmissions(*parsed);
  }

  Emissions ReadEmissionsConfigurationFromString(const std::string& json_or_yaml_string)
  {
    auto parsed = mechanism_configuration::ParseFromString(json_or_yaml_string);
    if (!parsed)
    {
      std::string errors = "Failed to parse configuration string:\n";
      for (const auto& [code, message] : parsed.error())
      {
        errors += message + "\n";
      }
      throw musica::Exception(musica::ParseErrorCode::ParsingFailed, errors);
    }

    return ConvertEmissions(*parsed);
  }
}  // namespace musica
