// Copyright (C) 2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#include "species.h"
#include <napi.h>
#include <mechanism_configuration/v1/types.hpp>

namespace musica
{

  Species::Species(const Napi::CallbackInfo& info)
      : Napi::ObjectWrap<Species>(info)
  {
    Napi::Env env = info.Env();

    // Initialize the internal species struct
    species_ = mechanism_configuration::v1::types::Species();

    // If constructor called with options object
    if (info.Length() > 0 && info[0].IsObject())
    {
      Napi::Object options = info[0].As<Napi::Object>();

      // Set name (required)
      if (options.Has("name"))
      {
        species_.name = options.Get("name").As<Napi::String>().Utf8Value();
      }

      // Set molecular_weight_kg_mol (optional)
      if (options.Has("molecular_weight_kg_mol"))
      {
        species_.molecular_weight = options.Get("molecular_weight_kg_mol").As<Napi::Number>().DoubleValue();
      }

      // Set constant_concentration_mol_m3 (optional)
      if (options.Has("constant_concentration_mol_m3"))
      {
        species_.constant_concentration = options.Get("constant_concentration_mol_m3").As<Napi::Number>().DoubleValue();
      }

      // Set constant_mixing_ratio_mol_mol (optional)
      if (options.Has("constant_mixing_ratio_mol_mol"))
      {
        species_.constant_mixing_ratio = options.Get("constant_mixing_ratio_mol_mol").As<Napi::Number>().DoubleValue();
      }

      // Set is_third_body (optional, defaults to false)
      if (options.Has("is_third_body"))
      {
        species_.is_third_body = options.Get("is_third_body").As<Napi::Boolean>().Value();
      }

      // Set other_properties (optional)
      if (options.Has("other_properties") && options.Get("other_properties").IsObject())
      {
        Napi::Object other_props = options.Get("other_properties").As<Napi::Object>();
        Napi::Array prop_names = other_props.GetPropertyNames();

        for (uint32_t i = 0; i < prop_names.Length(); i++)
        {
          std::string key = prop_names.Get(i).As<Napi::String>().Utf8Value();
          std::string value = other_props.Get(key).As<Napi::String>().Utf8Value();
          species_.unknown_properties[key] = value;
        }
      }
    }
  }

  Napi::Value Species::GetName(const Napi::CallbackInfo& info)
  {
    return Napi::String::New(info.Env(), species_.name);
  }

  void Species::SetName(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    species_.name = value.As<Napi::String>().Utf8Value();
  }

  Napi::Value Species::GetMolecularWeight(const Napi::CallbackInfo& info)
  {
    if (species_.molecular_weight.has_value())
    {
      return Napi::Number::New(info.Env(), species_.molecular_weight.value());
    }
    return info.Env().Undefined();
  }

  void Species::SetMolecularWeight(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    if (value.IsNumber())
    {
      species_.molecular_weight = value.As<Napi::Number>().DoubleValue();
    }
    else
    {
      species_.molecular_weight = std::nullopt;
    }
  }

  Napi::Value Species::GetConstantConcentration(const Napi::CallbackInfo& info)
  {
    if (species_.constant_concentration.has_value())
    {
      return Napi::Number::New(info.Env(), species_.constant_concentration.value());
    }
    return info.Env().Undefined();
  }

  void Species::SetConstantConcentration(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    if (value.IsNumber())
    {
      species_.constant_concentration = value.As<Napi::Number>().DoubleValue();
    }
    else
    {
      species_.constant_concentration = std::nullopt;
    }
  }

  Napi::Value Species::GetConstantMixingRatio(const Napi::CallbackInfo& info)
  {
    if (species_.constant_mixing_ratio.has_value())
    {
      return Napi::Number::New(info.Env(), species_.constant_mixing_ratio.value());
    }
    return info.Env().Undefined();
  }

  void Species::SetConstantMixingRatio(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    if (value.IsNumber())
    {
      species_.constant_mixing_ratio = value.As<Napi::Number>().DoubleValue();
    }
    else
    {
      species_.constant_mixing_ratio = std::nullopt;
    }
  }

  Napi::Value Species::GetIsThirdBody(const Napi::CallbackInfo& info)
  {
    if (species_.is_third_body.has_value())
    {
      return Napi::Boolean::New(info.Env(), species_.is_third_body.value());
    }
    return Napi::Boolean::New(info.Env(), false);
  }

  void Species::SetIsThirdBody(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    if (value.IsBoolean())
    {
      species_.is_third_body = value.As<Napi::Boolean>().Value();
    }
  }

  Napi::Value Species::GetOtherProperties(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);

    for (const auto& [key, value] : species_.unknown_properties)
    {
      result.Set(key, value);
    }

    return result;
  }

  void Species::SetOtherProperties(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    if (value.IsObject())
    {
      species_.unknown_properties.clear();
      Napi::Object obj = value.As<Napi::Object>();
      Napi::Array prop_names = obj.GetPropertyNames();

      for (uint32_t i = 0; i < prop_names.Length(); i++)
      {
        std::string key = prop_names.Get(i).As<Napi::String>().Utf8Value();
        std::string val = obj.Get(key).As<Napi::String>().Utf8Value();
        species_.unknown_properties[key] = val;
      }
    }
  }

  Napi::Value Species::Serialize(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);

    // Add name
    result.Set("name", species_.name);

    // Add molecular weight (if set)
    if (species_.molecular_weight.has_value())
    {
      result.Set("molecular weight [kg mol-1]", species_.molecular_weight.value());
    }

    // Add constant concentration (if set)
    if (species_.constant_concentration.has_value())
    {
      result.Set("constant concentration [mol m-3]", species_.constant_concentration.value());
    }

    // Add constant mixing ratio (if set)
    if (species_.constant_mixing_ratio.has_value())
    {
      result.Set("constant mixing ratio [mol mol-1]", species_.constant_mixing_ratio.value());
    }

    // Add is_third_body (if set and true)
    if (species_.is_third_body.has_value() && species_.is_third_body.value())
    {
      result.Set("is third body", true);
    }

    // Add other properties
    for (const auto& [key, value] : species_.unknown_properties)
    {
      result.Set(key, value);
    }

    return result;
  }

  const mechanism_configuration::v1::types::Species& Species::GetInternalSpecies() const
  {
    return species_;
  }

  Napi::Function Species::GetClass(Napi::Env env)
  {
    return DefineClass(
        env,
        "Species",
        {
            InstanceAccessor("name", &Species::GetName, &Species::SetName),
            InstanceAccessor("molecular_weight_kg_mol", &Species::GetMolecularWeight, &Species::SetMolecularWeight),
            InstanceAccessor(
                "constant_concentration_mol_m3", &Species::GetConstantConcentration, &Species::SetConstantConcentration),
            InstanceAccessor(
                "constant_mixing_ratio_mol_mol", &Species::GetConstantMixingRatio, &Species::SetConstantMixingRatio),
            InstanceAccessor("is_third_body", &Species::GetIsThirdBody, &Species::SetIsThirdBody),
            InstanceAccessor("other_properties", &Species::GetOtherProperties, &Species::SetOtherProperties),
            InstanceMethod("serialize", &Species::Serialize),
        });
  }

}  // namespace musica
