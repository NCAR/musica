#pragma once

#include "musica/micm/micm.hpp"
#include "musica/util.hpp"
#include "state_wrapper.h"

#include <map>
#include <memory>
#include <napi.h>
#include <string>
#include <vector>

using namespace musica_addon;

class StateClass : public Napi::ObjectWrap<StateClass>
{
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  static Napi::FunctionReference g_StateConstructor;
  StateClass(const Napi::CallbackInfo& info);
  ~StateClass();
  musica::State* GetState() const;

 private:
  std::unique_ptr<StateWrapper> state_;
  Napi::Value SetConcentrations(const Napi::CallbackInfo& info);
  Napi::Value GetConcentrations(const Napi::CallbackInfo& info);
  Napi::Value SetUserDefinedRateParameters(const Napi::CallbackInfo& info);
  Napi::Value GetUserDefinedRateParameters(const Napi::CallbackInfo& info);
  Napi::Value SetConditions(const Napi::CallbackInfo& info);
  Napi::Value GetConditions(const Napi::CallbackInfo& info);
  Napi::Value GetSpeciesOrdering(const Napi::CallbackInfo& info);
  Napi::Value GetUserDefinedRateParametersOrdering(const Napi::CallbackInfo& info);
  Napi::Value GetNumberOfGridCells(const Napi::CallbackInfo& info);
  Napi::Value ConcentrationStrides(const Napi::CallbackInfo& info);
  Napi::Value UserDefinedRateParameterStrides(const Napi::CallbackInfo& info);
};
