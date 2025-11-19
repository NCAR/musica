/*
Node API for MICM State Class
*/
#include "state.h"

#include <map>
#include <string>
#include <vector>

using namespace musica_addon;

// State Class - N-API Wrapper
StateClass::StateClass(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<StateClass>(info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 1 || !info[0].IsExternal())
  {
    Napi::TypeError::New(env, "Internal construction only").ThrowAsJavaScriptException();
    return;
  }

  // Transfer ownership from External to unique_ptr
  state_ = std::unique_ptr<StateWrapper>(info[0].As<Napi::External<StateWrapper>>().Data());
}

StateClass::~StateClass() = default;

musica::State* StateClass::GetState() const
{
  return state_ ? state_->GetState() : nullptr;
}

Napi::Value StateClass::SetConcentrations(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 1 || !info[0].IsObject())
  {
    Napi::TypeError::New(env, "Expected object with concentrations").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  Napi::Object conc_obj = info[0].As<Napi::Object>();
  std::map<std::string, std::vector<double>> concentrations;

  Napi::Array keys = conc_obj.GetPropertyNames();
  for (uint32_t i = 0; i < keys.Length(); ++i)
  {
    std::string key = keys.Get(i).As<Napi::String>().Utf8Value();
    Napi::Value value = conc_obj.Get(key);

    std::vector<double> values;
    if (value.IsNumber())
    {
      values.push_back(value.As<Napi::Number>().DoubleValue());
    }
    else if (value.IsArray())
    {
      Napi::Array arr = value.As<Napi::Array>();
      for (uint32_t j = 0; j < arr.Length(); ++j)
      {
        values.push_back(arr.Get(j).As<Napi::Number>().DoubleValue());
      }
    }
    concentrations[key] = values;
  }

  try
  {
    state_->SetConcentrations(concentrations);
  }
  catch (const std::exception& e)
  {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
  }

  return env.Undefined();
}

