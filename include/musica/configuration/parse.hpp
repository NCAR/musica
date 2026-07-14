#pragma once

#include <musica/configuration/chemistry.hpp>
#include <musica/micm/micm.hpp>
#include <musica/utils/error.hpp>

#include <mechanism_configuration/mechanism.hpp>
#include <mechanism_configuration/parse.hpp>

#include <stdexcept>

namespace musica
{
  Chemistry ConvertChemistry(const mechanism_configuration::Mechanism& mechanism);

  // Utility functions to check types and perform conversions
  bool IsBool(const std::string& value);
  bool IsInt(const std::string& value);
  bool IsFloatingPoint(const std::string& value);
}  // namespace musica