// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Unit tests for the MIAM builder: config → miam::Model → MICM solver

#include <musica/miam/miam_builder.hpp>
#include <musica/miam/miam_types.hpp>
#include <musica/micm/micm.hpp>
#include <musica/micm/parse.hpp>
#include <musica/micm/state.hpp>
#include <musica/util.hpp>

#include <gtest/gtest.h>

#include <cmath>
#include <string>
#include <vector>

namespace mc = musica::miam_config;

// ═══ Helper: minimal CAM Cloud Chemistry configuration ══════════════════════
//
// This mirrors the MIAM integration test with SO2/H2O2/O3 sulfate chemistry.
// Simplified to a single forward reaction (R1: HSO3- + H2O2_aq → SO4-- + H2O + H+)
// so the test is small but exercises the full build pipeline.

namespace
{
  constexpr double M_ATM_TO_MOL_M3_PA = 1000.0 / 101325.0;
  constexpr double c_H2O_M = 55.556;  // mol/L
  constexpr double MW_H2O = 0.018;    // kg/mol
  constexpr double RHO_H2O = 1000.0;  // kg/m3

  // Build a gas mechanism with SO2, H2O2, O3 as gas species
  musica::Chemistry CreateGasMechanism()
  {
    // Minimal mechanism: gas phase with the 3 gas species, no gas reactions
    // (all reactions happen in the MIAM external model).
    namespace v1 = mechanism_configuration::v1::types;

    v1::Mechanism mech;

    // Gas-phase species
    v1::Species so2_g, h2o2_g, o3_g;
    so2_g.name = "SO2";
    h2o2_g.name = "H2O2";
    o3_g.name = "O3";
    mech.species = { so2_g, h2o2_g, o3_g };

    // Gas phase (species are PhaseSpecies, not strings)
    v1::Phase gas_phase;
    gas_phase.name = "gas";
    gas_phase.species = {
        v1::PhaseSpecies{ .name = "SO2" },
        v1::PhaseSpecies{ .name = "H2O2" },
        v1::PhaseSpecies{ .name = "O3" },
    };
    mech.phases.push_back(gas_phase);

    return musica::ConvertV1Mechanism(mech);
  }

