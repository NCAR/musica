#include <musica/micm/parse.hpp>
#include <musica/utils/error_code.hpp>

#include <mechanism_configuration/parse.hpp>

#include <sstream>

namespace musica
{
  Chemistry ReadConfiguration(const std::string& config_path)
  {
    Chemistry chemistry{};

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
    else
    {
      chemistry = ConvertMechanism(*parsed);
    }

    return chemistry;
  }

  Chemistry ReadConfigurationFromString(const std::string& json_or_yaml_string)
  {
    Chemistry chemistry{};

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

    chemistry = ConvertMechanism(*parsed);
    return chemistry;
  }

  bool IsBool(const std::string& value)
  {
    return (value == "true" || value == "false");
  }

  bool IsInt(const std::string& value)
  {
    std::istringstream iss(value);
    int result;
    return (iss >> result >> std::ws).eof() && !value.empty();
  }

  bool IsFloatingPoint(const std::string& value)
  {
    std::istringstream iss(value);
    double result;
    return (iss >> result >> std::ws).eof() && !value.empty();
  }
}  // namespace musica