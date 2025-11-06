// Copyright (C) 2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#include "mechanism.h"
#include "species.h"
#include "phase.h"
#include "arrhenius.h"
#include "photolysis.h"
#include "emission.h"
#include "user_defined.h"
#include <napi.h>
#include <mechanism_configuration/v1/mechanism.hpp>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <sstream>

namespace musica
{

  Mechanism::Mechanism(const Napi::CallbackInfo& info)
      : Napi::ObjectWrap<Mechanism>(info)
  {
    Napi::Env env = info.Env();

    // Initialize the internal mechanism struct
    mechanism_ = mechanism_configuration::v1::types::Mechanism();

    // If constructor called with options object
    if (info.Length() > 0 && info[0].IsObject())
    {
      Napi::Object options = info[0].As<Napi::Object>();

      // Set name (optional)
      if (options.Has("name"))
      {
        mechanism_.name = options.Get("name").As<Napi::String>().Utf8Value();
      }

      // Set species (optional)
      if (options.Has("species") && options.Get("species").IsArray())
      {
        Napi::Array species_arr = options.Get("species").As<Napi::Array>();

        for (uint32_t i = 0; i < species_arr.Length(); i++)
        {
          Napi::Value item = species_arr.Get(i);

          if (item.IsObject())
          {
            Napi::Object species_obj = item.As<Napi::Object>();

            // Try to unwrap as a Species instance
            Species* species_wrapper = nullptr;
            if (species_obj.Has("serialize") && species_obj.Get("serialize").IsFunction())
            {
              // This looks like a wrapped Species object - try to unwrap
              try
              {
                species_wrapper = Species::Unwrap(species_obj);
              }
              catch (...)
              {
                // Unwrap failed, treat as plain object
                species_wrapper = nullptr;
              }
            }

            if (species_wrapper != nullptr)
            {
              // Successfully unwrapped a Species instance
              mechanism_.species.push_back(species_wrapper->GetInternalSpecies());
            }
            // Otherwise, create a species from plain object
            else
            {
              mechanism_configuration::v1::types::Species species;

              if (species_obj.Has("name"))
              {
                species.name = species_obj.Get("name").As<Napi::String>().Utf8Value();
              }

              if (species_obj.Has("molecular_weight_kg_mol") && species_obj.Get("molecular_weight_kg_mol").IsNumber())
              {
                species.molecular_weight = species_obj.Get("molecular_weight_kg_mol").As<Napi::Number>().DoubleValue();
              }

              if (species_obj.Has("constant_concentration_mol_m3") && species_obj.Get("constant_concentration_mol_m3").IsNumber())
              {
                species.constant_concentration = species_obj.Get("constant_concentration_mol_m3").As<Napi::Number>().DoubleValue();
              }

              if (species_obj.Has("constant_mixing_ratio_mol_mol") && species_obj.Get("constant_mixing_ratio_mol_mol").IsNumber())
              {
                species.constant_mixing_ratio = species_obj.Get("constant_mixing_ratio_mol_mol").As<Napi::Number>().DoubleValue();
              }

              if (species_obj.Has("is_third_body") && species_obj.Get("is_third_body").IsBoolean())
              {
                species.is_third_body = species_obj.Get("is_third_body").As<Napi::Boolean>().Value();
              }

              mechanism_.species.push_back(species);
            }
          }
        }
      }

      // Set phases (optional)
      if (options.Has("phases") && options.Get("phases").IsArray())
      {
        Napi::Array phases_arr = options.Get("phases").As<Napi::Array>();

        for (uint32_t i = 0; i < phases_arr.Length(); i++)
        {
          Napi::Value item = phases_arr.Get(i);

          if (item.IsObject())
          {
            Napi::Object phase_obj = item.As<Napi::Object>();

            // If it's a Phase instance, extract the internal phase
            if (phase_obj.InstanceOf(Phase::GetClass(env)))
            {
              Phase* phase_wrapper = Phase::Unwrap(phase_obj);
              mechanism_.phases.push_back(phase_wrapper->GetInternalPhase());
            }
            // Otherwise, create a phase from plain object
            else
            {
              mechanism_configuration::v1::types::Phase phase;

              if (phase_obj.Has("name"))
              {
                phase.name = phase_obj.Get("name").As<Napi::String>().Utf8Value();
              }

              if (phase_obj.Has("species") && phase_obj.Get("species").IsArray())
              {
                Napi::Array phase_species_arr = phase_obj.Get("species").As<Napi::Array>();

                for (uint32_t j = 0; j < phase_species_arr.Length(); j++)
                {
                  Napi::Value species_item = phase_species_arr.Get(j);
                  mechanism_configuration::v1::types::PhaseSpecies phase_species;

                  if (species_item.IsString())
                  {
                    phase_species.name = species_item.As<Napi::String>().Utf8Value();
                  }
                  else if (species_item.IsObject())
                  {
                    Napi::Object ps_obj = species_item.As<Napi::Object>();
                    if (ps_obj.Has("name"))
                    {
                      phase_species.name = ps_obj.Get("name").As<Napi::String>().Utf8Value();
                    }
                    if (ps_obj.Has("diffusion_coefficient"))
                    {
                      phase_species.diffusion_coefficient = ps_obj.Get("diffusion_coefficient").As<Napi::Number>().DoubleValue();
                    }
                  }

                  phase.species.push_back(phase_species);
                }
              }

              mechanism_.phases.push_back(phase);
            }
          }
        }
      }

      // Set reactions (optional)
      if (options.Has("reactions") && options.Get("reactions").IsArray())
      {
        Napi::Array reactions_arr = options.Get("reactions").As<Napi::Array>();

        for (uint32_t i = 0; i < reactions_arr.Length(); i++)
        {
          Napi::Value item = reactions_arr.Get(i);

          if (item.IsObject())
          {
            Napi::Object reaction_obj = item.As<Napi::Object>();

            // Try unwrapping as different reaction types
            // We try each type with a serialize() check first (more reliable than InstanceOf)
            bool unwrapped = false;

            if (reaction_obj.Has("serialize"))
            {
              // Try Arrhenius
              try
              {
                Arrhenius* wrapper = Arrhenius::Unwrap(reaction_obj);
                mechanism_.reactions.arrhenius.push_back(wrapper->GetInternalArrhenius());
                unwrapped = true;
              }
              catch (...)
              {
              }

              // Try Photolysis
              if (!unwrapped)
              {
                try
                {
                  Photolysis* wrapper = Photolysis::Unwrap(reaction_obj);
                  mechanism_.reactions.photolysis.push_back(wrapper->GetInternalPhotolysis());
                  unwrapped = true;
                }
                catch (...)
                {
                }
              }

              // Try Emission
              if (!unwrapped)
              {
                try
                {
                  Emission* wrapper = Emission::Unwrap(reaction_obj);
                  mechanism_.reactions.emission.push_back(wrapper->GetInternalEmission());
                  unwrapped = true;
                }
                catch (...)
                {
                }
              }

              // Try UserDefined
              if (!unwrapped)
              {
                try
                {
                  UserDefined* wrapper = UserDefined::Unwrap(reaction_obj);
                  mechanism_.reactions.user_defined.push_back(wrapper->GetInternalUserDefined());
                  unwrapped = true;
                }
                catch (...)
                {
                }
              }
            }

            // If not unwrapped, check for plain objects with type field
            if (!unwrapped && reaction_obj.Has("type"))
            {
              std::string type = reaction_obj.Get("type").As<Napi::String>().Utf8Value();

              if (type == "ARRHENIUS")
              {
                mechanism_configuration::v1::types::Arrhenius arrhenius;

                if (reaction_obj.Has("name"))
                {
                  arrhenius.name = reaction_obj.Get("name").As<Napi::String>().Utf8Value();
                }

                if (reaction_obj.Has("A"))
                {
                  arrhenius.A = reaction_obj.Get("A").As<Napi::Number>().DoubleValue();
                }

                if (reaction_obj.Has("B"))
                {
                  arrhenius.B = reaction_obj.Get("B").As<Napi::Number>().DoubleValue();
                }

                if (reaction_obj.Has("C"))
                {
                  arrhenius.C = reaction_obj.Get("C").As<Napi::Number>().DoubleValue();
                }

                if (reaction_obj.Has("D"))
                {
                  arrhenius.D = reaction_obj.Get("D").As<Napi::Number>().DoubleValue();
                }

                if (reaction_obj.Has("E"))
                {
                  arrhenius.E = reaction_obj.Get("E").As<Napi::Number>().DoubleValue();
                }

                if (reaction_obj.Has("gas_phase"))
                {
                  arrhenius.gas_phase = reaction_obj.Get("gas_phase").As<Napi::String>().Utf8Value();
                }

                // Parse reactants
                if (reaction_obj.Has("reactants") && reaction_obj.Get("reactants").IsArray())
                {
                  Napi::Array reactants_arr = reaction_obj.Get("reactants").As<Napi::Array>();
                  for (uint32_t j = 0; j < reactants_arr.Length(); j++)
                  {
                    Napi::Value reactant_item = reactants_arr.Get(j);
                    mechanism_configuration::v1::types::ReactionComponent comp;

                    if (reactant_item.IsString())
                    {
                      comp.species_name = reactant_item.As<Napi::String>().Utf8Value();
                      comp.coefficient = 1.0;
                    }
                    else if (reactant_item.IsObject())
                    {
                      Napi::Object comp_obj = reactant_item.As<Napi::Object>();
                      if (comp_obj.Has("species_name"))
                      {
                        comp.species_name = comp_obj.Get("species_name").As<Napi::String>().Utf8Value();
                      }
                      if (comp_obj.Has("coefficient"))
                      {
                        comp.coefficient = comp_obj.Get("coefficient").As<Napi::Number>().DoubleValue();
                      }
                    }

                    arrhenius.reactants.push_back(comp);
                  }
                }

                // Parse products
                if (reaction_obj.Has("products") && reaction_obj.Get("products").IsArray())
                {
                  Napi::Array products_arr = reaction_obj.Get("products").As<Napi::Array>();
                  for (uint32_t j = 0; j < products_arr.Length(); j++)
                  {
                    Napi::Value product_item = products_arr.Get(j);
                    mechanism_configuration::v1::types::ReactionComponent comp;

                    if (product_item.IsString())
                    {
                      comp.species_name = product_item.As<Napi::String>().Utf8Value();
                      comp.coefficient = 1.0;
                    }
                    else if (product_item.IsObject())
                    {
                      Napi::Object comp_obj = product_item.As<Napi::Object>();
                      if (comp_obj.Has("species_name"))
                      {
                        comp.species_name = comp_obj.Get("species_name").As<Napi::String>().Utf8Value();
                      }
                      if (comp_obj.Has("coefficient"))
                      {
                        comp.coefficient = comp_obj.Get("coefficient").As<Napi::Number>().DoubleValue();
                      }
                    }

                    arrhenius.products.push_back(comp);
                  }
                }

                mechanism_.reactions.arrhenius.push_back(arrhenius);
              }
              // Add support for other reaction types here in the future
            }
          }
        }
      }
    }
  }