  // Build MIAM model config for simplified cloud chemistry
  mc::ModelConfig CreateCloudChemistryConfig()
  {
    mc::ModelConfig config;
    config.name = "cloud_chemistry";

    // Species
    config.species = {
      { "SO2", {}, {} },
      { "H2O2", {}, {} },
      { "O3", {}, {} },
      { "SO2_aq", {}, {} },
      { "H2O2_aq", {}, {} },
      { "O3_aq", {}, {} },
      { "Hp", {}, {} },        // H+
      { "OHm", {}, {} },       // OH-
      { "HSO3m", {}, {} },     // HSO3-
      { "SO3mm", {}, {} },     // SO3--
      { "SO4mm", {}, {} },     // SO4--
      { "H2O", MW_H2O, RHO_H2O },
    };

    // Condensed phase
    config.condensed_phases = { {
        "AQUEOUS",
        { "H2O", "SO2_aq", "H2O2_aq", "O3_aq", "Hp", "OHm", "HSO3m", "SO3mm", "SO4mm" },
    } };

    // Representation: uniform cloud section
    config.representations = { mc::UniformSection{ "CLOUD", { "AQUEOUS" }, 1e-6, 1e-5 } };

    // Processes: single dissolved reaction R1 with callable rate constant
    // R1: HSO3- + H2O2_aq → SO4-- + H2O + H+
    // k = c_H2O_M * 7.45e7 * exp(-4430 * (1/T - 1/298.15))
    mc::DissolvedReaction r1;
    r1.phase_name = "AQUEOUS";
    r1.reactant_names = { "HSO3m", "H2O2_aq" };
    r1.product_names = { "SO4mm", "H2O", "Hp" };
    r1.solvent_name = "H2O";
    r1.rate_constant = mc::ArrheniusRateConstant{ c_H2O_M * 7.45e7, 4430.0 };
    config.processes = { r1 };

    // Constraints

    // Henry's Law equilibrium for SO2
    config.constraints.push_back(mc::HenryLawEquilibriumConstraint{
        "SO2", "SO2_aq", "H2O", "AQUEOUS",
        { 1.23 * M_ATM_TO_MOL_M3_PA, 3120.0 },
        MW_H2O, RHO_H2O });

    // Henry's Law equilibrium for H2O2
    config.constraints.push_back(mc::HenryLawEquilibriumConstraint{
        "H2O2", "H2O2_aq", "H2O", "AQUEOUS",
        { 7.4e4 * M_ATM_TO_MOL_M3_PA, 6621.0 },
        MW_H2O, RHO_H2O });

    // Henry's Law equilibrium for O3
    config.constraints.push_back(mc::HenryLawEquilibriumConstraint{
        "O3", "O3_aq", "H2O", "AQUEOUS",
        { 1.15e-2 * M_ATM_TO_MOL_M3_PA, 2560.0 },
        MW_H2O, RHO_H2O });

    // Dissolved equilibrium: Kw  (H2O ⇌ H+ + OH-)
    config.constraints.push_back(mc::DissolvedEquilibriumConstraint{
        "AQUEOUS",
        { "H2O" },
        { "Hp", "OHm" },
        "OHm",
        "H2O",
        { 1e-14 / (c_H2O_M * c_H2O_M), 0.0 } });

    // Dissolved equilibrium: Ka1  (SO2_aq ⇌ HSO3- + H+)
    config.constraints.push_back(mc::DissolvedEquilibriumConstraint{
        "AQUEOUS",
        { "SO2_aq" },
        { "HSO3m", "Hp" },
        "HSO3m",
        "H2O",
        { 1.7e-2 / c_H2O_M, 2090.0 } });

    // Dissolved equilibrium: Ka2  (HSO3- ⇌ SO3-- + H+)
    config.constraints.push_back(mc::DissolvedEquilibriumConstraint{
        "AQUEOUS",
        { "HSO3m" },
        { "SO3mm", "Hp" },
        "SO3mm",
        "H2O",
        { 6.0e-8 / c_H2O_M, 1120.0 } });

    // Linear constraints for mass conservation and charge balance

    // Mass conservation: SO2_g + SO2_aq + HSO3- + SO3-- + SO4-- = total_S
    // Algebraic species: SO2 (gas)
    mc::LinearConstraint mass_s;
    mass_s.algebraic_phase_name = "gas";
    mass_s.algebraic_species_name = "SO2";
    mass_s.terms = {
        { "gas", "SO2", 1.0 },
        { "AQUEOUS", "SO2_aq", 1.0 },
        { "AQUEOUS", "HSO3m", 1.0 },
        { "AQUEOUS", "SO3mm", 1.0 },
        { "AQUEOUS", "SO4mm", 1.0 },
    };
    mass_s.constant = 0.0;  // will be set from initial conditions
    config.constraints.push_back(mass_s);

    // Mass conservation: H2O2_g + H2O2_aq = total_H2O2
    mc::LinearConstraint mass_h2o2;
    mass_h2o2.algebraic_phase_name = "gas";
    mass_h2o2.algebraic_species_name = "H2O2";
    mass_h2o2.terms = {
        { "gas", "H2O2", 1.0 },
        { "AQUEOUS", "H2O2_aq", 1.0 },
    };
    mass_h2o2.constant = 0.0;
    config.constraints.push_back(mass_h2o2);

    // Mass conservation: O3_g + O3_aq = total_O3
    mc::LinearConstraint mass_o3;
    mass_o3.algebraic_phase_name = "gas";
    mass_o3.algebraic_species_name = "O3";
    mass_o3.terms = {
        { "gas", "O3", 1.0 },
        { "AQUEOUS", "O3_aq", 1.0 },
    };
    mass_o3.constant = 0.0;
    config.constraints.push_back(mass_o3);

    // Charge balance: H+ = OH- + HSO3- + 2*SO3-- + 2*SO4--
    mc::LinearConstraint charge;
    charge.algebraic_phase_name = "AQUEOUS";
    charge.algebraic_species_name = "Hp";
    charge.terms = {
        { "AQUEOUS", "Hp", 1.0 },
        { "AQUEOUS", "OHm", -1.0 },
        { "AQUEOUS", "HSO3m", -1.0 },
        { "AQUEOUS", "SO3mm", -2.0 },
        { "AQUEOUS", "SO4mm", -2.0 },
    };
    charge.constant = 0.0;
    config.constraints.push_back(charge);

    return config;
  }
}  // anonymous namespace

// ═══ Tests ══════════════════════════════════════════════════════════════════

