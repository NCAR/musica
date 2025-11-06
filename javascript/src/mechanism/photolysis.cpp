// Copyright (C) 2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#include "photolysis.h"
#include "reaction_component.h"
#include "species.h"
#include "phase.h"
#include <napi.h>
#include <mechanism_configuration/v1/reaction_types.hpp>

namespace musica
{

  Photolysis::Photolysis(const Napi::CallbackInfo& info)
      : Napi::ObjectWrap<Photolysis>(info)
  {
    Napi::Env env = info.Env();

    // Initialize the internal photolysis struct
    photolysis_ = mechanism_configuration::v1::types::Photolysis();

    // If constructor called with options object
    if (info.Length() > 0 && info[0].IsObject())
    {
      Napi::Object options = info[0].As<Napi::Object>();

      // Set name (optional)
      if (options.Has("name"))
      {
        photolysis_.name = options.Get("name").As<Napi::String>().Utf8Value();
      }

      // Set scaling_factor (defaults to 1.0)
      if (options.Has("scaling_factor"))
      {
        photolysis_.scaling_factor = options.Get("scaling_factor").As<Napi::Number>().DoubleValue();
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
            photolysis_.gas_phase = phase_wrapper->GetInternalPhase().name;
          }
          else if (phase_obj.Has("name"))
          {
            photolysis_.gas_phase = phase_obj.Get("name").As<Napi::String>().Utf8Value();
          }
        }
        // If it's a string
        else if (gas_phase_val.IsString())
        {
          photolysis_.gas_phase = gas_phase_val.As<Napi::String>().Utf8Value();
        }
      }

