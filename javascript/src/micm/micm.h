#pragma once

#include "micm_wrapper.h"
#include "solver_result_wrapper.h"
#include "state.h"
#include "state_wrapper.h"

using namespace musica_addon;

class MICMClass : public Napi::ObjectWrap<MICMClass>
{
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  MICMClass(const Napi::CallbackInfo& info);

  // Static factory methods
  static Napi::Value FromConfigPath(const Napi::CallbackInfo& info);
  static Napi::Value FromConfigString(const Napi::CallbackInfo& info);

 private:
  std::unique_ptr<MICMWrapper> micm_;

  Napi::Value CreateState(const Napi::CallbackInfo& info);
  Napi::Value Solve(const Napi::CallbackInfo& info);
  Napi::Value GetSolverType(const Napi::CallbackInfo& info);

  friend class StateClass;
};
