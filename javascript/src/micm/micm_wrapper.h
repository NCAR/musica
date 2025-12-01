#pragma once

#include "state_wrapper.h"

#include <musica/micm/micm.hpp>

#include <micm/solver/solver_result.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

// Forward declarations of MUSICA types
namespace musica
{
  class State;
}

namespace musica_addon
{

  /// @brief C++ wrapper for MICM solver
  class MICMWrapper
  {
   public:
    MICMWrapper(const std::string& config_path, int solver_type);
    ~MICMWrapper();

    musica::State* CreateState(size_t number_of_grid_cells);
    micm::SolverResult Solve(musica::State* state, double time_step);
    int GetSolverType() const;

   private:
    musica::MICM* micm_;
    int solver_type_;
  };

}  // namespace musica_addon