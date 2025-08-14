#include <musica/micm/parse.hpp>

#include <gtest/gtest.h>
#include <mechanism_configuration/parser.hpp>
#include <mechanism_configuration/v0/types.hpp>
#include <mechanism_configuration/v1/types.hpp>

static constexpr double avogadro = 6.02214076e23;  // # mol^{-1}
static constexpr double MolesM3ToMoleculesCm3 = 1.0e-6 * avogadro;

TEST(Parser, BadConfigurationFilePath)
{
  mechanism_configuration::UniversalParser parser;
  auto parsed = parser.Parse("bad config path");
  EXPECT_FALSE(parsed);
}

TEST(Parser, Version0Configuration)
{
  mechanism_configuration::UniversalParser parser;
  auto parsed = parser.Parse("configs/v0/chapman");
  EXPECT_TRUE(parsed);

  using V0 = mechanism_configuration::v0::types::Mechanism;
  V0* v0_mechanism = dynamic_cast<V0*>(parsed.mechanism.get());

  EXPECT_EQ(v0_mechanism->name, "Chapman");
  EXPECT_EQ(v0_mechanism->version.major, 0);
  EXPECT_EQ(v0_mechanism->version.minor, 0);
  EXPECT_EQ(v0_mechanism->version.patch, 0);
  EXPECT_EQ(v0_mechanism->reactions.arrhenius.size(), 4);
  EXPECT_EQ(v0_mechanism->reactions.user_defined.size(), 3);
  EXPECT_EQ(v0_mechanism->species.size(), 5);
}

TEST(Parser, Version1Configuration)
{
  mechanism_configuration::UniversalParser parser;
  auto parsed = parser.Parse("configs/v1/chapman/config.json");
  EXPECT_TRUE(parsed);

  using V1 = mechanism_configuration::v1::types::Mechanism;
  V1* v1_mechanism = dynamic_cast<V1*>(parsed.mechanism.get());

  EXPECT_EQ(v1_mechanism->name, "Chapman");
  EXPECT_EQ(v1_mechanism->version.major, 1);
  EXPECT_EQ(v1_mechanism->version.minor, 0);
  EXPECT_EQ(v1_mechanism->version.patch, 0);
  EXPECT_EQ(v1_mechanism->reactions.arrhenius.size(), 4);
  EXPECT_EQ(v1_mechanism->reactions.photolysis.size(), 3);
  EXPECT_EQ(v1_mechanism->species.size(), 5);
}

TEST(Parser, CanParseChapmanV0)
{
  musica::Chemistry chemistry = musica::ReadConfiguration("configs/v0/chapman");
  EXPECT_EQ(chemistry.system.gas_phase_.species_.size(), 5);
  EXPECT_EQ(chemistry.processes.size(), 7);
  EXPECT_EQ(chemistry.system.gas_phase_.species_[0].name_, "M");
  EXPECT_NE(chemistry.system.gas_phase_.species_[0].parameterize_, nullptr);
  EXPECT_EQ(chemistry.system.gas_phase_.species_[1].name_, "O2");
  EXPECT_EQ(chemistry.system.gas_phase_.species_[2].name_, "O");
  EXPECT_EQ(chemistry.system.gas_phase_.species_[3].name_, "O1D");
  EXPECT_EQ(chemistry.system.gas_phase_.species_[4].name_, "O3");
}

TEST(Parser, CanParseCBVV0)
{
  musica::Chemistry chemistry = musica::ReadConfiguration("configs/v0/carbon_bond_5");
  EXPECT_EQ(chemistry.system.gas_phase_.species_.size(), 67);
  EXPECT_EQ(chemistry.processes.size(), 200);
}

TEST(Parser, CanParseTS1V0)
{
  musica::Chemistry chemistry = musica::ReadConfiguration("configs/v0/TS1");
  EXPECT_EQ(chemistry.system.gas_phase_.species_.size(), 210);
  EXPECT_EQ(chemistry.processes.size(), 547);
}

TEST(Parser, DetectsInvalidConfigV0)
{
  EXPECT_ANY_THROW(musica::ReadConfiguration("configs/v0/invalid"));
}

