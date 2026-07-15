#pragma once

#include <mechanism_configuration/mechanism.hpp>

#include <string>

namespace musica
{
  mechanism_configuration::Mechanism ReadMechanism(const std::string& config_path);
  mechanism_configuration::Mechanism ReadMechanismFromString(const std::string& json_or_yaml_string);
}  // namespace musica
