/*
Node API for MICM Class
*/
#include "micm.h"

using namespace musica_addon;

/// @brief C++ wrapper for MICM solver
// MICM Class - N-API Wrapper

MICMClass::MICMClass(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<MICMClass>(info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 2)
  {
    Napi::TypeError::New(env, "Expected 2 arguments: config_path and solver_type").ThrowAsJavaScriptException();
    return;
  }

  if (!info[0].IsString())
  {
    Napi::TypeError::New(env, "config_path must be a string").ThrowAsJavaScriptException();
    return;
  }

  if (!info[1].IsNumber())
  {
    Napi::TypeError::New(env, "solver_type must be a number").ThrowAsJavaScriptException();
    return;
  }

  std::string config_path = info[0].As<Napi::String>().Utf8Value();
  int solver_type = info[1].As<Napi::Number>().Int32Value();

  try
  {
    micm_ = std::make_unique<MICMWrapper>(config_path.c_str(), static_cast<musica::MICMSolver>(solver_type));
  }
  catch (const std::exception& e)
  {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
  }
}

Napi::Value MICMClass::CreateState(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 1)
  {
    Napi::TypeError::New(env, "Expected 1 argument: number_of_grid_cells").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsNumber())
  {
    Napi::TypeError::New(env, "number_of_grid_cells must be a number").ThrowAsJavaScriptException();
    return env.Null();
  }

  size_t num_cells = info[0].As<Napi::Number>().Int64Value();

  try
  {
    musica::State* raw_state = micm_->CreateState(num_cells);
    auto state_wrapper = std::make_shared<StateWrapper>(raw_state);

    // Return StateClass instance using global constructor
    if (g_StateConstructor == nullptr)
    {
      Napi::Error::New(env, "State constructor not initialized").ThrowAsJavaScriptException();
      return env.Null();
    }

    Napi::Object state_obj = g_StateConstructor.New({ Napi::External<StateWrapper>::New(env, state_wrapper.get()) });
    return state_obj;
  }
  catch (const std::exception& e)
  {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value MICMClass::Solve(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 2)
  {
    Napi::TypeError::New(env, "Expected 2 arguments: state and time_step").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (!info[0].IsObject() || !info[1].IsNumber())
  {
    Napi::TypeError::New(env, "Invalid arguments").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  // Extract StateWrapper from the object
  Napi::Object state_obj = info[0].As<Napi::Object>();
  StateClass* state_class = StateClass::Unwrap(state_obj);
  double time_step = info[1].As<Napi::Number>().DoubleValue();

  try
  {
    musica::String solver_state;
    musica::SolverResultStats stats;
    micm_->Solve(state_class->GetState(), time_step);
  }
  catch (const std::exception& e)
  {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
  }

  return env.Undefined();
}

Napi::Value MICMClass::GetSolverType(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();
  return Napi::Number::New(env, micm_->GetSolverType());
}

Napi::Object MICMClass::Init(Napi::Env env, Napi::Object exports)
{
  Napi::Function func = DefineClass(
      env,
      "MICM",
      {
          InstanceMethod("createState", &MICMClass::CreateState),
          InstanceMethod("solve", &MICMClass::Solve),
          InstanceMethod("getSolverType", &MICMClass::GetSolverType),
      });

  Napi::FunctionReference* constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  env.SetInstanceData(constructor);

  exports.Set("MICM", func);
  return exports;
}