      // Set reactants (optional)
      if (options.Has("reactants") && options.Get("reactants").IsArray())
      {
        Napi::Array reactants_arr = options.Get("reactants").As<Napi::Array>();

        for (uint32_t i = 0; i < reactants_arr.Length(); i++)
        {
          Napi::Value item = reactants_arr.Get(i);

          // If it's a ReactionComponent object
          if (item.IsObject())
          {
            Napi::Object comp_obj = item.As<Napi::Object>();

            if (comp_obj.InstanceOf(ReactionComponent::GetClass(env)))
            {
              ReactionComponent* comp_wrapper = ReactionComponent::Unwrap(comp_obj);
              photolysis_.reactants.push_back(comp_wrapper->GetInternalComponent());
            }
            // If it's a Species object
            else if (comp_obj.InstanceOf(Species::GetClass(env)))
            {
              Species* species_wrapper = Species::Unwrap(comp_obj);
              mechanism_configuration::v1::types::ReactionComponent comp;
              comp.species_name = species_wrapper->GetInternalSpecies().name;
              comp.coefficient = 1.0;
              photolysis_.reactants.push_back(comp);
            }
            // If it's a plain object with species_name
            else if (comp_obj.Has("species_name"))
            {
              mechanism_configuration::v1::types::ReactionComponent comp;
              comp.species_name = comp_obj.Get("species_name").As<Napi::String>().Utf8Value();
              comp.coefficient = comp_obj.Has("coefficient") ? comp_obj.Get("coefficient").As<Napi::Number>().DoubleValue()
                                                              : 1.0;
              photolysis_.reactants.push_back(comp);
            }
          }
          // If it's a string (species name)
          else if (item.IsString())
          {
            mechanism_configuration::v1::types::ReactionComponent comp;
            comp.species_name = item.As<Napi::String>().Utf8Value();
            comp.coefficient = 1.0;
            photolysis_.reactants.push_back(comp);
          }
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
              photolysis_.products.push_back(comp_wrapper->GetInternalComponent());
            }
            // If it's a Species object
            else if (comp_obj.InstanceOf(Species::GetClass(env)))
            {
              Species* species_wrapper = Species::Unwrap(comp_obj);
              mechanism_configuration::v1::types::ReactionComponent comp;
              comp.species_name = species_wrapper->GetInternalSpecies().name;
              comp.coefficient = 1.0;
              photolysis_.products.push_back(comp);
            }
            // If it's a plain object with species_name
            else if (comp_obj.Has("species_name"))
            {
              mechanism_configuration::v1::types::ReactionComponent comp;
              comp.species_name = comp_obj.Get("species_name").As<Napi::String>().Utf8Value();
              comp.coefficient = comp_obj.Has("coefficient") ? comp_obj.Get("coefficient").As<Napi::Number>().DoubleValue()
                                                              : 1.0;
              photolysis_.products.push_back(comp);
            }
          }
          // If it's a string (species name)
          else if (item.IsString())
          {
            mechanism_configuration::v1::types::ReactionComponent comp;
            comp.species_name = item.As<Napi::String>().Utf8Value();
            comp.coefficient = 1.0;
            photolysis_.products.push_back(comp);
          }
        }
      }
    }
  }

  Napi::Value Photolysis::GetName(const Napi::CallbackInfo& info)
  {
    return Napi::String::New(info.Env(), photolysis_.name);
  }

  void Photolysis::SetName(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    photolysis_.name = value.As<Napi::String>().Utf8Value();
  }

  Napi::Value Photolysis::GetScalingFactor(const Napi::CallbackInfo& info)
  {
    return Napi::Number::New(info.Env(), photolysis_.scaling_factor);
  }

  void Photolysis::SetScalingFactor(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    photolysis_.scaling_factor = value.As<Napi::Number>().DoubleValue();
  }

  Napi::Value Photolysis::GetGasPhase(const Napi::CallbackInfo& info)
  {
    return Napi::String::New(info.Env(), photolysis_.gas_phase);
  }

  void Photolysis::SetGasPhase(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    if (value.IsString())
    {
      photolysis_.gas_phase = value.As<Napi::String>().Utf8Value();
    }
    else if (value.IsObject())
    {
      Napi::Env env = info.Env();
      Napi::Object phase_obj = value.As<Napi::Object>();

      if (phase_obj.InstanceOf(Phase::GetClass(env)))
      {
        Phase* phase_wrapper = Phase::Unwrap(phase_obj);
        photolysis_.gas_phase = phase_wrapper->GetInternalPhase().name;
      }
    }
  }

  Napi::Value Photolysis::GetReactants(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();
    Napi::Array result = Napi::Array::New(env, photolysis_.reactants.size());

    for (size_t i = 0; i < photolysis_.reactants.size(); i++)
    {
      Napi::Object comp_obj = Napi::Object::New(env);
      comp_obj.Set("species_name", photolysis_.reactants[i].species_name);
      comp_obj.Set("coefficient", photolysis_.reactants[i].coefficient);
      result.Set(uint32_t(i), comp_obj);
    }

    return result;
  }

  void Photolysis::SetReactants(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    Napi::Env env = info.Env();

    if (value.IsArray())
    {
      photolysis_.reactants.clear();
      Napi::Array reactants_arr = value.As<Napi::Array>();

      for (uint32_t i = 0; i < reactants_arr.Length(); i++)
      {
        Napi::Value item = reactants_arr.Get(i);

        if (item.IsObject())
        {
          Napi::Object comp_obj = item.As<Napi::Object>();

          if (comp_obj.InstanceOf(ReactionComponent::GetClass(env)))
          {
            ReactionComponent* comp_wrapper = ReactionComponent::Unwrap(comp_obj);
            photolysis_.reactants.push_back(comp_wrapper->GetInternalComponent());
          }
          else if (comp_obj.InstanceOf(Species::GetClass(env)))
          {
            Species* species_wrapper = Species::Unwrap(comp_obj);
            mechanism_configuration::v1::types::ReactionComponent comp;
            comp.species_name = species_wrapper->GetInternalSpecies().name;
            comp.coefficient = 1.0;
            photolysis_.reactants.push_back(comp);
          }
        }
        else if (item.IsString())
        {
          mechanism_configuration::v1::types::ReactionComponent comp;
          comp.species_name = item.As<Napi::String>().Utf8Value();
          comp.coefficient = 1.0;
          photolysis_.reactants.push_back(comp);
        }
      }
    }
  }

  Napi::Value Photolysis::GetProducts(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();
    Napi::Array result = Napi::Array::New(env, photolysis_.products.size());

    for (size_t i = 0; i < photolysis_.products.size(); i++)
    {
      Napi::Object comp_obj = Napi::Object::New(env);
      comp_obj.Set("species_name", photolysis_.products[i].species_name);
      comp_obj.Set("coefficient", photolysis_.products[i].coefficient);
      result.Set(uint32_t(i), comp_obj);
    }

    return result;
  }

  void Photolysis::SetProducts(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    Napi::Env env = info.Env();

    if (value.IsArray())
    {
      photolysis_.products.clear();
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
            photolysis_.products.push_back(comp_wrapper->GetInternalComponent());
          }
          else if (comp_obj.InstanceOf(Species::GetClass(env)))
          {
            Species* species_wrapper = Species::Unwrap(comp_obj);
            mechanism_configuration::v1::types::ReactionComponent comp;
            comp.species_name = species_wrapper->GetInternalSpecies().name;
            comp.coefficient = 1.0;
            photolysis_.products.push_back(comp);
          }
        }
        else if (item.IsString())
        {
          mechanism_configuration::v1::types::ReactionComponent comp;
          comp.species_name = item.As<Napi::String>().Utf8Value();
          comp.coefficient = 1.0;
          photolysis_.products.push_back(comp);
        }
      }
    }
  }

  Napi::Value Photolysis::Serialize(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);

    // Add type
    result.Set("type", "PHOTOLYSIS");

    // Add name
    if (!photolysis_.name.empty())
    {
      result.Set("name", photolysis_.name);
    }

    // Add scaling factor
    result.Set("scaling factor", photolysis_.scaling_factor);

    // Add gas phase
    if (!photolysis_.gas_phase.empty())
    {
      result.Set("gas phase", photolysis_.gas_phase);
    }

    // Add reactants
    Napi::Array reactants_arr = Napi::Array::New(env, photolysis_.reactants.size());
    for (size_t i = 0; i < photolysis_.reactants.size(); i++)
    {
      Napi::Object comp_obj = Napi::Object::New(env);
      comp_obj.Set(photolysis_.reactants[i].species_name, photolysis_.reactants[i].coefficient);
      reactants_arr.Set(uint32_t(i), comp_obj);
    }
    result.Set("reactants", reactants_arr);

    // Add products
    Napi::Array products_arr = Napi::Array::New(env, photolysis_.products.size());
    for (size_t i = 0; i < photolysis_.products.size(); i++)
    {
      Napi::Object comp_obj = Napi::Object::New(env);
      comp_obj.Set(photolysis_.products[i].species_name, photolysis_.products[i].coefficient);
      products_arr.Set(uint32_t(i), comp_obj);
    }
    result.Set("products", products_arr);

    return result;
  }

  const mechanism_configuration::v1::types::Photolysis& Photolysis::GetInternalPhotolysis() const
  {
    return photolysis_;
  }

  Napi::Function Photolysis::GetClass(Napi::Env env)
  {
    return DefineClass(
        env,
        "Photolysis",
        {
            InstanceAccessor("name", &Photolysis::GetName, &Photolysis::SetName),
            InstanceAccessor("scaling_factor", &Photolysis::GetScalingFactor, &Photolysis::SetScalingFactor),
            InstanceAccessor("gas_phase", &Photolysis::GetGasPhase, &Photolysis::SetGasPhase),
            InstanceAccessor("reactants", &Photolysis::GetReactants, &Photolysis::SetReactants),
            InstanceAccessor("products", &Photolysis::GetProducts, &Photolysis::SetProducts),
            InstanceMethod("serialize", &Photolysis::Serialize),
        });
  }

}  // namespace musica