TEST(Parser, CanParseChapmanV1)
{
  std::vector<std::string> extensions = { ".json", ".yaml" };

  for (const auto& extension : extensions)
  {
    musica::Chemistry chemistry = musica::ReadConfiguration("configs/v1/chapman/config" + extension);
    EXPECT_EQ(chemistry.system.gas_phase_.species_.size(), 5);
    EXPECT_EQ(chemistry.processes.size(), 7);
    EXPECT_EQ(chemistry.system.phases_.size(), 0);
    EXPECT_EQ(chemistry.system.gas_phase_.species_[0].name_, "M");
    EXPECT_NE(chemistry.system.gas_phase_.species_[0].parameterize_, nullptr);
    EXPECT_EQ(chemistry.system.gas_phase_.species_[1].name_, "O");
    EXPECT_EQ(chemistry.system.gas_phase_.species_[2].name_, "O2");
    EXPECT_EQ(chemistry.system.gas_phase_.species_[3].name_, "O3");
    EXPECT_EQ(chemistry.system.gas_phase_.species_[4].name_, "O1D");

    EXPECT_EQ(chemistry.system.gas_phase_.species_[3].GetProperty<std::string>("__long name"), "ozone");
  }
}

TEST(Parser, CanParseFullV1)
{
  std::vector<std::string> extensions = { ".json", ".yaml" };

  for (const auto& extension : extensions)
  {
    musica::Chemistry chemistry = musica::ReadConfiguration("configs/v1/full_configuration/full_configuration" + extension);
    EXPECT_EQ(chemistry.system.gas_phase_.species_.size(), 5);
    EXPECT_EQ(chemistry.system.gas_phase_.name_, "gas");
    EXPECT_EQ(chemistry.system.phases_.size(), 3);
    EXPECT_EQ(chemistry.processes.size(), 12);
  }
}

TEST(Parser, CanConvertFromV0ToV1)
{
  EXPECT_NO_THROW(mechanism_configuration::v1::types::Mechanism mechanism =
                      musica::ConvertV0MechanismToV1("configs/v0/chapman"););
  EXPECT_NO_THROW(mechanism_configuration::v1::types::Mechanism mechanism =
                      musica::ConvertV0MechanismToV1("configs/v0/analytical"););
  EXPECT_NO_THROW(mechanism_configuration::v1::types::Mechanism mechanism =
                      musica::ConvertV0MechanismToV1("configs/v0/carbon_bond_5"););
  EXPECT_NO_THROW(mechanism_configuration::v1::types::Mechanism mechanism =
                      musica::ConvertV0MechanismToV1("configs/v0/robertson"););
  EXPECT_NO_THROW(mechanism_configuration::v1::types::Mechanism mechanism =
                      musica::ConvertV0MechanismToV1("configs/v0/TS1"););
}

TEST(Parser, ConvertArrheniusV0ToV1)
{
  // Create a V0 mechanism with a single Arrhenius reaction
  mechanism_configuration::v0::types::Mechanism v0_mechanism;
  v0_mechanism.name = "Test Arrhenius";
  v0_mechanism.version.major = 0;
  v0_mechanism.version.minor = 0;
  v0_mechanism.version.patch = 0;

  // Add species
  mechanism_configuration::v0::types::Species speciesA, speciesB, speciesC;
  speciesA.name = "A";
  speciesB.name = "B";
  speciesC.name = "C";
  v0_mechanism.species = { speciesA, speciesB, speciesC };

  // Add Arrhenius reaction: A + B -> C
  mechanism_configuration::v0::types::Arrhenius arrhenius;
  arrhenius.A = 1.0e-11;
  arrhenius.B = 0.0;
  arrhenius.C = 200.0;
  arrhenius.D = 300.0;
  arrhenius.E = 0.0;

  mechanism_configuration::v0::types::ReactionComponent reactantA, reactantB, productC;
  reactantA.species_name = "A";
  reactantA.coefficient = 1.0;
  reactantB.species_name = "B";
  reactantB.coefficient = 1.0;
  productC.species_name = "C";
  productC.coefficient = 1.0;

  arrhenius.reactants = { reactantA, reactantB };
  arrhenius.products = { productC };
  v0_mechanism.reactions.arrhenius = { arrhenius };

  // Convert to V1
  auto v1_mechanism = musica::ConvertV0MechanismToV1(v0_mechanism);

  // Verify conversion
  EXPECT_EQ(v1_mechanism.name, "Test Arrhenius");
  EXPECT_EQ(v1_mechanism.version.major, 1);
  EXPECT_EQ(v1_mechanism.species.size(), 3);
  EXPECT_EQ(v1_mechanism.phases.size(), 2);
  EXPECT_EQ(v1_mechanism.reactions.arrhenius.size(), 1);
  EXPECT_EQ(v1_mechanism.reactions.arrhenius[0].gas_phase, "gas");
  EXPECT_EQ(v1_mechanism.reactions.arrhenius[0].reactants.size(), 2);
  EXPECT_EQ(v1_mechanism.reactions.arrhenius[0].products.size(), 1);

  // Check unit conversion (moles m-3 to molec cm-3)
  // For bimolecular reaction (2 reactants), A should be multiplied by MolesM3ToMoleculesCm3^(2-1) = MolesM3ToMoleculesCm3
  double expected_A = 1.0e-11 * MolesM3ToMoleculesCm3;
  EXPECT_NEAR(v1_mechanism.reactions.arrhenius[0].A, expected_A, expected_A * 1e-13);
}

