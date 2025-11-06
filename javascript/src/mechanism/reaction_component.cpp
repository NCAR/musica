// Copyright (C) 2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#include "reaction_component.h"
#include <napi.h>
#include <mechanism_configuration/v1/types.hpp>

namespace musica
{

  ReactionComponent::ReactionComponent(const Napi::CallbackInfo& info)
      : Napi::ObjectWrap<ReactionComponent>(info)
  {
    Napi::Env env = info.Env();

    // Initialize the internal component struct
    component_ = mechanism_configuration::v1::types::ReactionComponent();

    // If constructor called with options object
    if (info.Length() > 0 && info[0].IsObject())
    {
      Napi::Object options = info[0].As<Napi::Object>();

      // Set species_name (required)
      if (options.Has("species_name"))
      {
        component_.species_name = options.Get("species_name").As<Napi::String>().Utf8Value();
      }

      // Set coefficient (optional, defaults to 1.0)
      if (options.Has("coefficient"))
      {
        component_.coefficient = options.Get("coefficient").As<Napi::Number>().DoubleValue();
      }
    }
    // If constructor called with just a string (species name)
    else if (info.Length() > 0 && info[0].IsString())
    {
      component_.species_name = info[0].As<Napi::String>().Utf8Value();
      component_.coefficient = 1.0;

      // Optional second argument for coefficient
      if (info.Length() > 1 && info[1].IsNumber())
      {
        component_.coefficient = info[1].As<Napi::Number>().DoubleValue();
      }
    }
  }

  Napi::Value ReactionComponent::GetSpeciesName(const Napi::CallbackInfo& info)
  {
    return Napi::String::New(info.Env(), component_.species_name);
  }

  void ReactionComponent::SetSpeciesName(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    component_.species_name = value.As<Napi::String>().Utf8Value();
  }

  Napi::Value ReactionComponent::GetCoefficient(const Napi::CallbackInfo& info)
  {
    return Napi::Number::New(info.Env(), component_.coefficient);
  }

  void ReactionComponent::SetCoefficient(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    component_.coefficient = value.As<Napi::Number>().DoubleValue();
  }

  Napi::Value ReactionComponent::Serialize(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);

    // Add species name
    result.Set(component_.species_name, component_.coefficient);

    return result;
  }

  const mechanism_configuration::v1::types::ReactionComponent& ReactionComponent::GetInternalComponent() const
  {
    return component_;
  }

  Napi::Function ReactionComponent::GetClass(Napi::Env env)
  {
    return DefineClass(
        env,
        "ReactionComponent",
        {
            InstanceAccessor("species_name", &ReactionComponent::GetSpeciesName, &ReactionComponent::SetSpeciesName),
            InstanceAccessor("coefficient", &ReactionComponent::GetCoefficient, &ReactionComponent::SetCoefficient),
            InstanceMethod("serialize", &ReactionComponent::Serialize),
        });
  }

}  // namespace musica