TEST(MiamBuilder, CreateMicmWithMiam_DAE4)
{
  auto chemistry = CreateGasMechanism();
  auto miam_config = CreateCloudChemistryConfig();

  musica::Error error;
  musica::MICM* micm =
      musica::CreateMicmWithMiam(chemistry, musica::MICMSolver::RosenbrockDAE4StandardOrder, miam_config, &error);

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
  auto chemistry = CreateGasMechanism();
  auto miam_config = CreateCloudChemistryConfig();

  musica::Error error;
  musica::MICM* micm =
      musica::CreateMicmWithMiam(chemistry, musica::MICMSolver::RosenbrockDAE6StandardOrder, miam_config, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  delete micm;
  musica::DeleteError(&error);
}

TEST(MiamBuilder, CreateMicmWithMiam_Rosenbrock)
{
  auto chemistry = CreateGasMechanism();
  auto miam_config = CreateCloudChemistryConfig();

  musica::Error error;
  musica::MICM* micm =
      musica::CreateMicmWithMiam(chemistry, musica::MICMSolver::RosenbrockStandardOrder, miam_config, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  delete micm;
  musica::DeleteError(&error);
}

TEST(MiamBuilder, InvalidSpeciesName)
{
  auto chemistry = CreateGasMechanism();
  auto miam_config = CreateCloudChemistryConfig();

  // Add a process referencing a species that doesn't exist
  mc::DissolvedReaction bad_rxn;
  bad_rxn.phase_name = "AQUEOUS";
  bad_rxn.reactant_names = { "NONEXISTENT_SPECIES" };
  bad_rxn.product_names = { "SO4mm" };
  bad_rxn.solvent_name = "H2O";
  bad_rxn.rate_constant = mc::ArrheniusRateConstant{ 1.0, 0.0 };
  miam_config.processes.push_back(bad_rxn);

  musica::Error error;
  musica::MICM* micm =
      musica::CreateMicmWithMiam(chemistry, musica::MICMSolver::RosenbrockDAE4StandardOrder, miam_config, &error);

  EXPECT_FALSE(musica::IsSuccess(error));
  EXPECT_EQ(micm, nullptr);
  musica::DeleteError(&error);
}

TEST(MiamBuilder, InvalidPhaseName)
{
  auto chemistry = CreateGasMechanism();
  auto miam_config = CreateCloudChemistryConfig();

  // Add a process referencing a phase that doesn't exist
  mc::DissolvedReaction bad_rxn;
  bad_rxn.phase_name = "NONEXISTENT_PHASE";
  bad_rxn.reactant_names = { "HSO3m" };
  bad_rxn.product_names = { "SO4mm" };
  bad_rxn.solvent_name = "H2O";
  bad_rxn.rate_constant = mc::ArrheniusRateConstant{ 1.0, 0.0 };
  miam_config.processes.push_back(bad_rxn);

  musica::Error error;
  musica::MICM* micm =
      musica::CreateMicmWithMiam(chemistry, musica::MICMSolver::RosenbrockDAE4StandardOrder, miam_config, &error);

  EXPECT_FALSE(musica::IsSuccess(error));
  EXPECT_EQ(micm, nullptr);
  musica::DeleteError(&error);
}

TEST(MiamBuilder, CallbackRateConstant)
{
  auto chemistry = CreateGasMechanism();
  auto miam_config = CreateCloudChemistryConfig();

  // Replace the Arrhenius rate constant with a std::function callback
  auto& rxn = std::get<mc::DissolvedReaction>(miam_config.processes[0]);
  rxn.rate_constant = std::function<double(double)>(
      [](double T) -> double
      {
        constexpr double c_H2O_M = 55.556;
        return c_H2O_M * 7.45e7 * std::exp(-4430.0 * (1.0 / T - 1.0 / 298.15));
      });

  musica::Error error;
  musica::MICM* micm =
      musica::CreateMicmWithMiam(chemistry, musica::MICMSolver::RosenbrockDAE4StandardOrder, miam_config, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  delete micm;
  musica::DeleteError(&error);
}

TEST(MiamBuilder, EmptyProcessesAndConstraints)
{
  auto chemistry = CreateGasMechanism();

  // Minimal config: just species, phases, and a representation — no processes or constraints
  mc::ModelConfig config;
  config.name = "minimal";
  config.species = {
    { "SO2", {}, {} },
    { "H2O", MW_H2O, RHO_H2O },
    { "SO2_aq", {}, {} },
  };
  config.condensed_phases = { { "AQUEOUS", { "H2O", "SO2_aq" } } };
  config.representations = { mc::UniformSection{ "CLOUD", { "AQUEOUS" }, 1e-6, 1e-5 } };

  musica::Error error;
  musica::MICM* micm =
      musica::CreateMicmWithMiam(chemistry, musica::MICMSolver::RosenbrockStandardOrder, config, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  delete micm;
  musica::DeleteError(&error);
}

TEST(MiamBuilder, SingleMomentModeRepresentation)
{
  auto chemistry = CreateGasMechanism();

  mc::ModelConfig config;
  config.name = "single_moment";
  config.species = {
    { "SO2", {}, {} },
    { "H2O", MW_H2O, RHO_H2O },
    { "SO2_aq", {}, {} },
  };
  config.condensed_phases = { { "AQUEOUS", { "H2O", "SO2_aq" } } };
  config.representations = { mc::SingleMomentMode{ "AEROSOL", { "AQUEOUS" }, 1e-7, 1.5 } };

  musica::Error error;
  musica::MICM* micm =
      musica::CreateMicmWithMiam(chemistry, musica::MICMSolver::RosenbrockStandardOrder, config, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  delete micm;
  musica::DeleteError(&error);
}

TEST(MiamBuilder, TwoMomentModeRepresentation)
{
  auto chemistry = CreateGasMechanism();

  mc::ModelConfig config;
  config.name = "two_moment";
  config.species = {
    { "SO2", {}, {} },
    { "H2O", MW_H2O, RHO_H2O },
    { "SO2_aq", {}, {} },
  };
  config.condensed_phases = { { "AQUEOUS", { "H2O", "SO2_aq" } } };
  config.representations = { mc::TwoMomentMode{ "AEROSOL", { "AQUEOUS" }, 1.5 } };

  musica::Error error;
  musica::MICM* micm =
      musica::CreateMicmWithMiam(chemistry, musica::MICMSolver::RosenbrockStandardOrder, config, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  delete micm;
  musica::DeleteError(&error);
}

TEST(MiamBuilder, HenryLawPhaseTransferProcess)
{
  auto chemistry = CreateGasMechanism();

  mc::ModelConfig config;
  config.name = "henry_transfer";
  config.species = {
    { "SO2", 0.064, 1.46 },   // MW + density needed for phase transfer
    { "H2O2", {}, {} },
    { "O3", {}, {} },
    { "H2O", MW_H2O, RHO_H2O },
    { "SO2_aq", 0.064, 1.46 },
  };
  config.condensed_phases = { { "AQUEOUS", { "H2O", "SO2_aq" } } };
  config.representations = { mc::UniformSection{ "CLOUD", { "AQUEOUS" }, 1e-6, 1e-5 } };

  // Add a Henry's Law phase transfer instead of a dissolved reaction
  mc::HenryLawPhaseTransfer transfer;
  transfer.condensed_phase_name = "AQUEOUS";
  transfer.gas_species_name = "SO2";
  transfer.condensed_species_name = "SO2_aq";
  transfer.solvent_name = "H2O";
  transfer.henrys_law_constant = { 1.23 * M_ATM_TO_MOL_M3_PA, 3120.0 };
  transfer.diffusion_coefficient = 1.28e-5;
  transfer.accommodation_coefficient = 0.11;
  config.processes = { transfer };

  musica::Error error;
  musica::MICM* micm =
      musica::CreateMicmWithMiam(chemistry, musica::MICMSolver::RosenbrockStandardOrder, config, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  delete micm;
  musica::DeleteError(&error);
}

TEST(MiamBuilder, DissolvedReversibleReactionProcess)
{
  auto chemistry = CreateGasMechanism();

  mc::ModelConfig config;
  config.name = "reversible";
  config.species = {
    { "SO2", {}, {} },
    { "H2O2", {}, {} },
    { "O3", {}, {} },
    { "H2O", MW_H2O, RHO_H2O },
    { "SO2_aq", {}, {} },
    { "HSO3m", {}, {} },
    { "Hp", {}, {} },
  };
  config.condensed_phases = { { "AQUEOUS", { "H2O", "SO2_aq", "HSO3m", "Hp" } } };
  config.representations = { mc::UniformSection{ "CLOUD", { "AQUEOUS" }, 1e-6, 1e-5 } };

  // SO2_aq ⇌ HSO3- + H+ (with equilibrium constant)
  mc::DissolvedReversibleReaction rxn;
  rxn.phase_name = "AQUEOUS";
  rxn.reactant_names = { "SO2_aq" };
  rxn.product_names = { "HSO3m", "Hp" };
  rxn.solvent_name = "H2O";
  rxn.forward_rate_constant = mc::ArrheniusRateConstant{ 1e6, 0.0 };
  rxn.equilibrium_constant = mc::EquilibriumConstant{ 1.7e-2 / c_H2O_M, 2090.0 };
  config.processes = { rxn };

  musica::Error error;
  musica::MICM* micm =
      musica::CreateMicmWithMiam(chemistry, musica::MICMSolver::RosenbrockStandardOrder, config, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  delete micm;
  musica::DeleteError(&error);
}
