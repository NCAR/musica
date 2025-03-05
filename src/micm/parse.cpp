#include <musica/micm/parse.hpp>

#include <mechanism_configuration/v0/types.hpp>
#include <mechanism_configuration/v0/validation.hpp>

#include <micm/system/species.hpp>
#include <micm/system/phase.hpp>

namespace musica
{
  namespace v0 {
    bool convert_species(Chemistry& chemistry, const std::vector<mechanism_configuration::v0::types::Species>& species)
    {
      using namespace mechanism_configuration::v0;
      micm::Phase gas_phase;
      for(const auto& elem : species)
      {
        micm::Species s;
        s.name_ = elem.name;
        s.SetProperty(validation::MOL_WEIGHT, elem.molecular_weight);
        s.SetProperty(validation::DIFFUSION_COEFF, elem.diffusion_coefficient);
        s.SetProperty(validation::THIRD_BODY, elem.third_body);
        s.SetProperty(validation::ABS_TOLERANCE, elem.absolute_tolerance);
        s.SetProperty(validation::TRACER_TYPE, elem.tracer_type);
        gas_phase.species_.push_back(s);
      }
      chemistry.system.gas_phase_ = gas_phase;
      return true;
    }
  }

  Chemistry ParserV0(const mechanism_configuration::ParserResult<> &result, Error* error)
  {
    using V0 = mechanism_configuration::v0::types::Mechanism;
    V0* v0_mechanism = dynamic_cast<V0*>(result.mechanism.get());
    Chemistry chemistry{};
    if (!v0_mechanism)
    {
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_CONFIG_PARSE_FAILED, "Failed to cast to V0");
    }
    else {
      if (!v0::convert_species(chemistry, v0_mechanism->species))
      {
        *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_CONFIG_PARSE_FAILED, "Failed to convert species");
      }
    }

    return chemistry;
  }

  Chemistry ParserV1(const mechanism_configuration::ParserResult<> &result, Error* error)
  {
    return Chemistry{};
  }

}