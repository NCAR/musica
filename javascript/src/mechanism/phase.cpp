// Copyright (C) 2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#include "phase.h"
#include "species.h"
#include <napi.h>
#include <mechanism_configuration/v1/types.hpp>

namespace musica
{

  Phase::Phase(const Napi::CallbackInfo& info)
      : Napi::ObjectWrap<Phase>(info)
  {
    Napi::Env env = info.Env();

    // Initialize the internal phase struct
    phase_ = mechanism_configuration::v1::types::Phase();

    // If constructor called with options object
    if (info.Length() > 0 && info[0].IsObject())
    {
      Napi::Object options = info[0].As<Napi::Object>();

      // Set name (required)
      if (options.Has("name"))
      {
        phase_.name = options.Get("name").As<Napi::String>().Utf8Value();
      }

      // Set species (optional)
      if (options.Has("species") && options.Get("species").IsArray())
      {
        Napi::Array species_arr = options.Get("species").As<Napi::Array>();

        for (uint32_t i = 0; i < species_arr.Length(); i++)
        {
          Napi::Value item = species_arr.Get(i);

          // If it's a Species object, extract the name
          if (item.IsObject())
          {
            Napi::Object spec_obj = item.As<Napi::Object>();

            // Check if this is a Species instance
            if (spec_obj.InstanceOf(Species::GetClass(env)))
            {
              Species* species_wrapper = Species::Unwrap(spec_obj);
              mechanism_configuration::v1::types::PhaseSpecies phase_species;
              phase_species.name = species_wrapper->GetInternalSpecies().name;
              phase_.species.push_back(phase_species);
            }
            // Otherwise treat it as a plain object with name
            else if (spec_obj.Has("name"))
            {
              mechanism_configuration::v1::types::PhaseSpecies phase_species;
              phase_species.name = spec_obj.Get("name").As<Napi::String>().Utf8Value();

              if (spec_obj.Has("diffusion_coefficient"))
              {
                phase_species.diffusion_coefficient = spec_obj.Get("diffusion_coefficient").As<Napi::Number>().DoubleValue();
              }

              phase_.species.push_back(phase_species);
            }
          }
          // If it's a string, use it as the species name
          else if (item.IsString())
          {
            mechanism_configuration::v1::types::PhaseSpecies phase_species;
            phase_species.name = item.As<Napi::String>().Utf8Value();
            phase_.species.push_back(phase_species);
          }
        }
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
          phase_.unknown_properties[key] = value;
        }
      }
    }
  }

  Napi::Value Phase::GetName(const Napi::CallbackInfo& info)
  {
    return Napi::String::New(info.Env(), phase_.name);
  }

  void Phase::SetName(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    phase_.name = value.As<Napi::String>().Utf8Value();
  }

  Napi::Value Phase::GetSpecies(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();
    Napi::Array result = Napi::Array::New(env, phase_.species.size());

    for (size_t i = 0; i < phase_.species.size(); i++)
    {
      Napi::Object spec_obj = Napi::Object::New(env);
      spec_obj.Set("name", phase_.species[i].name);

      if (phase_.species[i].diffusion_coefficient.has_value())
      {
        spec_obj.Set("diffusion_coefficient", phase_.species[i].diffusion_coefficient.value());
      }

      result.Set(uint32_t(i), spec_obj);
    }

    return result;
  }

  void Phase::SetSpecies(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    Napi::Env env = info.Env();

    if (value.IsArray())
    {
      phase_.species.clear();
      Napi::Array species_arr = value.As<Napi::Array>();

      for (uint32_t i = 0; i < species_arr.Length(); i++)
      {
        Napi::Value item = species_arr.Get(i);

        // If it's a Species object
        if (item.IsObject())
        {
          Napi::Object spec_obj = item.As<Napi::Object>();

          if (spec_obj.InstanceOf(Species::GetClass(env)))
          {
            Species* species_wrapper = Species::Unwrap(spec_obj);
            mechanism_configuration::v1::types::PhaseSpecies phase_species;
            phase_species.name = species_wrapper->GetInternalSpecies().name;
            phase_.species.push_back(phase_species);
          }
          else if (spec_obj.Has("name"))
          {
            mechanism_configuration::v1::types::PhaseSpecies phase_species;
            phase_species.name = spec_obj.Get("name").As<Napi::String>().Utf8Value();

            if (spec_obj.Has("diffusion_coefficient"))
            {
              phase_species.diffusion_coefficient = spec_obj.Get("diffusion_coefficient").As<Napi::Number>().DoubleValue();
            }

            phase_.species.push_back(phase_species);
          }
        }
        else if (item.IsString())
        {
          mechanism_configuration::v1::types::PhaseSpecies phase_species;
          phase_species.name = item.As<Napi::String>().Utf8Value();
          phase_.species.push_back(phase_species);
        }
      }
    }
  }

  Napi::Value Phase::Serialize(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);

    // Add name
    result.Set("name", phase_.name);

    // Add species array
    Napi::Array species_arr = Napi::Array::New(env, phase_.species.size());
    for (size_t i = 0; i < phase_.species.size(); i++)
    {
      Napi::Object spec_obj = Napi::Object::New(env);
      spec_obj.Set("name", phase_.species[i].name);

      if (phase_.species[i].diffusion_coefficient.has_value())
      {
        spec_obj.Set("diffusion coefficient", phase_.species[i].diffusion_coefficient.value());
      }

      species_arr.Set(uint32_t(i), spec_obj);
    }
    result.Set("species", species_arr);

    // Add other properties
    for (const auto& [key, value] : phase_.unknown_properties)
    {
      result.Set(key, value);
    }

    return result;
  }

  const mechanism_configuration::v1::types::Phase& Phase::GetInternalPhase() const
  {
    return phase_;
  }

  Napi::Function Phase::GetClass(Napi::Env env)
  {
    return DefineClass(
        env,
        "Phase",
        {
            InstanceAccessor("name", &Phase::GetName, &Phase::SetName),
            InstanceAccessor("species", &Phase::GetSpecies, &Phase::SetSpecies),
            InstanceMethod("serialize", &Phase::Serialize),
        });
  }

}  // namespace musica
