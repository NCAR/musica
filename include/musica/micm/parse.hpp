#include <musica/util.hpp>
#include <musica/micm/micm.hpp>
#include <mechanism_configuration/parser.hpp>

namespace musica
{
  Chemistry ParserV0(const mechanism_configuration::ParserResult<> &result, Error* error);
  Chemistry ParserV1(const mechanism_configuration::ParserResult<> &result, Error* error);
}