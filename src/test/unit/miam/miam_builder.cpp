// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Unit tests for the MIAM builder: mechanism (aerosol section) → miam::Model → MICM solver

#include <musica/configuration/parse.hpp>
#include <musica/miam/miam_builder.hpp>
#include <musica/micm/micm.hpp>
#include <musica/micm/state.hpp>

#include <mechanism_configuration/mechanism_configuration.hpp>

#include <gtest/gtest.h>

#include <cmath>
#include <functional>
#include <optional>
#include <string>
#include <variant>
#include <vector>

// ═══ Helper: CAM Cloud Chemistry configuration ══════════════════════════════
//
// This mirrors the MIAM integration test with SO2/H2O2/O3 sulfate chemistry.
// Uses the revised 2-step H2O2 oxidation mechanism:
//   R1a: HSO3- + H2O2_aq ⇌ SO2OOH- + H2O  (reversible, Keq = 1725)
//   R1b: SO2OOH- + H+ → SO4--              (irreversible)
//   R2:  HSO3- + O3_aq → SO4-- + H+
//   R3:  SO3-- + O3_aq → SO4--
//
// The aerosol configuration lives on the mechanism's `aerosol` section; species
// and phases are shared with the (otherwise gas-only) mechanism and referenced
// by name. All concentrations (including condensed-phase) are in mol/m³ of AIR.
// c_H2O_M (55.556 mol/L) is a unit-conversion constant for rate/equilibrium
// constants from literature (M-based) to MIAM (mol/m³) — NOT a state variable.

namespace
{
  namespace types = mechanism_configuration::types;

  constexpr double M_ATM_TO_MOL_M3_PA = 1000.0 / 101325.0;
  constexpr double c_H2O_M = 55.556;  // mol/L — unit conversion constant (NOT a state variable)
  constexpr double MW_H2O = 0.018;    // kg/mol
  constexpr double RHO_H2O = 1000.0;  // kg/m3

  // ── Small builders to keep the configuration readable ──

  types::Species MakeSpecies(
      const std::string& name,
      std::optional<double> molecular_weight = std::nullopt,
      std::optional<double> density = std::nullopt)
  {
    types::Species s;
    s.name = name;
    s.molecular_weight = molecular_weight;
    s.density = density;
    return s;
  }

  // A phase-species entry: a bare name, or a name plus phase-specific properties
  // (diffusion coefficient, density) — mirroring the config's phase species list,
  // e.g. { "name": "SO2", "diffusion coefficient [m2 s-1]": 1.28e-5 }.
  struct PhaseSpec
  {
    std::string name;
    std::optional<double> diffusion_coefficient = std::nullopt;
    std::optional<double> density = std::nullopt;
    PhaseSpec(const char* n)
        : name(n)
    {
    }
    PhaseSpec(std::string n, std::optional<double> diffusion, std::optional<double> dens = std::nullopt)
        : name(std::move(n)),
          diffusion_coefficient(diffusion),
          density(dens)
    {
    }
  };

  types::Phase MakePhase(const std::string& name, const std::vector<PhaseSpec>& species)
  {
    types::Phase p;
    p.name = name;
    for (const auto& s : species)
    {
      types::PhaseSpecies ps;
      ps.name = s.name;
      ps.diffusion_coefficient = s.diffusion_coefficient;
      ps.density = s.density;
      p.species.push_back(ps);
    }
    return p;
  }

  std::vector<types::ReactionComponent> Comps(const std::vector<std::string>& names)
  {
    std::vector<types::ReactionComponent> components;
    for (const auto& n : names)
    {
      types::ReactionComponent c;
      c.name = n;
      components.push_back(c);
    }
    return components;
  }

  types::ArrheniusReferenceTemperature Arr(double A, double C = 0.0)
  {
    types::ArrheniusReferenceTemperature a;
    a.A = A;
    a.C = C;
    return a;
  }