  Napi::Value Mechanism::GetName(const Napi::CallbackInfo& info)
  {
    return Napi::String::New(info.Env(), mechanism_.name);
  }

  void Mechanism::SetName(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    mechanism_.name = value.As<Napi::String>().Utf8Value();
  }

  Napi::Value Mechanism::GetSpecies(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();
    Napi::Array result = Napi::Array::New(env, mechanism_.species.size());

    for (size_t i = 0; i < mechanism_.species.size(); i++)
    {
      Napi::Object species_obj = Napi::Object::New(env);
      species_obj.Set("name", mechanism_.species[i].name);

      if (mechanism_.species[i].molecular_weight.has_value())
      {
        species_obj.Set("molecular_weight_kg_mol", mechanism_.species[i].molecular_weight.value());
      }

      if (mechanism_.species[i].constant_concentration.has_value())
      {
        species_obj.Set("constant_concentration_mol_m3", mechanism_.species[i].constant_concentration.value());
      }

      if (mechanism_.species[i].constant_mixing_ratio.has_value())
      {
        species_obj.Set("constant_mixing_ratio_mol_mol", mechanism_.species[i].constant_mixing_ratio.value());
      }

      if (mechanism_.species[i].is_third_body.has_value())
      {
        species_obj.Set("is_third_body", mechanism_.species[i].is_third_body.value());
      }

      result.Set(uint32_t(i), species_obj);
    }

    return result;
  }

