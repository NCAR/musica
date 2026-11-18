#pragma once

#include <micm/process/process.hpp>
#include <micm/system/phase.hpp>

namespace musica
{
  struct Chemistry
  {
    micm::Phase gas_phase;
    std::vector<micm::Process> processes;
  };
}  // namespace musica