TEST(Parser, ConvertBranchedV0ToV1)
{
  // Create a V0 mechanism with a single Branched reaction
  mechanism_configuration::v0::types::Mechanism v0_mechanism;
  v0_mechanism.name = "Test Branched";
  v0_mechanism.version.major = 0;
  v0_mechanism.version.minor = 0;
  v0_mechanism.version.patch = 0;

  // Add species
  mechanism_configuration::v0::types::Species speciesA, speciesB, speciesC, speciesD;
  speciesA.name = "A";
  speciesB.name = "B";
  speciesC.name = "C";
  speciesD.name = "D";
  v0_mechanism.species = { speciesA, speciesB, speciesC, speciesD };

  // Add Branched reaction: A -> B (alkoxy) or C (nitrate)
  mechanism_configuration::v0::types::Branched branched;
  branched.X = 1.0e-12;
  branched.Y = 0.5;
  branched.a0 = 0.3;
  branched.n = 1.0;

  mechanism_configuration::v0::types::ReactionComponent reactantA, reactantB, alkoxyB, nitrateC;
  reactantA.species_name = "A";
  reactantA.coefficient = 1.0;
  reactantB.species_name = "B";
  reactantB.coefficient = 2.0;
  alkoxyB.species_name = "B";
  alkoxyB.coefficient = 1.0;
  nitrateC.species_name = "C";
  nitrateC.coefficient = 1.0;

  branched.reactants = { reactantA, reactantB };
  branched.alkoxy_products = { alkoxyB };
  branched.nitrate_products = { nitrateC };
  v0_mechanism.reactions.branched = { branched };

  // Convert to V1
  auto v1_mechanism = musica::ConvertV0MechanismToV1(v0_mechanism);

  // Verify conversion
  EXPECT_EQ(v1_mechanism.name, "Test Branched");
  EXPECT_EQ(v1_mechanism.species.size(), 4);
  EXPECT_EQ(v1_mechanism.reactions.branched.size(), 1);
  EXPECT_EQ(v1_mechanism.reactions.branched[0].gas_phase, "gas");
  EXPECT_EQ(v1_mechanism.reactions.branched[0].reactants.size(), 2);
  EXPECT_EQ(v1_mechanism.reactions.branched[0].alkoxy_products.size(), 1);
  EXPECT_EQ(v1_mechanism.reactions.branched[0].nitrate_products.size(), 1);

  // Check unit conversion for unimolecular reaction (1 reactant)
  // X should be multiplied by MolesM3ToMoleculesCm3^(3-1) = 2
  double expected_X = 1.0e-12 * MolesM3ToMoleculesCm3 * MolesM3ToMoleculesCm3;
  EXPECT_NEAR(v1_mechanism.reactions.branched[0].X, expected_X, expected_X * 1e-13);
}