  void Mechanism::SetSpecies(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    Napi::Env env = info.Env();

    if (value.IsArray())
    {
      mechanism_.species.clear();
      Napi::Array species_arr = value.As<Napi::Array>();

      for (uint32_t i = 0; i < species_arr.Length(); i++)
      {
        Napi::Value item = species_arr.Get(i);

        if (item.IsObject())
        {
          Napi::Object species_obj = item.As<Napi::Object>();

          if (species_obj.InstanceOf(Species::GetClass(env)))
          {
            Species* species_wrapper = Species::Unwrap(species_obj);
            mechanism_.species.push_back(species_wrapper->GetInternalSpecies());
          }
        }
      }
    }
  }

  Napi::Value Mechanism::GetPhases(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();
    Napi::Array result = Napi::Array::New(env, mechanism_.phases.size());

    for (size_t i = 0; i < mechanism_.phases.size(); i++)
    {
      Napi::Object phase_obj = Napi::Object::New(env);
      phase_obj.Set("name", mechanism_.phases[i].name);

      Napi::Array species_arr = Napi::Array::New(env, mechanism_.phases[i].species.size());
      for (size_t j = 0; j < mechanism_.phases[i].species.size(); j++)
      {
        Napi::Object species_obj = Napi::Object::New(env);
        species_obj.Set("name", mechanism_.phases[i].species[j].name);

        if (mechanism_.phases[i].species[j].diffusion_coefficient.has_value())
        {
          species_obj.Set("diffusion_coefficient", mechanism_.phases[i].species[j].diffusion_coefficient.value());
        }

        species_arr.Set(uint32_t(j), species_obj);
      }
      phase_obj.Set("species", species_arr);

      result.Set(uint32_t(i), phase_obj);
    }

    return result;
  }

