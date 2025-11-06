// Copyright (C) 2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#include "emission.h"
#include "reaction_component.h"
#include "species.h"
#include "phase.h"
#include <napi.h>
#include <mechanism_configuration/v1/reaction_types.hpp>

namespace musica
{

  Emission::Emission(const Napi::CallbackInfo& info)
      : Napi::ObjectWrap<Emission>(info)
  {
    Napi::Env env = info.Env();

    // Initialize the internal emission struct
    emission_ = mechanism_configuration::v1::types::Emission();

    // If constructor called with options object
    if (info.Length() > 0 && info[0].IsObject())
    {
      Napi::Object options = info[0].As<Napi::Object>();

      // Set name (optional)
      if (options.Has("name"))
      {
        emission_.name = options.Get("name").As<Napi::String>().Utf8Value();
      }

      // Set scaling_factor (defaults to 1.0)
      if (options.Has("scaling_factor"))
      {
        emission_.scaling_factor = options.Get("scaling_factor").As<Napi::Number>().DoubleValue();
      }

      // Set gas_phase (optional)
      if (options.Has("gas_phase"))
      {
        Napi::Value gas_phase_val = options.Get("gas_phase");

        // If it's a Phase object
        if (gas_phase_val.IsObject())
        {
          Napi::Object phase_obj = gas_phase_val.As<Napi::Object>();

          if (phase_obj.InstanceOf(Phase::GetClass(env)))
          {
            Phase* phase_wrapper = Phase::Unwrap(phase_obj);
            emission_.gas_phase = phase_wrapper->GetInternalPhase().name;
          }
          else if (phase_obj.Has("name"))
          {
            emission_.gas_phase = phase_obj.Get("name").As<Napi::String>().Utf8Value();
          }
        }
        // If it's a string
        else if (gas_phase_val.IsString())
        {
          emission_.gas_phase = gas_phase_val.As<Napi::String>().Utf8Value();
        }
      }

      // Set products (optional)
      if (options.Has("products") && options.Get("products").IsArray())
      {
        Napi::Array products_arr = options.Get("products").As<Napi::Array>();

        for (uint32_t i = 0; i < products_arr.Length(); i++)
        {
          Napi::Value item = products_arr.Get(i);

          // If it's a ReactionComponent object
          if (item.IsObject())
          {
            Napi::Object comp_obj = item.As<Napi::Object>();

            if (comp_obj.InstanceOf(ReactionComponent::GetClass(env)))
            {
              ReactionComponent* comp_wrapper = ReactionComponent::Unwrap(comp_obj);
              emission_.products.push_back(comp_wrapper->GetInternalComponent());
            }
            // If it's a Species object
            else if (comp_obj.InstanceOf(Species::GetClass(env)))
            {
              Species* species_wrapper = Species::Unwrap(comp_obj);
              mechanism_configuration::v1::types::ReactionComponent comp;
              comp.species_name = species_wrapper->GetInternalSpecies().name;
              comp.coefficient = 1.0;
              emission_.products.push_back(comp);
            }
            // If it's a plain object with species_name
            else if (comp_obj.Has("species_name"))
            {
              mechanism_configuration::v1::types::ReactionComponent comp;
              comp.species_name = comp_obj.Get("species_name").As<Napi::String>().Utf8Value();
              comp.coefficient = comp_obj.Has("coefficient") ? comp_obj.Get("coefficient").As<Napi::Number>().DoubleValue()
                                                              : 1.0;
              emission_.products.push_back(comp);
            }
          }
          // If it's a string (species name)
          else if (item.IsString())
          {
            mechanism_configuration::v1::types::ReactionComponent comp;
            comp.species_name = item.As<Napi::String>().Utf8Value();
            comp.coefficient = 1.0;
            emission_.products.push_back(comp);
          }
        }
      }
    }
  }

  Napi::Value Emission::GetName(const Napi::CallbackInfo& info)
  {
    return Napi::String::New(info.Env(), emission_.name);
  }

  void Emission::SetName(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    emission_.name = value.As<Napi::String>().Utf8Value();
  }

  Napi::Value Emission::GetScalingFactor(const Napi::CallbackInfo& info)
  {
    return Napi::Number::New(info.Env(), emission_.scaling_factor);
  }

