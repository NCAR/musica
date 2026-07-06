// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/miem/emissions.hpp>

#include <mechanism_configuration/mechanism.hpp>

#include <string>

namespace musica
{
  Emissions ReadEmissionsConfiguration(const std::string& config_path);
  Emissions ReadEmissionsConfigurationFromString(const std::string& json_or_yaml_string);
  Emissions ConvertEmissions(const mechanism_configuration::Mechanism& mechanism);
}  // namespace musica