  void Mechanism::SetPhases(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    Napi::Env env = info.Env();

    if (value.IsArray())
    {
      mechanism_.phases.clear();
      Napi::Array phases_arr = value.As<Napi::Array>();

      for (uint32_t i = 0; i < phases_arr.Length(); i++)
      {
        Napi::Value item = phases_arr.Get(i);

        if (item.IsObject())
        {
          Napi::Object phase_obj = item.As<Napi::Object>();

          if (phase_obj.InstanceOf(Phase::GetClass(env)))
          {
            Phase* phase_wrapper = Phase::Unwrap(phase_obj);
            mechanism_.phases.push_back(phase_wrapper->GetInternalPhase());
          }
        }
      }
    }
  }

  Napi::Value Mechanism::GetReactions(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();

    // Count total reactions
    size_t total_reactions = mechanism_.reactions.arrhenius.size() +
                            mechanism_.reactions.branched.size() +
                            mechanism_.reactions.emission.size() +
                            mechanism_.reactions.first_order_loss.size() +
                            mechanism_.reactions.photolysis.size() +
                            mechanism_.reactions.surface.size() +
                            mechanism_.reactions.taylor_series.size() +
                            mechanism_.reactions.troe.size() +
                            mechanism_.reactions.ternary_chemical_activation.size() +
                            mechanism_.reactions.tunneling.size() +
                            mechanism_.reactions.user_defined.size();

    Napi::Array result = Napi::Array::New(env, total_reactions);
    uint32_t index = 0;

    // Add Arrhenius reactions
    for (const auto& rxn : mechanism_.reactions.arrhenius)
    {
      Napi::Object reaction_obj = Napi::Object::New(env);
      reaction_obj.Set("type", "ARRHENIUS");

      if (!rxn.name.empty())
      {
        reaction_obj.Set("name", rxn.name);
      }

      reaction_obj.Set("A", rxn.A);
      reaction_obj.Set("B", rxn.B);
      reaction_obj.Set("C", rxn.C);
      reaction_obj.Set("D", rxn.D);
      reaction_obj.Set("E", rxn.E);

      if (!rxn.gas_phase.empty())
      {
        reaction_obj.Set("gas_phase", rxn.gas_phase);
      }

      Napi::Array reactants_arr = Napi::Array::New(env, rxn.reactants.size());
      for (size_t i = 0; i < rxn.reactants.size(); i++)
      {
        Napi::Object comp_obj = Napi::Object::New(env);
        comp_obj.Set("species_name", rxn.reactants[i].species_name);
        comp_obj.Set("coefficient", rxn.reactants[i].coefficient);
        reactants_arr.Set(uint32_t(i), comp_obj);
      }
      reaction_obj.Set("reactants", reactants_arr);

      Napi::Array products_arr = Napi::Array::New(env, rxn.products.size());
      for (size_t i = 0; i < rxn.products.size(); i++)
      {
        Napi::Object comp_obj = Napi::Object::New(env);
        comp_obj.Set("species_name", rxn.products[i].species_name);
        comp_obj.Set("coefficient", rxn.products[i].coefficient);
        products_arr.Set(uint32_t(i), comp_obj);
      }
      reaction_obj.Set("products", products_arr);

      result.Set(index++, reaction_obj);
    }

    // Add other reaction types here in the future

    return result;
  }