TEST(Parser, ConvertSurfaceV0ToV1)
{
  // Create a V0 mechanism with a single Surface reaction
  mechanism_configuration::v0::types::Mechanism v0_mechanism;
  v0_mechanism.name = "Test Surface";
  v0_mechanism.version.major = 0;
  v0_mechanism.version.minor = 0;
  v0_mechanism.version.patch = 0;

  // Add species
  mechanism_configuration::v0::types::Species speciesA, speciesB;
  speciesA.name = "A";
  speciesB.name = "B";
  v0_mechanism.species = { speciesA, speciesB };

  // Add Surface reaction: A(g) -> B(g)
  mechanism_configuration::v0::types::Surface surface;
  surface.name = "test_surface";
  surface.reaction_probability = 0.1;

  mechanism_configuration::v0::types::ReactionComponent gasSpeciesA, gasProductB;
  gasSpeciesA.species_name = "A";
  gasSpeciesA.coefficient = 1.0;
  gasProductB.species_name = "B";
  gasProductB.coefficient = 1.0;

  surface.gas_phase_species = gasSpeciesA;
  surface.gas_phase_products = { gasProductB };
  v0_mechanism.reactions.surface = { surface };

  // Convert to V1
  auto v1_mechanism = musica::ConvertV0MechanismToV1(v0_mechanism);

  // Verify conversion
  EXPECT_EQ(v1_mechanism.name, "Test Surface");
  EXPECT_EQ(v1_mechanism.species.size(), 2);
  EXPECT_EQ(v1_mechanism.reactions.surface.size(), 1);
  EXPECT_EQ(v1_mechanism.reactions.surface[0].name, "test_surface");
  EXPECT_EQ(v1_mechanism.reactions.surface[0].gas_phase, "gas");
  EXPECT_EQ(v1_mechanism.reactions.surface[0].condensed_phase, "condensed");
  EXPECT_NEAR(v1_mechanism.reactions.surface[0].reaction_probability, 0.1, 1e-10);
  EXPECT_EQ(v1_mechanism.reactions.surface[0].gas_phase_species.species_name, "A");
  EXPECT_EQ(v1_mechanism.reactions.surface[0].gas_phase_products.size(), 1);
}

TEST(Parser, ConvertTroeV0ToV1)
{
  // Create a V0 mechanism with a single Troe reaction
  mechanism_configuration::v0::types::Mechanism v0_mechanism;
  v0_mechanism.name = "Test Troe";
  v0_mechanism.version.major = 0;
  v0_mechanism.version.minor = 0;
  v0_mechanism.version.patch = 0;

  // Add species
  mechanism_configuration::v0::types::Species speciesA, speciesB, speciesC;
  speciesA.name = "A";
  speciesB.name = "B";
  speciesC.name = "C";
  v0_mechanism.species = { speciesA, speciesB, speciesC };

  // Add Troe reaction: A + B -> C
  mechanism_configuration::v0::types::Troe troe;
  troe.k0_A = 1.0e-30;
  troe.k0_B = -2.0;
  troe.k0_C = 0.0;
  troe.kinf_A = 1.0e-10;
  troe.kinf_B = 0.0;
  troe.kinf_C = 0.0;
  troe.Fc = 0.6;
  troe.N = 1.0;

  mechanism_configuration::v0::types::ReactionComponent reactantA, reactantB, productC;
  reactantA.species_name = "A";
  reactantA.coefficient = 1.0;
  reactantB.species_name = "B";
  reactantB.coefficient = 1.0;
  productC.species_name = "C";
  productC.coefficient = 1.0;

  troe.reactants = { reactantA, reactantB };
  troe.products = { productC };
  v0_mechanism.reactions.troe = { troe };

  // Convert to V1
  auto v1_mechanism = musica::ConvertV0MechanismToV1(v0_mechanism);

  // Verify conversion
  EXPECT_EQ(v1_mechanism.name, "Test Troe");
  EXPECT_EQ(v1_mechanism.species.size(), 3);
  EXPECT_EQ(v1_mechanism.reactions.troe.size(), 1);
  EXPECT_EQ(v1_mechanism.reactions.troe[0].gas_phase, "gas");
  EXPECT_EQ(v1_mechanism.reactions.troe[0].reactants.size(), 2);
  EXPECT_EQ(v1_mechanism.reactions.troe[0].products.size(), 1);

  // Check unit conversion for bimolecular reaction
  // k0_A should be multiplied by MolesM3ToMoleculesCm3^(total_moles) = MolesM3ToMoleculesCm3^2
  // kinf_A should be multiplied by MolesM3ToMoleculesCm3^(total_moles-1) = MolesM3ToMoleculesCm3^1
  double expected_k0_A = 1.0e-30 * MolesM3ToMoleculesCm3 * MolesM3ToMoleculesCm3;
  EXPECT_NEAR(v1_mechanism.reactions.troe[0].k0_A, expected_k0_A, expected_k0_A * 1e-13);
  double expected_kinf_A = 1.0e-10 * MolesM3ToMoleculesCm3;
  EXPECT_NEAR(v1_mechanism.reactions.troe[0].kinf_A, expected_kinf_A, expected_kinf_A * 1e-13);
}

