#include <musica/micm/parse.hpp>
#include <musica/util.hpp>

#include <mechanism_configuration/parser.hpp>

namespace musica
{
  Chemistry ReadConfiguration(const std::string& config_path, Error* error)
  {
    DeleteError(error);

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
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_CONFIG_PARSE_FAILED, errors.c_str());
    }
    else
    {
      mechanism_configuration::Version version = parsed.mechanism->version;

      switch (version.major)
      {
        case 0: chemistry = ParserV0(parsed, error); break;
        case 1: chemistry = ParserV1(parsed, error); break;
        default:
          std::string msg = "Version " + std::to_string(version.major) + " not supported";
          *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_VERSION_NOT_SUPPORTED, msg.c_str());
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