  void Mechanism::SetReactions(const Napi::CallbackInfo& info, const Napi::Value& value)
  {
    Napi::Env env = info.Env();

    if (value.IsArray())
    {
      // Clear all reaction types
      mechanism_.reactions.arrhenius.clear();
      mechanism_.reactions.branched.clear();
      mechanism_.reactions.emission.clear();
      mechanism_.reactions.first_order_loss.clear();
      mechanism_.reactions.photolysis.clear();
      mechanism_.reactions.surface.clear();
      mechanism_.reactions.taylor_series.clear();
      mechanism_.reactions.troe.clear();
      mechanism_.reactions.ternary_chemical_activation.clear();
      mechanism_.reactions.tunneling.clear();
      mechanism_.reactions.user_defined.clear();

      Napi::Array reactions_arr = value.As<Napi::Array>();

      for (uint32_t i = 0; i < reactions_arr.Length(); i++)
      {
        Napi::Value item = reactions_arr.Get(i);

        if (item.IsObject())
        {
          Napi::Object reaction_obj = item.As<Napi::Object>();

          // Check if it's an Arrhenius instance
          if (reaction_obj.InstanceOf(Arrhenius::GetClass(env)))
          {
            Arrhenius* arrhenius_wrapper = Arrhenius::Unwrap(reaction_obj);
            mechanism_.reactions.arrhenius.push_back(arrhenius_wrapper->GetInternalArrhenius());
          }
        }
      }
    }
  }

  Napi::Value Mechanism::Serialize(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);

    // Add version
    result.Set("version", "1.0.0");

    // Add name (if set)
    if (!mechanism_.name.empty())
    {
      result.Set("name", mechanism_.name);
    }

    // Add species array
    Napi::Array species_arr = Napi::Array::New(env, mechanism_.species.size());
    for (size_t i = 0; i < mechanism_.species.size(); i++)
    {
      Napi::Object species_obj = Napi::Object::New(env);
      species_obj.Set("name", mechanism_.species[i].name);

      if (mechanism_.species[i].molecular_weight.has_value())
      {
        species_obj.Set("molecular weight [kg mol-1]", mechanism_.species[i].molecular_weight.value());
      }

      if (mechanism_.species[i].constant_concentration.has_value())
      {
        species_obj.Set("constant concentration [mol m-3]", mechanism_.species[i].constant_concentration.value());
      }

      if (mechanism_.species[i].constant_mixing_ratio.has_value())
      {
        species_obj.Set("constant mixing ratio [mol mol-1]", mechanism_.species[i].constant_mixing_ratio.value());
      }

      if (mechanism_.species[i].is_third_body.has_value() && mechanism_.species[i].is_third_body.value())
      {
        species_obj.Set("is third body", true);
      }

      // Add unknown properties
      for (const auto& [key, value] : mechanism_.species[i].unknown_properties)
      {
        species_obj.Set(key, value);
      }

      species_arr.Set(uint32_t(i), species_obj);
    }
    result.Set("species", species_arr);

