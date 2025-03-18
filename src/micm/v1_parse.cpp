#include <musica/micm/parse.hpp>

#include <micm/process/arrhenius_rate_constant.hpp>
#include <micm/process/branched_rate_constant.hpp>
#include <micm/process/process.hpp>
#include <micm/process/surface_rate_constant.hpp>
#include <micm/process/ternary_chemical_activation_rate_constant.hpp>
#include <micm/process/troe_rate_constant.hpp>
#include <micm/process/tunneling_rate_constant.hpp>
#include <micm/process/user_defined_rate_constant.hpp>
#include <micm/system/phase.hpp>
#include <micm/system/species.hpp>

#include <mechanism_configuration/v0/types.hpp>
#include <mechanism_configuration/v0/validation.hpp>

namespace musica
{
  Chemistry ParserV1(const mechanism_configuration::ParserResult<>& result, Error* error)
  {
    return Chemistry{};
  }

}  // namespace musica