  types::HenryLawConstant Henry(double hlc_ref, double C)
  {
    types::HenryLawConstant h;
    h.HLC_ref = hlc_ref;
    h.C = C;
    return h;
  }

  // A mechanism whose only chemistry is the MIAM aerosol section. The gas phase
  // carries `gas_species`; all condensed-phase species live in `condensed_species`.
  mechanism_configuration::Mechanism MakeAerosolMechanism(
      const std::string& name,
      const std::vector<types::Species>& species,
      const std::vector<PhaseSpec>& gas_species,
      const std::vector<PhaseSpec>& condensed_species,
      types::Aerosol aerosol)
  {
    mechanism_configuration::Mechanism mech;
    mech.name = name;
    mech.species = species;
    mech.phases = { MakePhase("gas", gas_species), MakePhase("AQUEOUS", condensed_species) };
    mech.aerosol = std::move(aerosol);
    return mech;
  }

  // Full cloud-chemistry mechanism (species + phases + aerosol section).
  mechanism_configuration::Mechanism CreateCloudChemistryMechanism()
  {
    types::Aerosol aerosol;

    // Representation: uniform cloud section
    aerosol.representations = {
      types::UniformSection{ .name = "CLOUD", .phases = { "AQUEOUS" }, .min_radius = 1e-6, .max_radius = 1e-5 },
    };

    // R1a: HSO3- + H2O2_aq ⇌ SO2OOH- + H2O  (reversible; forward + equilibrium given)
    types::DissolvedReversibleReaction r1a;
    r1a.phase = "AQUEOUS";
    r1a.solvent = "H2O";
    r1a.reactants = Comps({ "HSO3m", "H2O2_aq" });
    r1a.products = Comps({ "SO2OOHm", "H2O" });
    r1a.forward_rate_constants = { { "CLOUD", Arr(c_H2O_M * (7.45e7 / 13.0), 4430.0) } };
    r1a.equilibrium_constant = Arr(1725.0, 0.0);

    // R1b: SO2OOH- + H+ → SO4--  (irreversible)
    types::DissolvedReaction r1b;
    r1b.phase = "AQUEOUS";
    r1b.solvent = "H2O";
    r1b.reactants = Comps({ "SO2OOHm", "Hp" });
    r1b.products = Comps({ "SO4mm" });
    r1b.rate_constants = { { "CLOUD", Arr(c_H2O_M * 2.4e6, 4430.0) } };

    // R2: HSO3- + O3_aq → SO4-- + H+
    types::DissolvedReaction r2;
    r2.phase = "AQUEOUS";
    r2.solvent = "H2O";
    r2.reactants = Comps({ "HSO3m", "O3_aq" });
    r2.products = Comps({ "SO4mm", "Hp" });
    r2.rate_constants = { { "CLOUD", Arr(c_H2O_M * 3.75e5, 5530.0) } };

    // R3: SO3-- + O3_aq → SO4--
    types::DissolvedReaction r3;
    r3.phase = "AQUEOUS";
    r3.solvent = "H2O";
    r3.reactants = Comps({ "SO3mm", "O3_aq" });
    r3.products = Comps({ "SO4mm" });
    r3.rate_constants = { { "CLOUD", Arr(c_H2O_M * 1.59e9, 5280.0) } };

    aerosol.processes = { r1a, r1b, r2, r3 };

    // Henry's Law equilibria (gas ⇌ dissolved)
    auto henry_eq = [](const std::string& gas_species, const std::string& condensed_species, types::HenryLawConstant hlc)
    {
      types::HenryLawEquilibrium h;
      h.gas_phase = "gas";
      h.gas_species = gas_species;
      h.condensed_phase = "AQUEOUS";
      h.condensed_species = condensed_species;
      h.solvent = "H2O";
      h.henry_law_constant = hlc;
      h.solvent_molecular_weight = MW_H2O;
      h.solvent_density = RHO_H2O;
      return h;
    };

    // Dissolved equilibria
    auto dissolved_eq = [](const std::vector<std::string>& reactants,
                           const std::vector<std::string>& products,
                           const std::string& algebraic_species,
                           types::ArrheniusReferenceTemperature eq)
    {
      types::DissolvedEquilibrium d;
      d.phase = "AQUEOUS";
      d.reactants = Comps(reactants);
      d.products = Comps(products);
      d.algebraic_species = algebraic_species;
      d.solvent = "H2O";
      d.equilibrium_constant = eq;
      return d;
    };

    // Mass-conservation / charge-balance linear constraints
    auto linear = [](const std::string& algebraic_phase,
                     const std::string& algebraic_species,
                     std::vector<types::LinearConstraintTerm> terms,
                     std::variant<types::FixedConstant, types::DiagnoseFromState> constant)
    {
      types::LinearConstraint lc;
      lc.algebraic_phase = algebraic_phase;
      lc.algebraic_species = algebraic_species;
      lc.terms = std::move(terms);
      lc.constant = constant;
      return lc;
    };

    aerosol.constraints = {
      henry_eq("SO2", "SO2_aq", Henry(1.23 * M_ATM_TO_MOL_M3_PA, 3120.0)),
      henry_eq("H2O2", "H2O2_aq", Henry(7.4e4 * M_ATM_TO_MOL_M3_PA, 6621.0)),
      henry_eq("O3", "O3_aq", Henry(1.15e-2 * M_ATM_TO_MOL_M3_PA, 2560.0)),
      // Kw: H2O ⇌ H+ + OH-
      dissolved_eq({ "H2O" }, { "Hp", "OHm" }, "OHm", Arr(1e-14 / (c_H2O_M * c_H2O_M), 0.0)),
      // Ka1: SO2_aq ⇌ HSO3- + H+
      dissolved_eq({ "SO2_aq" }, { "HSO3m", "Hp" }, "HSO3m", Arr(1.7e-2 / c_H2O_M, 2090.0)),
      // Ka2: HSO3- ⇌ SO3-- + H+
      dissolved_eq({ "HSO3m" }, { "SO3mm", "Hp" }, "SO3mm", Arr(6.0e-8 / c_H2O_M, 1120.0)),
      // Total S conservation (diagnosed from the initial state), algebraic species SO2 (gas)
      linear(
          "gas",
          "SO2",
          { { .phase = "gas", .name = "SO2", .coefficient = 1.0 },
            { .phase = "AQUEOUS", .name = "SO2_aq", .coefficient = 1.0 },
            { .phase = "AQUEOUS", .name = "HSO3m", .coefficient = 1.0 },
            { .phase = "AQUEOUS", .name = "SO3mm", .coefficient = 1.0 },
            { .phase = "AQUEOUS", .name = "SO4mm", .coefficient = 1.0 },
            { .phase = "AQUEOUS", .name = "SO2OOHm", .coefficient = 1.0 } },
          types::DiagnoseFromState{}),
      // Total H2O2 conservation
      linear(
          "gas",
          "H2O2",
          { { .phase = "gas", .name = "H2O2", .coefficient = 1.0 },
            { .phase = "AQUEOUS", .name = "H2O2_aq", .coefficient = 1.0 } },
          types::DiagnoseFromState{}),
      // Total O3 conservation
      linear(
          "gas",
          "O3",
          { { .phase = "gas", .name = "O3", .coefficient = 1.0 },
            { .phase = "AQUEOUS", .name = "O3_aq", .coefficient = 1.0 } },
          types::DiagnoseFromState{}),
      // Charge balance: H+ = OH- + HSO3- + 2*SO3-- + 2*SO4-- + SO2OOH-
      linear(
          "AQUEOUS",
          "Hp",
          { { .phase = "AQUEOUS", .name = "Hp", .coefficient = 1.0 },
            { .phase = "AQUEOUS", .name = "OHm", .coefficient = -1.0 },
            { .phase = "AQUEOUS", .name = "HSO3m", .coefficient = -1.0 },
            { .phase = "AQUEOUS", .name = "SO3mm", .coefficient = -2.0 },
            { .phase = "AQUEOUS", .name = "SO4mm", .coefficient = -2.0 },
            { .phase = "AQUEOUS", .name = "SO2OOHm", .coefficient = -1.0 } },
          types::FixedConstant{ 0.0 }),
    };

    return MakeAerosolMechanism(
        "cloud_chemistry",
        { MakeSpecies("SO2"),
          MakeSpecies("H2O2"),
          MakeSpecies("O3"),
          MakeSpecies("SO2_aq"),
          MakeSpecies("H2O2_aq"),
          MakeSpecies("O3_aq"),
          MakeSpecies("Hp"),
          MakeSpecies("OHm"),
          MakeSpecies("HSO3m"),
          MakeSpecies("SO3mm"),
          MakeSpecies("SO4mm"),
          MakeSpecies("SO2OOHm"),
          MakeSpecies("H2O", MW_H2O, RHO_H2O) },
        { { "SO2", 1.28e-5 }, { "H2O2", 1.46e-5 }, { "O3", 1.48e-5 } },
        { { "H2O", std::nullopt, RHO_H2O },
          "SO2_aq",
          "H2O2_aq",
          "O3_aq",
          "Hp",
          "OHm",
          "HSO3m",
          "SO3mm",
          "SO4mm",
          "SO2OOHm" },
        std::move(aerosol));
  }
}  // anonymous namespace