TEST(Parser, ConvertTernaryChemicalActivationV0ToV1)
{
  // Create a V0 mechanism with a single TernaryChemicalActivation reaction
  mechanism_configuration::v0::types::Mechanism v0_mechanism;
  v0_mechanism.name = "Test TernaryChemicalActivation";
  v0_mechanism.version.major = 0;
  v0_mechanism.version.minor = 0;
  v0_mechanism.version.patch = 0;

  // Add species
  mechanism_configuration::v0::types::Species speciesA, speciesB, speciesC;
  speciesA.name = "A";
  speciesB.name = "B";
  speciesC.name = "C";
  v0_mechanism.species = { speciesA, speciesB, speciesC };

  // Add TernaryChemicalActivation reaction: A + B -> C
  mechanism_configuration::v0::types::TernaryChemicalActivation ternary;
  ternary.k0_A = 2.0e-31;
  ternary.k0_B = -1.5;
  ternary.k0_C = 0.0;
  ternary.kinf_A = 1.5e-11;
  ternary.kinf_B = 0.0;
  ternary.kinf_C = 0.0;
  ternary.Fc = 0.8;
  ternary.N = 1.0;

  mechanism_configuration::v0::types::ReactionComponent reactantA, reactantB, productC;
  reactantA.species_name = "A";
  reactantA.coefficient = 1.0;
  reactantB.species_name = "B";
  reactantB.coefficient = 1.0;
  productC.species_name = "C";
  productC.coefficient = 1.0;

  ternary.reactants = { reactantA, reactantB };
  ternary.products = { productC };
  v0_mechanism.reactions.ternary_chemical_activation = { ternary };

  // Convert to V1
  auto v1_mechanism = musica::ConvertV0MechanismToV1(v0_mechanism);

  // Verify conversion
  EXPECT_EQ(v1_mechanism.name, "Test TernaryChemicalActivation");
  EXPECT_EQ(v1_mechanism.species.size(), 3);
  EXPECT_EQ(v1_mechanism.reactions.ternary_chemical_activation.size(), 1);
  EXPECT_EQ(v1_mechanism.reactions.ternary_chemical_activation[0].gas_phase, "gas");
  EXPECT_EQ(v1_mechanism.reactions.ternary_chemical_activation[0].reactants.size(), 2);
  EXPECT_EQ(v1_mechanism.reactions.ternary_chemical_activation[0].products.size(), 1);

  // Check unit conversion for bimolecular reaction
  double expected_k0_A = 2.0e-31 * MolesM3ToMoleculesCm3;
  EXPECT_NEAR(v1_mechanism.reactions.ternary_chemical_activation[0].k0_A, expected_k0_A, expected_k0_A * 1e-13);
  double expected_kinf_A = 1.5e-11 * MolesM3ToMoleculesCm3;
  EXPECT_NEAR(v1_mechanism.reactions.ternary_chemical_activation[0].kinf_A, expected_kinf_A, expected_kinf_A * 1e-13);
}

