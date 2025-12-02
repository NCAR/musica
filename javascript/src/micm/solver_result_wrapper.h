#pragma once

#include <musica/micm/micm.hpp>

#include <micm/solver/solver_result.hpp>

#include <napi.h>

namespace musica_addon
{

  /// @brief C++ wrapper for converting micm::SolverResult to JavaScript objects
  class SolverResultWrapper
  {
   public:
    /// @brief Convert a SolverStats object to a JavaScript object
    /// @param env Napi environment
    /// @param stats The solver stats to convert
    /// @return JavaScript object containing the stats
    static Napi::Object StatsToJS(Napi::Env env, const musica::SolverResultStats& stats);

    /// @brief Convert a SolverResult to a JavaScript object
    /// @param env Napi environment
    /// @param result The solver result to convert
    /// @return JavaScript object containing state and stats
    static Napi::Object ResultToJS(Napi::Env env, const micm::SolverResult& result);
  };

}  // namespace musica_addon