    // Add phases array
    Napi::Array phases_arr = Napi::Array::New(env, mechanism_.phases.size());
    for (size_t i = 0; i < mechanism_.phases.size(); i++)
    {
      Napi::Object phase_obj = Napi::Object::New(env);
      phase_obj.Set("name", mechanism_.phases[i].name);

      Napi::Array phase_species_arr = Napi::Array::New(env, mechanism_.phases[i].species.size());
      for (size_t j = 0; j < mechanism_.phases[i].species.size(); j++)
      {
        Napi::Object ps_obj = Napi::Object::New(env);
        ps_obj.Set("name", mechanism_.phases[i].species[j].name);

        if (mechanism_.phases[i].species[j].diffusion_coefficient.has_value())
        {
          ps_obj.Set("diffusion coefficient", mechanism_.phases[i].species[j].diffusion_coefficient.value());
        }

        phase_species_arr.Set(uint32_t(j), ps_obj);
      }
      phase_obj.Set("species", phase_species_arr);

      phases_arr.Set(uint32_t(i), phase_obj);
    }
    result.Set("phases", phases_arr);

    // Add reactions object
    Napi::Object reactions_obj = Napi::Object::New(env);

    // Add Arrhenius reactions
    if (!mechanism_.reactions.arrhenius.empty())
    {
      Napi::Array arrhenius_arr = Napi::Array::New(env, mechanism_.reactions.arrhenius.size());
      for (size_t i = 0; i < mechanism_.reactions.arrhenius.size(); i++)
      {
        const auto& rxn = mechanism_.reactions.arrhenius[i];
        Napi::Object reaction_obj = Napi::Object::New(env);

        reaction_obj.Set("type", "ARRHENIUS");

        if (!rxn.name.empty())
        {
          reaction_obj.Set("name", rxn.name);
        }

        reaction_obj.Set("A", rxn.A);
        reaction_obj.Set("B", rxn.B);
        reaction_obj.Set("C", rxn.C);
        reaction_obj.Set("D", rxn.D);
        reaction_obj.Set("E", rxn.E);

        if (!rxn.gas_phase.empty())
        {
          reaction_obj.Set("gas phase", rxn.gas_phase);
        }

        Napi::Array reactants_arr = Napi::Array::New(env, rxn.reactants.size());
        for (size_t j = 0; j < rxn.reactants.size(); j++)
        {
          Napi::Object comp_obj = Napi::Object::New(env);
          comp_obj.Set(rxn.reactants[j].species_name, rxn.reactants[j].coefficient);
          reactants_arr.Set(uint32_t(j), comp_obj);
        }
        reaction_obj.Set("reactants", reactants_arr);

        Napi::Array products_arr = Napi::Array::New(env, rxn.products.size());
        for (size_t j = 0; j < rxn.products.size(); j++)
        {
          Napi::Object comp_obj = Napi::Object::New(env);
          comp_obj.Set(rxn.products[j].species_name, rxn.products[j].coefficient);
          products_arr.Set(uint32_t(j), comp_obj);
        }
        reaction_obj.Set("products", products_arr);

        arrhenius_arr.Set(uint32_t(i), reaction_obj);
      }
      reactions_obj.Set("ARRHENIUS", arrhenius_arr);
    }

    // Add support for other reaction types here in the future

    result.Set("reactions", reactions_obj);

