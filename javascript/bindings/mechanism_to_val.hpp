// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Convert a parsed mechanism_configuration::Mechanism into a plain JavaScript
// object (emscripten::val) in the v1 mechanism-configuration wire shape — the
// same shape the JavaScript getJSON() methods produce and that downstream
// consumers already parse.
//
// The wire key strings below mirror mechanism_configuration's private
// src/detail/v1/**/keys.hpp. They are transcribed (not #included) because those
// headers are internal to the dependency. If the upstream keys change, update
// them here. The long-term fix is a serializer in mechanism_configuration
// itself; until then this walker is the inverse of its parser.
#pragma once

#include <musica/configuration/read_mechanism.hpp>

#include <mechanism_configuration/mechanism.hpp>
#include <mechanism_configuration/types/reactions.hpp>
#include <mechanism_configuration/types/species.hpp>

#include <emscripten/val.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace musica
{
  namespace mc_types = mechanism_configuration::types;

  // ── reaction components ──────────────────────────────────────
  inline void SetUnknownProperties(
      emscripten::val& obj,
      const std::unordered_map<std::string, std::string>& unknown_properties)
  {
    // Keys already carry their `__` prefix as parsed.
    for (const auto& [key, value] : unknown_properties)
      obj.set(key, value);
  }

  inline emscripten::val ComponentToVal(const mc_types::ReactionComponent& component)
  {
    emscripten::val obj = emscripten::val::object();
    obj.set("name", component.name);
    obj.set("coefficient", component.coefficient);
    SetUnknownProperties(obj, component.unknown_properties);
    return obj;
  }

  inline emscripten::val ComponentsToVal(const std::vector<mc_types::ReactionComponent>& components)
  {
    emscripten::val arr = emscripten::val::array();
    for (const auto& component : components)
      arr.call<void>("push", ComponentToVal(component));
    return arr;
  }

  // The wire format always uses an array for reactants/products, even where the
  // struct models a single component (photolysis, first-order loss).
  inline emscripten::val ComponentToArray(const mc_types::ReactionComponent& component)
  {
    emscripten::val arr = emscripten::val::array();
    arr.call<void>("push", ComponentToVal(component));
    return arr;
  }

  inline void SetGasPhase(emscripten::val& obj, const std::string& gas_phase)
  {
    if (!gas_phase.empty())
      obj.set("gas phase", gas_phase);
  }

  // ── per-reaction-type conversion ─────────────────────────────
  inline emscripten::val ReactionToVal(const mc_types::Arrhenius& r)
  {
    emscripten::val o = emscripten::val::object();
    o.set("type", "ARRHENIUS");
    o.set("name", r.name);
    o.set("A", r.A);
    o.set("B", r.B);
    o.set("C", r.C);
    o.set("D", r.D);
    o.set("E", r.E);
    SetGasPhase(o, r.gas_phase);
    o.set("reactants", ComponentsToVal(r.reactants));
    o.set("products", ComponentsToVal(r.products));
    SetUnknownProperties(o, r.unknown_properties);
    return o;
  }

  inline emscripten::val ReactionToVal(const mc_types::Branched& r)
  {
    emscripten::val o = emscripten::val::object();
    o.set("type", "BRANCHED_NO_RO2");
    o.set("name", r.name);
    o.set("X", r.X);
    o.set("Y", r.Y);
    o.set("a0", r.a0);
    o.set("n", r.n);
    SetGasPhase(o, r.gas_phase);
    o.set("reactants", ComponentsToVal(r.reactants));
    o.set("nitrate products", ComponentsToVal(r.nitrate_products));
    o.set("alkoxy products", ComponentsToVal(r.alkoxy_products));
    SetUnknownProperties(o, r.unknown_properties);
    return o;
  }

  inline emscripten::val ReactionToVal(const mc_types::Emission& r)
  {
    emscripten::val o = emscripten::val::object();
    o.set("type", "EMISSION");
    o.set("name", r.name);
    o.set("scaling factor", r.scaling_factor);
    SetGasPhase(o, r.gas_phase);
    o.set("products", ComponentsToVal(r.products));
    SetUnknownProperties(o, r.unknown_properties);
    return o;
  }

  inline emscripten::val ReactionToVal(const mc_types::FirstOrderLoss& r)
  {
    emscripten::val o = emscripten::val::object();
    o.set("type", "FIRST_ORDER_LOSS");
    o.set("name", r.name);
    o.set("scaling factor", r.scaling_factor);
    SetGasPhase(o, r.gas_phase);
    o.set("reactants", ComponentToArray(r.reactants));
    if (!r.products.empty())
      o.set("products", ComponentsToVal(r.products));
    SetUnknownProperties(o, r.unknown_properties);
    return o;
  }

  inline emscripten::val ReactionToVal(const mc_types::Photolysis& r)
  {
    emscripten::val o = emscripten::val::object();
    o.set("type", "PHOTOLYSIS");
    o.set("name", r.name);
    o.set("scaling factor", r.scaling_factor);
    SetGasPhase(o, r.gas_phase);
    o.set("reactants", ComponentsToVal(r.reactants));
    o.set("products", ComponentsToVal(r.products));
    SetUnknownProperties(o, r.unknown_properties);
    return o;
  }

  inline emscripten::val ReactionToVal(const mc_types::Surface& r)
  {
    emscripten::val o = emscripten::val::object();
    o.set("type", "SURFACE");
    o.set("name", r.name);
    o.set("reaction probability", r.reaction_probability);
    SetGasPhase(o, r.gas_phase);
    o.set("gas-phase species", r.gas_phase_species.name);
    o.set("gas-phase products", ComponentsToVal(r.gas_phase_products));
    if (!r.condensed_phase.empty())
      o.set("condensed phase", r.condensed_phase);
    SetUnknownProperties(o, r.unknown_properties);
    return o;
  }

  inline emscripten::val ReactionToVal(const mc_types::TaylorSeries& r)
  {
    emscripten::val o = emscripten::val::object();
    o.set("type", "TAYLOR_SERIES");
    o.set("name", r.name);
    o.set("A", r.A);
    o.set("B", r.B);
    o.set("C", r.C);
    o.set("D", r.D);
    o.set("E", r.E);
    emscripten::val coefficients = emscripten::val::array();
    for (double c : r.taylor_coefficients)
      coefficients.call<void>("push", c);
    o.set("taylor coefficients", coefficients);
    SetGasPhase(o, r.gas_phase);
    o.set("reactants", ComponentsToVal(r.reactants));
    o.set("products", ComponentsToVal(r.products));
    SetUnknownProperties(o, r.unknown_properties);
    return o;
  }

  inline emscripten::val ReactionToVal(const mc_types::Troe& r)
  {
    emscripten::val o = emscripten::val::object();
    o.set("type", "TROE");
    o.set("name", r.name);
    o.set("k0_A", r.k0_A);
    o.set("k0_B", r.k0_B);
    o.set("k0_C", r.k0_C);
    o.set("kinf_A", r.kinf_A);
    o.set("kinf_B", r.kinf_B);
    o.set("kinf_C", r.kinf_C);
    o.set("Fc", r.Fc);
    o.set("N", r.N);
    SetGasPhase(o, r.gas_phase);
    o.set("reactants", ComponentsToVal(r.reactants));
    o.set("products", ComponentsToVal(r.products));
    SetUnknownProperties(o, r.unknown_properties);
    return o;
  }

  inline emscripten::val ReactionToVal(const mc_types::TernaryChemicalActivation& r)
  {
    emscripten::val o = emscripten::val::object();
    o.set("type", "TERNARY_CHEMICAL_ACTIVATION");
    o.set("name", r.name);
    o.set("k0_A", r.k0_A);
    o.set("k0_B", r.k0_B);
    o.set("k0_C", r.k0_C);
    o.set("kinf_A", r.kinf_A);
    o.set("kinf_B", r.kinf_B);
    o.set("kinf_C", r.kinf_C);
    o.set("Fc", r.Fc);
    o.set("N", r.N);
    SetGasPhase(o, r.gas_phase);
    o.set("reactants", ComponentsToVal(r.reactants));
    o.set("products", ComponentsToVal(r.products));
    SetUnknownProperties(o, r.unknown_properties);
    return o;
  }

  inline emscripten::val ReactionToVal(const mc_types::Tunneling& r)
  {
    emscripten::val o = emscripten::val::object();
    o.set("type", "TUNNELING");
    o.set("name", r.name);
    o.set("A", r.A);
    o.set("B", r.B);
    o.set("C", r.C);
    SetGasPhase(o, r.gas_phase);
    o.set("reactants", ComponentsToVal(r.reactants));
    o.set("products", ComponentsToVal(r.products));
    SetUnknownProperties(o, r.unknown_properties);
    return o;
  }

  inline emscripten::val ReactionToVal(const mc_types::UserDefined& r)
  {
    emscripten::val o = emscripten::val::object();
    o.set("type", "USER_DEFINED");
    o.set("name", r.name);
    o.set("scaling factor", r.scaling_factor);
    SetGasPhase(o, r.gas_phase);
    o.set("reactants", ComponentsToVal(r.reactants));
    o.set("products", ComponentsToVal(r.products));
    SetUnknownProperties(o, r.unknown_properties);
    return o;
  }

  inline emscripten::val ReactionToVal(const mc_types::LambdaRateConstant& r)
  {
    emscripten::val o = emscripten::val::object();
    o.set("type", "LAMBDA_RATE_CONSTANT");
    o.set("name", r.name);
    o.set("lambda function", r.lambda_function);
    SetGasPhase(o, r.gas_phase);
    o.set("reactants", ComponentsToVal(r.reactants));
    o.set("products", ComponentsToVal(r.products));
    SetUnknownProperties(o, r.unknown_properties);
    return o;
  }

  // ── species / phases ─────────────────────────────────────────
  inline emscripten::val SpeciesToVal(const mc_types::Species& s)
  {
    emscripten::val o = emscripten::val::object();
    o.set("name", s.name);
    if (s.absolute_tolerance)
      o.set("absolute tolerance", *s.absolute_tolerance);
    if (s.diffusion_coefficient)
      o.set("diffusion coefficient [m2 s-1]", *s.diffusion_coefficient);
    if (s.molecular_weight)
      o.set("molecular weight [kg mol-1]", *s.molecular_weight);
    if (s.henrys_law_constant_298)
      o.set("HLC(298K) [mol m-3 Pa-1]", *s.henrys_law_constant_298);
    if (s.henrys_law_constant_exponential_factor)
      o.set("HLC exponential factor [K]", *s.henrys_law_constant_exponential_factor);
    if (s.n_star)
      o.set("N star", *s.n_star);
    if (s.constant_concentration)
      o.set("constant concentration [mol m-3]", *s.constant_concentration);
    if (s.constant_mixing_ratio)
      o.set("constant mixing ratio [mol mol-1]", *s.constant_mixing_ratio);
    if (s.is_third_body)
      o.set("is third body", *s.is_third_body);
    SetUnknownProperties(o, s.unknown_properties);
    return o;
  }

  inline emscripten::val PhaseSpeciesToVal(const mc_types::PhaseSpecies& ps)
  {
    emscripten::val o = emscripten::val::object();
    o.set("name", ps.name);
    if (ps.diffusion_coefficient)
      o.set("diffusion coefficient [m2 s-1]", *ps.diffusion_coefficient);
    if (ps.density)
      o.set("density [kg m-3]", *ps.density);
    SetUnknownProperties(o, ps.unknown_properties);
    return o;
  }

  inline emscripten::val PhaseToVal(const mc_types::Phase& p)
  {
    emscripten::val o = emscripten::val::object();
    o.set("name", p.name);
    emscripten::val species = emscripten::val::array();
    for (const auto& ps : p.species)
      species.call<void>("push", PhaseSpeciesToVal(ps));
    o.set("species", species);
    SetUnknownProperties(o, p.unknown_properties);
    return o;
  }

  // ── whole mechanism ──────────────────────────────────────────
  inline emscripten::val MechanismToVal(const mechanism_configuration::Mechanism& m)
  {
    emscripten::val o = emscripten::val::object();
    o.set("version", m.version.to_string());
    o.set("name", m.name);

    emscripten::val species = emscripten::val::array();
    for (const auto& s : m.species)
      species.call<void>("push", SpeciesToVal(s));
    o.set("species", species);

    emscripten::val phases = emscripten::val::array();
    for (const auto& p : m.phases)
      phases.call<void>("push", PhaseToVal(p));
    o.set("phases", phases);

    // The struct groups reactions by type; the wire format is a single flat
    // array of reactions each carrying a `type` discriminator.
    emscripten::val reactions = emscripten::val::array();
    auto push_all = [&reactions](const auto& vec)
    {
      for (const auto& r : vec)
        reactions.call<void>("push", ReactionToVal(r));
    };
    push_all(m.reactions.arrhenius);
    push_all(m.reactions.branched);
    push_all(m.reactions.emission);
    push_all(m.reactions.first_order_loss);
    push_all(m.reactions.photolysis);
    push_all(m.reactions.surface);
    push_all(m.reactions.taylor_series);
    push_all(m.reactions.troe);
    push_all(m.reactions.ternary_chemical_activation);
    push_all(m.reactions.tunneling);
    push_all(m.reactions.user_defined);
    push_all(m.reactions.lambda_rate_constant);
    o.set("reactions", reactions);

    return o;
  }
}  // namespace musica
