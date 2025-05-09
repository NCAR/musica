#include <musica/micm/parse.hpp>

#include <mechanism_configuration/parser.hpp>

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
      mechanism_configuration::Version version = parsed.mechanism->version;

      switch (version.major)
      {
        case 0: chemistry = ParserV0(parsed); break;
        case 1: chemistry = ParserV1(parsed); break;
        default:
          std::string msg = "Version " + std::to_string(version.major) + " not supported";
          throw std::system_error(make_error_code(MusicaParseErrc::UnsupportedVersion), msg);
      }
    }

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