    return result;
  }

  Napi::Value Mechanism::Export(const Napi::CallbackInfo& info)
  {
    Napi::Env env = info.Env();

    // Check that a filename was provided
    if (info.Length() < 1 || !info[0].IsString())
    {
      Napi::TypeError::New(env, "String filename expected").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    std::string filename = info[0].As<Napi::String>().Utf8Value();

    // Determine file type by extension
    bool is_yaml = false;
    if (filename.length() >= 5 &&
        (filename.substr(filename.length() - 5) == ".yaml" ||
         filename.substr(filename.length() - 4) == ".yml"))
    {
      is_yaml = true;
    }
    else if (filename.length() < 5 || filename.substr(filename.length() - 5) != ".json")
    {
      // Default to JSON if no recognized extension
      filename += ".json";
    }

    try
    {
      if (is_yaml)
      {
        // Convert mechanism to YAML
        YAML::Node root;

        root["version"] = "1.0.0";

        if (!mechanism_.name.empty())
        {
          root["name"] = mechanism_.name;
        }

        // Add species
        for (const auto& species : mechanism_.species)
        {
          YAML::Node species_node;
          species_node["name"] = species.name;

          if (species.molecular_weight.has_value())
          {
            species_node["molecular weight [kg mol-1]"] = species.molecular_weight.value();
          }

          if (species.constant_concentration.has_value())
          {
            species_node["constant concentration [mol m-3]"] = species.constant_concentration.value();
          }

          if (species.constant_mixing_ratio.has_value())
          {
            species_node["constant mixing ratio [mol mol-1]"] = species.constant_mixing_ratio.value();
          }

          if (species.is_third_body.has_value() && species.is_third_body.value())
          {
            species_node["is third body"] = true;
          }

          for (const auto& [key, value] : species.unknown_properties)
          {
            species_node[key] = value;
          }

          root["species"].push_back(species_node);
        }

        // Add phases
        for (const auto& phase : mechanism_.phases)
        {
          YAML::Node phase_node;
          phase_node["name"] = phase.name;

          for (const auto& ps : phase.species)
          {
            YAML::Node ps_node;
            ps_node["name"] = ps.name;

            if (ps.diffusion_coefficient.has_value())
            {
              ps_node["diffusion coefficient"] = ps.diffusion_coefficient.value();
            }

            phase_node["species"].push_back(ps_node);
          }

          root["phases"].push_back(phase_node);
        }

        // Add reactions
        if (!mechanism_.reactions.arrhenius.empty())
        {
          for (const auto& rxn : mechanism_.reactions.arrhenius)
          {
            YAML::Node rxn_node;
            rxn_node["type"] = "ARRHENIUS";

            if (!rxn.name.empty())
            {
              rxn_node["name"] = rxn.name;
            }

            rxn_node["A"] = rxn.A;
            rxn_node["B"] = rxn.B;
            rxn_node["C"] = rxn.C;
            rxn_node["D"] = rxn.D;
            rxn_node["E"] = rxn.E;

            if (!rxn.gas_phase.empty())
            {
              rxn_node["gas phase"] = rxn.gas_phase;
            }

            for (const auto& comp : rxn.reactants)
            {
              YAML::Node comp_node;
              comp_node[comp.species_name] = comp.coefficient;
              rxn_node["reactants"].push_back(comp_node);
            }

            for (const auto& comp : rxn.products)
            {
              YAML::Node comp_node;
              comp_node[comp.species_name] = comp.coefficient;
              rxn_node["products"].push_back(comp_node);
            }

            root["reactions"]["ARRHENIUS"].push_back(rxn_node);
          }
        }

        // Write to file
        std::ofstream fout(filename);
        fout << root;
        fout.close();
      }
      else
      {
        // JSON export - use the serialize method and write to file
        Napi::Object serialized = Serialize(info).As<Napi::Object>();

        // Convert to JSON string
        Napi::Object json_obj = env.Global().Get("JSON").As<Napi::Object>();
        Napi::Function stringify = json_obj.Get("stringify").As<Napi::Function>();

        // Call JSON.stringify with 2-space indentation
        Napi::Value json_string = stringify.Call(json_obj, { serialized, env.Undefined(), Napi::Number::New(env, 2) });

        // Write to file
        std::ofstream fout(filename);
        fout << json_string.As<Napi::String>().Utf8Value();
        fout.close();
      }

      return Napi::Boolean::New(env, true);
    }
    catch (const std::exception& e)
    {
      Napi::Error::New(env, std::string("Failed to export mechanism: ") + e.what()).ThrowAsJavaScriptException();
      return env.Undefined();
    }
  }

  const mechanism_configuration::v1::types::Mechanism& Mechanism::GetInternalMechanism() const
  {
    return mechanism_;
  }

  Napi::Function Mechanism::GetClass(Napi::Env env)
  {
    return DefineClass(
        env,
        "Mechanism",
        {
            InstanceAccessor("name", &Mechanism::GetName, &Mechanism::SetName),
            InstanceAccessor("species", &Mechanism::GetSpecies, &Mechanism::SetSpecies),
            InstanceAccessor("phases", &Mechanism::GetPhases, &Mechanism::SetPhases),
            InstanceAccessor("reactions", &Mechanism::GetReactions, &Mechanism::SetReactions),
            InstanceMethod("serialize", &Mechanism::Serialize),
            InstanceMethod("export", &Mechanism::Export),
        });
  }

}  // namespace musica