// ═══ Tests ══════════════════════════════════════════════════════════════════

TEST(MiamBuilder, CreateMicmWithMiam_DAE4)
{
  auto mechanism = CreateCloudChemistryMechanism();

  musica::Error error;
  musica::MICM* micm = musica::CreateMicmWithMiam(mechanism, musica::MICMSolver::RosenbrockDAE4StandardOrder, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  // Verify we can create a state
  musica::State state(*micm, 1);

  // Set initial conditions
  state.SetConditions({ { .temperature_ = 280.0, .pressure_ = 70000.0 } });

  delete micm;
  musica::DeleteError(&error);
}

TEST(MiamBuilder, CreateMicmWithMiam_DAE6)
{
  auto mechanism = CreateCloudChemistryMechanism();

  musica::Error error;
  musica::MICM* micm = musica::CreateMicmWithMiam(mechanism, musica::MICMSolver::RosenbrockDAE6StandardOrder, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  delete micm;
  musica::DeleteError(&error);
}

TEST(MiamBuilder, CreateMicmWithMiam_Rosenbrock)
{
  auto mechanism = CreateCloudChemistryMechanism();

  musica::Error error;
  musica::MICM* micm = musica::CreateMicmWithMiam(mechanism, musica::MICMSolver::RosenbrockStandardOrder, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  delete micm;
  musica::DeleteError(&error);
}

TEST(MiamBuilder, InvalidSpeciesName)
{
  auto mechanism = CreateCloudChemistryMechanism();

  // Add a process referencing a species that doesn't exist
  types::DissolvedReaction bad_rxn;
  bad_rxn.phase = "AQUEOUS";
  bad_rxn.solvent = "H2O";
  bad_rxn.reactants = Comps({ "NONEXISTENT_SPECIES" });
  bad_rxn.products = Comps({ "SO4mm" });
  bad_rxn.rate_constants = { { "CLOUD", Arr(1.0, 0.0) } };
  mechanism.aerosol->processes.push_back(bad_rxn);

  musica::Error error;
  musica::MICM* micm = musica::CreateMicmWithMiam(mechanism, musica::MICMSolver::RosenbrockDAE4StandardOrder, &error);

  EXPECT_FALSE(musica::IsSuccess(error));
  EXPECT_EQ(micm, nullptr);
  musica::DeleteError(&error);
}

TEST(MiamBuilder, InvalidPhaseName)
{
  auto mechanism = CreateCloudChemistryMechanism();

  // Add a process referencing a phase that doesn't exist
  types::DissolvedReaction bad_rxn;
  bad_rxn.phase = "NONEXISTENT_PHASE";
  bad_rxn.solvent = "H2O";
  bad_rxn.reactants = Comps({ "HSO3m" });
  bad_rxn.products = Comps({ "SO4mm" });
  bad_rxn.rate_constants = { { "CLOUD", Arr(1.0, 0.0) } };
  mechanism.aerosol->processes.push_back(bad_rxn);

  musica::Error error;
  musica::MICM* micm = musica::CreateMicmWithMiam(mechanism, musica::MICMSolver::RosenbrockDAE4StandardOrder, &error);

  EXPECT_FALSE(musica::IsSuccess(error));
  EXPECT_EQ(micm, nullptr);
  musica::DeleteError(&error);
}

TEST(MiamBuilder, CallbackRateConstant)
{
  auto mechanism = CreateCloudChemistryMechanism();

  // Replace R1b's Arrhenius rate constant with a std::function callback.
  // Processes are ordered { R1a (reversible), R1b, R2, R3 }.
  auto& rxn = std::get<types::DissolvedReaction>(mechanism.aerosol->processes[1]);
  rxn.rate_constants["CLOUD"] = std::function<double(double)>(
      [](double T) -> double
      {
        constexpr double c_H2O_M = 55.556;
        return c_H2O_M * 2.4e6 * std::exp(-4430.0 * (1.0 / T - 1.0 / 298.15));
      });

  musica::Error error;
  musica::MICM* micm = musica::CreateMicmWithMiam(mechanism, musica::MICMSolver::RosenbrockDAE4StandardOrder, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  delete micm;
  musica::DeleteError(&error);
}

TEST(MiamBuilder, EmptyProcessesAndConstraints)
{
  // Minimal aerosol: just a representation — no processes or constraints
  types::Aerosol aerosol;
  aerosol.representations = {
    types::UniformSection{ .name = "CLOUD", .phases = { "AQUEOUS" }, .min_radius = 1e-6, .max_radius = 1e-5 },
  };
  auto mechanism = MakeAerosolMechanism(
      "minimal",
      { MakeSpecies("SO2"), MakeSpecies("H2O", MW_H2O, RHO_H2O), MakeSpecies("SO2_aq") },
      { "SO2" },
      { "H2O", "SO2_aq" },
      std::move(aerosol));

  musica::Error error;
  musica::MICM* micm = musica::CreateMicmWithMiam(mechanism, musica::MICMSolver::RosenbrockStandardOrder, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  delete micm;
  musica::DeleteError(&error);
}

TEST(MiamBuilder, SingleMomentModeRepresentation)
{
  types::Aerosol aerosol;
  aerosol.representations = {
    types::SingleMomentMode{
        .name = "AEROSOL", .phases = { "AQUEOUS" }, .geometric_mean_radius = 1e-7, .geometric_standard_deviation = 1.5 },
  };
  auto mechanism = MakeAerosolMechanism(
      "single_moment",
      { MakeSpecies("SO2"), MakeSpecies("H2O", MW_H2O, RHO_H2O), MakeSpecies("SO2_aq") },
      { "SO2" },
      { "H2O", "SO2_aq" },
      std::move(aerosol));

  musica::Error error;
  musica::MICM* micm = musica::CreateMicmWithMiam(mechanism, musica::MICMSolver::RosenbrockStandardOrder, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  delete micm;
  musica::DeleteError(&error);
}

TEST(MiamBuilder, TwoMomentModeRepresentation)
{
  types::Aerosol aerosol;
  aerosol.representations = {
    types::TwoMomentMode{ .name = "AEROSOL", .phases = { "AQUEOUS" }, .geometric_standard_deviation = 1.5 },
  };
  auto mechanism = MakeAerosolMechanism(
      "two_moment",
      { MakeSpecies("SO2"), MakeSpecies("H2O", MW_H2O, RHO_H2O), MakeSpecies("SO2_aq") },
      { "SO2" },
      { "H2O", "SO2_aq" },
      std::move(aerosol));

  musica::Error error;
  musica::MICM* micm = musica::CreateMicmWithMiam(mechanism, musica::MICMSolver::RosenbrockStandardOrder, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  delete micm;
  musica::DeleteError(&error);
}

TEST(MiamBuilder, HenryLawPhaseTransferProcess)
{
  types::Aerosol aerosol;
  aerosol.representations = {
    types::UniformSection{ .name = "CLOUD", .phases = { "AQUEOUS" }, .min_radius = 1e-6, .max_radius = 1e-5 },
  };

  // Henry's Law phase transfer instead of a dissolved reaction
  types::HenryLawPhaseTransfer transfer;
  transfer.gas_phase = "gas";
  transfer.gas_species = "SO2";
  transfer.condensed_phase = "AQUEOUS";
  transfer.condensed_species = "SO2_aq";
  transfer.solvent = "H2O";
  transfer.henry_law_constant = Henry(1.23 * M_ATM_TO_MOL_M3_PA, 3120.0);
  transfer.diffusion_coefficient = 1.28e-5;
  transfer.accommodation_coefficient = 0.11;
  aerosol.processes = { transfer };

  auto mechanism = MakeAerosolMechanism(
      "henry_transfer",
      { MakeSpecies("SO2", 0.064, 1.46),  // MW + density needed for phase transfer
        MakeSpecies("H2O2"),
        MakeSpecies("O3"),
        MakeSpecies("H2O", MW_H2O, RHO_H2O),
        MakeSpecies("SO2_aq", 0.064, 1.46) },
      { { "SO2", 1.28e-5 }, { "H2O2", 1.46e-5 }, { "O3", 1.48e-5 } },
      { { "H2O", std::nullopt, RHO_H2O }, "SO2_aq" },
      std::move(aerosol));

  musica::Error error;
  musica::MICM* micm = musica::CreateMicmWithMiam(mechanism, musica::MICMSolver::RosenbrockStandardOrder, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  delete micm;
  musica::DeleteError(&error);
}

TEST(MiamBuilder, DissolvedReversibleReactionProcess)
{
  types::Aerosol aerosol;
  aerosol.representations = {
    types::UniformSection{ .name = "CLOUD", .phases = { "AQUEOUS" }, .min_radius = 1e-6, .max_radius = 1e-5 },
  };

  // SO2_aq ⇌ HSO3- + H+ (forward + equilibrium constant, per representation "CLOUD")
  types::DissolvedReversibleReaction rxn;
  rxn.phase = "AQUEOUS";
  rxn.solvent = "H2O";
  rxn.reactants = Comps({ "SO2_aq" });
  rxn.products = Comps({ "HSO3m", "Hp" });
  rxn.forward_rate_constants = { { "CLOUD", Arr(1e6, 0.0) } };
  rxn.equilibrium_constant = Arr(1.7e-2 / c_H2O_M, 2090.0);
  aerosol.processes = { rxn };

  auto mechanism = MakeAerosolMechanism(
      "reversible",
      { MakeSpecies("SO2"),
        MakeSpecies("H2O2"),
        MakeSpecies("O3"),
        MakeSpecies("H2O", MW_H2O, RHO_H2O),
        MakeSpecies("SO2_aq"),
        MakeSpecies("HSO3m"),
        MakeSpecies("Hp") },
      { "SO2", "H2O2", "O3" },
      { "H2O", "SO2_aq", "HSO3m", "Hp" },
      std::move(aerosol));

  musica::Error error;
  musica::MICM* micm = musica::CreateMicmWithMiam(mechanism, musica::MICMSolver::RosenbrockStandardOrder, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  delete micm;
  musica::DeleteError(&error);
}
