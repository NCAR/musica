#include "solver_result_wrapper.h"

#include <micm/solver/state.hpp>

namespace musica_addon
{

  Napi::Object SolverResultWrapper::StatsToJS(Napi::Env env, const musica::SolverResultStats& stats)
  {
    Napi::Object obj = Napi::Object::New(env);

    obj.Set("function_calls", Napi::Number::New(env, static_cast<double>(stats.function_calls_)));
    obj.Set("jacobian_updates", Napi::Number::New(env, static_cast<double>(stats.jacobian_updates_)));
    obj.Set("number_of_steps", Napi::Number::New(env, static_cast<double>(stats.number_of_steps_)));
    obj.Set("accepted", Napi::Number::New(env, static_cast<double>(stats.accepted_)));
    obj.Set("rejected", Napi::Number::New(env, static_cast<double>(stats.rejected_)));
    obj.Set("decompositions", Napi::Number::New(env, static_cast<double>(stats.decompositions_)));
    obj.Set("solves", Napi::Number::New(env, static_cast<double>(stats.solves_)));
    obj.Set("final_time", Napi::Number::New(env, stats.final_time_));

    return obj;
  }

  Napi::Object SolverResultWrapper::ResultToJS(Napi::Env env, const micm::SolverResult& result)
  {
    Napi::Object obj = Napi::Object::New(env);

    // Convert SolverState enum to integer
    obj.Set("state", Napi::Number::New(env, static_cast<int>(result.state_)));

    // Convert stats to JavaScript object
    obj.Set("stats", StatsToJS(env, result.stats_));

    return obj;
  }

}  // namespace musica_addon
