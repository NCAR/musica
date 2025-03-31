#pragma once

#include <micm/process/process.hpp>
#include <micm/system/system.hpp>

namespace musica
{
  struct Chemistry
  {
    micm::System system;
    std::vector<micm::Process> processes;
  };
}  // namespace musica