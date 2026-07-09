// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <mechanism_configuration/mechanism.hpp>

#include <miem/source_types.hpp>

#include <string>
#include <vector>

namespace musica
{
  struct Emissions
  {
    std::vector<miem::Source> sources;
    miem::Regridding regridding;
  };

  Emissions ReadEmissionsConfiguration(const std::string& config_path);
  Emissions ReadEmissionsConfigurationFromString(const std::string& json_or_yaml_string);
  Emissions ConvertEmissions(const mechanism_configuration::Mechanism& mechanism);
}  // namespace musica
