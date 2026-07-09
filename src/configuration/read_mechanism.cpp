#include <musica/configuration/read_mechanism.hpp>
#include <musica/utils/error_code.hpp>

#include <mechanism_configuration/parse.hpp>

namespace musica
{
  mechanism_configuration::Mechanism ReadMechanism(const std::string& config_path)
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

    return *parsed;
  }

  mechanism_configuration::Mechanism ReadMechanismFromString(const std::string& json_or_yaml_string)
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

    return *parsed;
  }
}  // namespace musica
