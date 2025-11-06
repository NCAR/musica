// Copyright (C) 2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#include "arrhenius.h"
#include "reaction_component.h"
#include "species.h"
#include "phase.h"
#include <napi.h>
#include <mechanism_configuration/v1/reaction_types.hpp>

namespace musica
{

  Arrhenius::Arrhenius(const Napi::CallbackInfo& info)
      : Napi::ObjectWrap<Arrhenius>(info)
  {
    Napi::Env env = info.Env();

    // Initialize the internal arrhenius struct
    arrhenius_ = mechanism_configuration::v1::types::Arrhenius();

    // If constructor called with options object
    if (info.Length() > 0 && info[0].IsObject())
    {
      Napi::Object options = info[0].As<Napi::Object>();

      // Set name (optional)
      if (options.Has("name"))
      {
        arrhenius_.name = options.Get("name").As<Napi::String>().Utf8Value();
      }

      // Set A (pre-exponential factor, defaults to 1)
      if (options.Has("A"))
      {
        arrhenius_.A = options.Get("A").As<Napi::Number>().DoubleValue();
      }

      // Set B (temperature exponent, defaults to 0)
      if (options.Has("B"))
      {
        arrhenius_.B = options.Get("B").As<Napi::Number>().DoubleValue();
      }

      // Set C (exponential term, defaults to 0)
      if (options.Has("C"))
      {
        arrhenius_.C = options.Get("C").As<Napi::Number>().DoubleValue();
      }

      // Set D (reference temperature, defaults to 300)
      if (options.Has("D"))
      {
        arrhenius_.D = options.Get("D").As<Napi::Number>().DoubleValue();
      }

      // Set E (pressure scaling, defaults to 0)
      if (options.Has("E"))
      {
        arrhenius_.E = options.Get("E").As<Napi::Number>().DoubleValue();
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
            arrhenius_.gas_phase = phase_wrapper->GetInternalPhase().name;
          }
          else if (phase_obj.Has("name"))
          {
            arrhenius_.gas_phase = phase_obj.Get("name").As<Napi::String>().Utf8Value();
          }
        }
        // If it's a string
        else if (gas_phase_val.IsString())
        {
          arrhenius_.gas_phase = gas_phase_val.As<Napi::String>().Utf8Value();
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
              arrhenius_.reactants.push_back(comp_wrapper->GetInternalComponent());
            }
            // If it's a Species object
            else if (comp_obj.InstanceOf(Species::GetClass(env)))
            {
              Species* species_wrapper = Species::Unwrap(comp_obj);
              mechanism_configuration::v1::types::ReactionComponent comp;
              comp.species_name = species_wrapper->GetInternalSpecies().name;
              comp.coefficient = 1.0;
              arrhenius_.reactants.push_back(comp);
            }
            // If it's a plain object with species_name
            else if (comp_obj.Has("species_name"))
            {
              mechanism_configuration::v1::types::ReactionComponent comp;
              comp.species_name = comp_obj.Get("species_name").As<Napi::String>().Utf8Value();
              comp.coefficient = comp_obj.Has("coefficient") ? comp_obj.Get("coefficient").As<Napi::Number>().DoubleValue()
                                                              : 1.0;
              arrhenius_.reactants.push_back(comp);
            }
          }
          // If it's a string (species name)
          else if (item.IsString())
          {
            mechanism_configuration::v1::types::ReactionComponent comp;
            comp.species_name = item.As<Napi::String>().Utf8Value();
            comp.coefficient = 1.0;
            arrhenius_.reactants.push_back(comp);
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
              arrhenius_.products.push_back(comp_wrapper->GetInternalComponent());
            }
            // If it's a Species object
            else if (comp_obj.InstanceOf(Species::GetClass(env)))
            {
              Species* species_wrapper = Species::Unwrap(comp_obj);
              mechanism_configuration::v1::types::ReactionComponent comp;
              comp.species_name = species_wrapper->GetInternalSpecies().name;
              comp.coefficient = 1.0;
              arrhenius_.products.push_back(comp);
            }
            // If it's a plain object with species_name
            else if (comp_obj.Has("species_name"))
            {
              mechanism_configuration::v1::types::ReactionComponent comp;
              comp.species_name = comp_obj.Get("species_name").As<Napi::String>().Utf8Value();
              comp.coefficient = comp_obj.Has("coefficient") ? comp_obj.Get("coefficient").As<Napi::Number>().DoubleValue()
                                                              : 1.0;
              arrhenius_.products.push_back(comp);
            }
          }
          // If it's a string (species name)
          else if (item.IsString())
          {
            mechanism_configuration::v1::types::ReactionComponent comp;
            comp.species_name = item.As<Napi::String>().Utf8Value();
            comp.coefficient = 1.0;
            arrhenius_.products.push_back(comp);
          }
        }
      }
    }
  }

  Napi::Value Arrhenius::GetName(const Napi::CallbackInfo& info)
  {
    return Napi::String::New(info.Env(), arrhenius_.name);
  }

  void Arrhenius::SetName(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    arrhenius_.name = value.As<Napi::String>().Utf8Value();
  }

  Napi::Value Arrhenius::GetA(const Napi::CallbackInfo& info)
  {
    return Napi::Number::New(info.Env(), arrhenius_.A);
  }

  void Arrhenius::SetA(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    arrhenius_.A = value.As<Napi::Number>().DoubleValue();
  }

  Napi::Value Arrhenius::GetB(const Napi::CallbackInfo& info)
  {
    return Napi::Number::New(info.Env(), arrhenius_.B);
  }

  void Arrhenius::SetB(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    arrhenius_.B = value.As<Napi::Number>().DoubleValue();
  }

  Napi::Value Arrhenius::GetC(const Napi::CallbackInfo& info)
  {
    return Napi::Number::New(info.Env(), arrhenius_.C);
  }

  void Arrhenius::SetC(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    arrhenius_.C = value.As<Napi::Number>().DoubleValue();
  }

  Napi::Value Arrhenius::GetD(const Napi::CallbackInfo& info)
  {
    return Napi::Number::New(info.Env(), arrhenius_.D);
  }

  void Arrhenius::SetD(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    arrhenius_.D = value.As<Napi::Number>().DoubleValue();
  }

  Napi::Value Arrhenius::GetE(const Napi::CallbackInfo& info)
  {
    return Napi::Number::New(info.Env(), arrhenius_.E);
  }

  void Arrhenius::SetE(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    arrhenius_.E = value.As<Napi::Number>().DoubleValue();
  }

  Napi::Value Arrhenius::GetGasPhase(const Napi::CallbackInfo& info)
  {
    return Napi::String::New(info.Env(), arrhenius_.gas_phase);
  }

  void Arrhenius::SetGasPhase(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    if (value.IsString())
    {
      arrhenius_.gas_phase = value.As<Napi::String>().Utf8Value();
    }
    else if (value.IsObject())
    {
      Napi::Env env = info.Env();
      Napi::Object phase_obj = value.As<Napi::Object>();

      if (phase_obj.InstanceOf(Phase::GetClass(env)))
      {
        Phase* phase_wrapper = Phase::Unwrap(phase_obj);
        arrhenius_.gas_phase = phase_wrapper->GetInternalPhase().name;
      }
    }
  }

  Napi::Value Arrhenius::GetReactants(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();
    Napi::Array result = Napi::Array::New(env, arrhenius_.reactants.size());

    for (size_t i = 0; i < arrhenius_.reactants.size(); i++)
    {
      Napi::Object comp_obj = Napi::Object::New(env);
      comp_obj.Set("species_name", arrhenius_.reactants[i].species_name);
      comp_obj.Set("coefficient", arrhenius_.reactants[i].coefficient);
      result.Set(uint32_t(i), comp_obj);
    }

    return result;
  }

  void Arrhenius::SetReactants(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    Napi::Env env = info.Env();

    if (value.IsArray())
    {
      arrhenius_.reactants.clear();
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
            arrhenius_.reactants.push_back(comp_wrapper->GetInternalComponent());
          }
          else if (comp_obj.InstanceOf(Species::GetClass(env)))
          {
            Species* species_wrapper = Species::Unwrap(comp_obj);
            mechanism_configuration::v1::types::ReactionComponent comp;
            comp.species_name = species_wrapper->GetInternalSpecies().name;
            comp.coefficient = 1.0;
            arrhenius_.reactants.push_back(comp);
          }
        }
        else if (item.IsString())
        {
          mechanism_configuration::v1::types::ReactionComponent comp;
          comp.species_name = item.As<Napi::String>().Utf8Value();
          comp.coefficient = 1.0;
          arrhenius_.reactants.push_back(comp);
        }
      }
    }
  }

  Napi::Value Arrhenius::GetProducts(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();
    Napi::Array result = Napi::Array::New(env, arrhenius_.products.size());

    for (size_t i = 0; i < arrhenius_.products.size(); i++)
    {
      Napi::Object comp_obj = Napi::Object::New(env);
      comp_obj.Set("species_name", arrhenius_.products[i].species_name);
      comp_obj.Set("coefficient", arrhenius_.products[i].coefficient);
      result.Set(uint32_t(i), comp_obj);
    }

    return result;
  }

  void Arrhenius::SetProducts(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    Napi::Env env = info.Env();

    if (value.IsArray())
    {
      arrhenius_.products.clear();
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
            arrhenius_.products.push_back(comp_wrapper->GetInternalComponent());
          }
          else if (comp_obj.InstanceOf(Species::GetClass(env)))
          {
            Species* species_wrapper = Species::Unwrap(comp_obj);
            mechanism_configuration::v1::types::ReactionComponent comp;
            comp.species_name = species_wrapper->GetInternalSpecies().name;
            comp.coefficient = 1.0;
            arrhenius_.products.push_back(comp);
          }
        }
        else if (item.IsString())
        {
          mechanism_configuration::v1::types::ReactionComponent comp;
          comp.species_name = item.As<Napi::String>().Utf8Value();
          comp.coefficient = 1.0;
          arrhenius_.products.push_back(comp);
        }
      }
    }
  }

  Napi::Value Arrhenius::Serialize(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);

    // Add type
    result.Set("type", "ARRHENIUS");

    // Add name
    if (!arrhenius_.name.empty())
    {
      result.Set("name", arrhenius_.name);
    }

    // Add parameters
    result.Set("A", arrhenius_.A);
    result.Set("B", arrhenius_.B);
    result.Set("C", arrhenius_.C);
    result.Set("D", arrhenius_.D);
    result.Set("E", arrhenius_.E);

    // Add gas phase
    if (!arrhenius_.gas_phase.empty())
    {
      result.Set("gas phase", arrhenius_.gas_phase);
    }

    // Add reactants
    Napi::Array reactants_arr = Napi::Array::New(env, arrhenius_.reactants.size());
    for (size_t i = 0; i < arrhenius_.reactants.size(); i++)
    {
      Napi::Object comp_obj = Napi::Object::New(env);
      comp_obj.Set(arrhenius_.reactants[i].species_name, arrhenius_.reactants[i].coefficient);
      reactants_arr.Set(uint32_t(i), comp_obj);
    }
    result.Set("reactants", reactants_arr);

    // Add products
    Napi::Array products_arr = Napi::Array::New(env, arrhenius_.products.size());
    for (size_t i = 0; i < arrhenius_.products.size(); i++)
    {
      Napi::Object comp_obj = Napi::Object::New(env);
      comp_obj.Set(arrhenius_.products[i].species_name, arrhenius_.products[i].coefficient);
      products_arr.Set(uint32_t(i), comp_obj);
    }
    result.Set("products", products_arr);

    return result;
  }

  const mechanism_configuration::v1::types::Arrhenius& Arrhenius::GetInternalArrhenius() const
  {
    return arrhenius_;
  }

  Napi::Function Arrhenius::GetClass(Napi::Env env)
  {
    return DefineClass(
        env,
        "Arrhenius",
        {
            InstanceAccessor("name", &Arrhenius::GetName, &Arrhenius::SetName),
            InstanceAccessor("A", &Arrhenius::GetA, &Arrhenius::SetA),
            InstanceAccessor("B", &Arrhenius::GetB, &Arrhenius::SetB),
            InstanceAccessor("C", &Arrhenius::GetC, &Arrhenius::SetC),
            InstanceAccessor("D", &Arrhenius::GetD, &Arrhenius::SetD),
            InstanceAccessor("E", &Arrhenius::GetE, &Arrhenius::SetE),
            InstanceAccessor("gas_phase", &Arrhenius::GetGasPhase, &Arrhenius::SetGasPhase),
            InstanceAccessor("reactants", &Arrhenius::GetReactants, &Arrhenius::SetReactants),
            InstanceAccessor("products", &Arrhenius::GetProducts, &Arrhenius::SetProducts),
            InstanceMethod("serialize", &Arrhenius::Serialize),
        });
  }

}  // namespace musica