TEST(Parser, ConvertTunnelingV0ToV1)
{
  // Create a V0 mechanism with a single Tunneling reaction
  mechanism_configuration::v0::types::Mechanism v0_mechanism;
  v0_mechanism.name = "Test Tunneling";
  v0_mechanism.version.major = 0;
  v0_mechanism.version.minor = 0;
  v0_mechanism.version.patch = 0;

  // Add species
  mechanism_configuration::v0::types::Species speciesA, speciesB, speciesC;
  speciesA.name = "A";
  speciesB.name = "B";
  speciesC.name = "C";
  v0_mechanism.species = { speciesA, speciesB, speciesC };

  // Add Tunneling reaction: A + B -> C
  mechanism_configuration::v0::types::Tunneling tunneling;
  tunneling.A = 1.0e-12;
  tunneling.B = 0.0;
  tunneling.C = 150.0;

  mechanism_configuration::v0::types::ReactionComponent reactantA, reactantB, productC;
  reactantA.species_name = "A";
  reactantA.coefficient = 1.0;
  reactantB.species_name = "B";
  reactantB.coefficient = 1.0;
  productC.species_name = "C";
  productC.coefficient = 1.0;

  tunneling.reactants = { reactantA, reactantB };
  tunneling.products = { productC };
  v0_mechanism.reactions.tunneling = { tunneling };

  // Convert to V1
  auto v1_mechanism = musica::ConvertV0MechanismToV1(v0_mechanism);

  // Verify conversion
  EXPECT_EQ(v1_mechanism.name, "Test Tunneling");
  EXPECT_EQ(v1_mechanism.species.size(), 3);
  EXPECT_EQ(v1_mechanism.reactions.tunneling.size(), 1);
  EXPECT_EQ(v1_mechanism.reactions.tunneling[0].gas_phase, "gas");
  EXPECT_EQ(v1_mechanism.reactions.tunneling[0].reactants.size(), 2);
  EXPECT_EQ(v1_mechanism.reactions.tunneling[0].products.size(), 1);

  // Check unit conversion for bimolecular reaction
  double expected_A = 1.0e-12 * MolesM3ToMoleculesCm3;
  EXPECT_NEAR(v1_mechanism.reactions.tunneling[0].A, expected_A, expected_A * 1e-13);
}

TEST(Parser, ConvertUserDefinedV0ToV1)
{
  // Create a V0 mechanism with a single UserDefined reaction
  mechanism_configuration::v0::types::Mechanism v0_mechanism;
  v0_mechanism.name = "Test UserDefined";
  v0_mechanism.version.major = 0;
  v0_mechanism.version.minor = 0;
  v0_mechanism.version.patch = 0;

  // Add species
  mechanism_configuration::v0::types::Species speciesA, speciesB;
  speciesA.name = "A";
  speciesB.name = "B";
  v0_mechanism.species = { speciesA, speciesB };

  // Add UserDefined reaction: A -> B
  mechanism_configuration::v0::types::UserDefined userDefined;
  userDefined.name = "test_user_defined";
  userDefined.scaling_factor = 2.0;

  mechanism_configuration::v0::types::ReactionComponent reactantA, productB;
  reactantA.species_name = "A";
  reactantA.coefficient = 1.0;
  productB.species_name = "B";
  productB.coefficient = 1.0;

  userDefined.reactants = { reactantA };
  userDefined.products = { productB };
  v0_mechanism.reactions.user_defined = { userDefined };

  // Convert to V1
  auto v1_mechanism = musica::ConvertV0MechanismToV1(v0_mechanism);

  // Verify conversion
  EXPECT_EQ(v1_mechanism.name, "Test UserDefined");
  EXPECT_EQ(v1_mechanism.species.size(), 2);
  EXPECT_EQ(v1_mechanism.reactions.user_defined.size(), 1);
  EXPECT_EQ(v1_mechanism.reactions.user_defined[0].name, "test_user_defined");
  EXPECT_EQ(v1_mechanism.reactions.user_defined[0].gas_phase, "gas");
  EXPECT_NEAR(v1_mechanism.reactions.user_defined[0].scaling_factor, 2.0, 1e-10);
  EXPECT_EQ(v1_mechanism.reactions.user_defined[0].reactants.size(), 1);
  EXPECT_EQ(v1_mechanism.reactions.user_defined[0].products.size(), 1);
}