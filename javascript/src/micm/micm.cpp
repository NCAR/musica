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
  // Constructor can be empty when using factory methods
  // Factory methods will initialize micm_ after construction
  return;
}

Napi::Value MICMClass::FromConfigPath(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 1)
  {
    Napi::TypeError::New(env, "Expected at least 1 argument: config_path").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString())
  {
    Napi::TypeError::New(env, "config_path must be a string").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string config_path = info[0].As<Napi::String>().Utf8Value();
  int solver_type = 1;  // Default to rosenbrock_standard_order

  if (info.Length() >= 2 && info[1].IsNumber())
  {
    solver_type = info[1].As<Napi::Number>().Int32Value();
  }

  try
  {
    // Get the constructor function
    Napi::Function cons = env.GetInstanceData<Napi::FunctionReference>()->Value();

    // Create new instance
    Napi::Object instance = cons.New({});
    MICMClass* obj = Napi::ObjectWrap<MICMClass>::Unwrap(instance);

    // Initialize the wrapper with config path using factory method
    obj->micm_ = MICMWrapper::FromConfigPath(config_path, solver_type);

    return instance;
  }
  catch (const std::exception& e)
  {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value MICMClass::FromConfigString(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 1)
  {
    Napi::TypeError::New(env, "Expected at least 1 argument: config_string").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString())
  {
    Napi::TypeError::New(env, "config_string must be a string").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string config_string = info[0].As<Napi::String>().Utf8Value();
  int solver_type = 1;  // Default to rosenbrock_standard_order

  if (info.Length() >= 2 && info[1].IsNumber())
  {
    solver_type = info[1].As<Napi::Number>().Int32Value();
  }

  try
  {
    // Get the constructor function
    Napi::Function cons = env.GetInstanceData<Napi::FunctionReference>()->Value();

    // Create new instance
    Napi::Object instance = cons.New({});
    MICMClass* obj = Napi::ObjectWrap<MICMClass>::Unwrap(instance);

    // Initialize the wrapper with config string using factory method
    obj->micm_ = MICMWrapper::FromConfigString(config_string, solver_type);

    return instance;
  }
  catch (const std::exception& e)
  {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
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
    auto state_wrapper = new StateWrapper(raw_state);

    // Create External without finalizer - StateClass will manage lifetime
    auto ext = Napi::External<StateWrapper>::New(env, state_wrapper);

    // Return StateClass instance using global constructor
    if (StateClass::g_StateConstructor == nullptr)
    {
      Napi::Error::New(env, "State constructor not initialized").ThrowAsJavaScriptException();
      return env.Null();
    }

    return StateClass::g_StateConstructor.New({ ext });
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
    return env.Null();
  }

  if (!info[0].IsObject() || !info[1].IsNumber())
  {
    Napi::TypeError::New(env, "Invalid arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  // Extract StateWrapper from the object
  Napi::Object state_obj = info[0].As<Napi::Object>();
  StateClass* state_class = StateClass::Unwrap(state_obj);
  double time_step = info[1].As<Napi::Number>().DoubleValue();

  try
  {
    micm::SolverResult result = micm_->Solve(state_class->GetState(), time_step);
    return SolverResultWrapper::ResultToJS(env, result);
  }
  catch (const std::exception& e)
  {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
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
          StaticMethod("fromConfigPath", &MICMClass::FromConfigPath),
          StaticMethod("fromConfigString", &MICMClass::FromConfigString),
      });

  Napi::FunctionReference* constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  env.SetInstanceData(constructor);

  exports.Set("MICM", func);
  return exports;
}