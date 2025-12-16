#include <musica/micm/parse.hpp>

#include <mechanism_configuration/parser.hpp>
#include <mechanism_configuration/v1/parser.hpp>

namespace musica
{
  Chemistry ReadConfiguration(const std::string& config_path)
  {
    mechanism_configuration::UniversalParser parser;
    Chemistry chemistry{};

    auto parsed = parser.Parse(config_path);
    if (!parsed)
    {
      std::string errors;
      for (auto& error : parsed.errors)
      {
        errors += error.second + "\n";
      }
      throw std::system_error(make_error_code(MusicaParseErrc::InvalidConfigFile), errors);
    }
    else
    {
      const mechanism_configuration::Version version = parsed.mechanism->version;

      switch (version.major)
      {
        case 0: chemistry = ParserV0(parsed); break;
        case 1: chemistry = ParserV1(parsed); break;
        default:
          const std::string msg = "Version " + std::to_string(version.major) + " not supported";
          throw std::system_error(make_error_code(MusicaParseErrc::UnsupportedVersion), msg);
      }
    }

    return chemistry;
  }

  Chemistry ReadConfigurationFromString(const std::string& json_or_yaml_string)
  {
    Chemistry chemistry{};

    // Parse as v1 format
    // Only v1 supports string parsing; JavaScript wrappers generate v1 format
    mechanism_configuration::v1::Parser v1_parser;
    auto v1_parsed = v1_parser.ParseFromString(json_or_yaml_string);

    if (!v1_parsed)
    {
      std::string errors = "Failed to parse configuration string:\n";
      for (auto& error : v1_parsed.errors)
      {
        errors += error.second + "\n";
      }
      throw std::system_error(make_error_code(MusicaParseErrc::ParsingFailed), errors);
    }

    // Convert v1 mechanism directly to Chemistry
    chemistry = ConvertV1Mechanism(*v1_parsed.mechanism);
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