Napi::Value StateClass::GetConcentrations(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  try
  {
    auto concentrations = state_->GetConcentrations();
    Napi::Object result = Napi::Object::New(env);

    for (const auto& pair : concentrations)
    {
      Napi::Array arr = Napi::Array::New(env, pair.second.size());
      for (size_t i = 0; i < pair.second.size(); ++i)
      {
        arr.Set(i, Napi::Number::New(env, pair.second[i]));
      }
      result.Set(pair.first, arr);
    }

    return result;
  }
  catch (const std::exception& e)
  {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value StateClass::SetUserDefinedRateParameters(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 1 || !info[0].IsObject())
  {
    Napi::TypeError::New(env, "Expected object with rate parameters").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  Napi::Object params_obj = info[0].As<Napi::Object>();
  std::map<std::string, std::vector<double>> params;

  Napi::Array keys = params_obj.GetPropertyNames();
  for (uint32_t i = 0; i < keys.Length(); ++i)
  {
    std::string key = keys.Get(i).As<Napi::String>().Utf8Value();
    Napi::Value value = params_obj.Get(key);

    std::vector<double> values;
    if (value.IsNumber())
    {
      values.push_back(value.As<Napi::Number>().DoubleValue());
    }
    else if (value.IsArray())
    {
      Napi::Array arr = value.As<Napi::Array>();
      for (uint32_t j = 0; j < arr.Length(); ++j)
      {
        values.push_back(arr.Get(j).As<Napi::Number>().DoubleValue());
      }
    }
    params[key] = values;
  }

  try
  {
    state_->SetUserDefinedRateParameters(params);
  }
  catch (const std::exception& e)
  {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
  }

  return env.Undefined();
}

Napi::Value StateClass::GetUserDefinedRateParameters(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  try
  {
    auto params = state_->GetUserDefinedRateParameters();
    Napi::Object result = Napi::Object::New(env);

    for (const auto& pair : params)
    {
      Napi::Array arr = Napi::Array::New(env, pair.second.size());
      for (size_t i = 0; i < pair.second.size(); ++i)
      {
        arr.Set(i, Napi::Number::New(env, pair.second[i]));
      }
      result.Set(pair.first, arr);
    }

    return result;
  }
  catch (const std::exception& e)
  {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value StateClass::SetConditions(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  if (info.Length() < 1 || !info[0].IsObject())
  {
    Napi::TypeError::New(env, "Expected object with conditions").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  Napi::Object cond_obj = info[0].As<Napi::Object>();

  std::vector<double>* temperatures = nullptr;
  std::vector<double>* pressures = nullptr;
  std::vector<double>* air_densities = nullptr;

  std::vector<double> temp_temps, temp_press, temp_air;

  if (cond_obj.Has("temperatures"))
  {
    Napi::Value val = cond_obj.Get("temperatures");
    if (val.IsNumber())
    {
      temp_temps.push_back(val.As<Napi::Number>().DoubleValue());
    }
    else if (val.IsArray())
    {
      Napi::Array arr = val.As<Napi::Array>();
      for (uint32_t i = 0; i < arr.Length(); ++i)
      {
        temp_temps.push_back(arr.Get(i).As<Napi::Number>().DoubleValue());
      }
    }
    if (!temp_temps.empty())
      temperatures = &temp_temps;
  }

  if (cond_obj.Has("pressures"))
  {
    Napi::Value val = cond_obj.Get("pressures");
    if (val.IsNumber())
    {
      temp_press.push_back(val.As<Napi::Number>().DoubleValue());
    }
    else if (val.IsArray())
    {
      Napi::Array arr = val.As<Napi::Array>();
      for (uint32_t i = 0; i < arr.Length(); ++i)
      {
        temp_press.push_back(arr.Get(i).As<Napi::Number>().DoubleValue());
      }
    }
    if (!temp_press.empty())
      pressures = &temp_press;
  }

  if (cond_obj.Has("air_densities"))
  {
    Napi::Value val = cond_obj.Get("air_densities");
    if (val.IsNumber())
    {
      temp_air.push_back(val.As<Napi::Number>().DoubleValue());
    }
    else if (val.IsArray())
    {
      Napi::Array arr = val.As<Napi::Array>();
      for (uint32_t i = 0; i < arr.Length(); ++i)
      {
        temp_air.push_back(arr.Get(i).As<Napi::Number>().DoubleValue());
      }
    }
    if (!temp_air.empty())
      air_densities = &temp_air;
  }

  try
  {
    state_->SetConditions(temperatures, pressures, air_densities);
  }
  catch (const std::exception& e)
  {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
  }

  return env.Undefined();
}

Napi::Value StateClass::GetConditions(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  try
  {
    auto conditions = state_->GetConditions();
    Napi::Object result = Napi::Object::New(env);

    for (const auto& pair : conditions)
    {
      Napi::Array arr = Napi::Array::New(env, pair.second.size());
      for (size_t i = 0; i < pair.second.size(); ++i)
      {
        arr.Set(i, Napi::Number::New(env, pair.second[i]));
      }
      result.Set(pair.first, arr);
    }

    return result;
  }
  catch (const std::exception& e)
  {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value StateClass::GetSpeciesOrdering(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  try
  {
    auto ordering = state_->GetSpeciesOrdering();
    Napi::Object result = Napi::Object::New(env);

    for (const auto& pair : ordering)
    {
      result.Set(pair.first, Napi::Number::New(env, pair.second));
    }

    return result;
  }
  catch (const std::exception& e)
  {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value StateClass::GetUserDefinedRateParametersOrdering(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  try
  {
    auto ordering = state_->GetUserDefinedRateParametersOrdering();
    Napi::Object result = Napi::Object::New(env);

    for (const auto& pair : ordering)
    {
      result.Set(pair.first, Napi::Number::New(env, pair.second));
    }

    return result;
  }
  catch (const std::exception& e)
  {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value StateClass::GetNumberOfGridCells(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();
  return Napi::Number::New(env, state_->GetNumberOfGridCells());
}

Napi::Value StateClass::ConcentrationStrides(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  size_t cell_stride, species_stride;
  state_->GetConcentrationStrides(cell_stride, species_stride);

  Napi::Array result = Napi::Array::New(env, 2);
  result.Set(uint32_t(0), Napi::Number::New(env, cell_stride));
  result.Set(uint32_t(1), Napi::Number::New(env, species_stride));

  return result;
}

Napi::Value StateClass::UserDefinedRateParameterStrides(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();

  size_t cell_stride, param_stride;
  state_->GetUserDefinedRateParameterStrides(cell_stride, param_stride);

  Napi::Array result = Napi::Array::New(env, 2);
  result.Set(uint32_t(0), Napi::Number::New(env, cell_stride));
  result.Set(uint32_t(1), Napi::Number::New(env, param_stride));

  return result;
}

Napi::FunctionReference StateClass::g_StateConstructor;
Napi::Object StateClass::Init(Napi::Env env, Napi::Object exports)
{
  Napi::Function func = DefineClass(
      env,
      "State",
      {
          InstanceMethod("setConcentrations", &StateClass::SetConcentrations),
          InstanceMethod("getConcentrations", &StateClass::GetConcentrations),
          InstanceMethod("setUserDefinedRateParameters", &StateClass::SetUserDefinedRateParameters),
          InstanceMethod("getUserDefinedRateParameters", &StateClass::GetUserDefinedRateParameters),
          InstanceMethod("setConditions", &StateClass::SetConditions),
          InstanceMethod("getConditions", &StateClass::GetConditions),
          InstanceMethod("getSpeciesOrdering", &StateClass::GetSpeciesOrdering),
          InstanceMethod("getUserDefinedRateParametersOrdering", &StateClass::GetUserDefinedRateParametersOrdering),
          InstanceMethod("getNumberOfGridCells", &StateClass::GetNumberOfGridCells),
          InstanceMethod("concentrationStrides", &StateClass::ConcentrationStrides),
          InstanceMethod("userDefinedRateParameterStrides", &StateClass::UserDefinedRateParameterStrides),
      });

  g_StateConstructor = Napi::Persistent(func);
  g_StateConstructor.SuppressDestruct();

  exports.Set("State", func);
  return exports;
}