  void Emission::SetScalingFactor(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    emission_.scaling_factor = value.As<Napi::Number>().DoubleValue();
  }

  Napi::Value Emission::GetProducts(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();
    Napi::Array result = Napi::Array::New(env, emission_.products.size());

    for (size_t i = 0; i < emission_.products.size(); i++)
    {
      Napi::Object comp_obj = Napi::Object::New(env);
      comp_obj.Set("species_name", emission_.products[i].species_name);
      comp_obj.Set("coefficient", emission_.products[i].coefficient);
      result.Set(uint32_t(i), comp_obj);
    }

    return result;
  }

  void Emission::SetProducts(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    Napi::Env env = info.Env();

    if (value.IsArray())
    {
      emission_.products.clear();
      Napi::Array products_arr = value.As<Napi::Array>();

      for (uint32_t i = 0; i < products_arr.Length(); i++)
      {
        Napi::Value item = products_arr.Get(i);

        if (item.IsObject())
        {
          Napi::Object comp_obj = item.As<Napi::Object>();

          if (comp_obj.InstanceOf(ReactionComponent::GetClass(env)))
          {
            ReactionComponent* comp_wrapper = ReactionComponent::Unwrap(comp_obj);
            emission_.products.push_back(comp_wrapper->GetInternalComponent());
          }
          else if (comp_obj.InstanceOf(Species::GetClass(env)))
          {
            Species* species_wrapper = Species::Unwrap(comp_obj);
            mechanism_configuration::v1::types::ReactionComponent comp;
            comp.species_name = species_wrapper->GetInternalSpecies().name;
            comp.coefficient = 1.0;
            emission_.products.push_back(comp);
          }
        }
        else if (item.IsString())
        {
          mechanism_configuration::v1::types::ReactionComponent comp;
          comp.species_name = item.As<Napi::String>().Utf8Value();
          comp.coefficient = 1.0;
          emission_.products.push_back(comp);
        }
      }
    }
  }

  Napi::Value Emission::GetGasPhase(const Napi::CallbackInfo& info)
  {
    return Napi::String::New(info.Env(), emission_.gas_phase);
  }

  void Emission::SetGasPhase(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    if (value.IsString())
    {
      emission_.gas_phase = value.As<Napi::String>().Utf8Value();
    }
    else if (value.IsObject())
    {
      Napi::Env env = info.Env();
      Napi::Object phase_obj = value.As<Napi::Object>();

      if (phase_obj.InstanceOf(Phase::GetClass(env)))
      {
        Phase* phase_wrapper = Phase::Unwrap(phase_obj);
        emission_.gas_phase = phase_wrapper->GetInternalPhase().name;
      }
    }
  }

  Napi::Value Emission::Serialize(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);

    // Add type
    result.Set("type", "EMISSION");

    // Add name
    if (!emission_.name.empty())
    {
      result.Set("name", emission_.name);
    }

    // Add scaling factor
    result.Set("scaling factor", emission_.scaling_factor);

    // Add gas phase
    if (!emission_.gas_phase.empty())
    {
      result.Set("gas phase", emission_.gas_phase);
    }

    // Add products
    Napi::Array products_arr = Napi::Array::New(env, emission_.products.size());
    for (size_t i = 0; i < emission_.products.size(); i++)
    {
      Napi::Object comp_obj = Napi::Object::New(env);
      comp_obj.Set(emission_.products[i].species_name, emission_.products[i].coefficient);
      products_arr.Set(uint32_t(i), comp_obj);
    }
    result.Set("products", products_arr);

    return result;
  }

  const mechanism_configuration::v1::types::Emission& Emission::GetInternalEmission() const
  {
    return emission_;
  }

  Napi::Function Emission::GetClass(Napi::Env env)
  {
    return DefineClass(
        env,
        "Emission",
        {
            InstanceAccessor("name", &Emission::GetName, &Emission::SetName),
            InstanceAccessor("scaling_factor", &Emission::GetScalingFactor, &Emission::SetScalingFactor),
            InstanceAccessor("gas_phase", &Emission::GetGasPhase, &Emission::SetGasPhase),
            InstanceAccessor("products", &Emission::GetProducts, &Emission::SetProducts),
            InstanceMethod("serialize", &Emission::Serialize),
        });
  }

}  // namespace musica
