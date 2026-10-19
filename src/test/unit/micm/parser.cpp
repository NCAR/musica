#include <musica/configuration/parse.hpp>
#include <musica/configuration/read_mechanism.hpp>

#include <mechanism_configuration/parse.hpp>

#include <gtest/gtest.h>

static constexpr double avogadro = 6.02214076e23;  // # mol^{-1}
static constexpr double MolesM3ToMoleculesCm3 = 1.0e-6 * avogadro;

TEST(Parser, BadConfigurationFilePath)
{
  auto parsed = mechanism_configuration::Parse("bad config path");
  EXPECT_FALSE(parsed);
}

TEST(Parser, Version0Configuration)
{
  auto parsed = mechanism_configuration::Parse("configs/v0/chapman");
  EXPECT_TRUE(parsed);

  mechanism_configuration::Mechanism v0_mechanism = *parsed;

  EXPECT_EQ(v0_mechanism.name, "Chapman");
  EXPECT_EQ(v0_mechanism.version.major, 0);
  EXPECT_EQ(v0_mechanism.version.minor, 0);
  EXPECT_EQ(v0_mechanism.version.patch, 0);
  EXPECT_EQ(v0_mechanism.reactions.arrhenius.size(), 4);
  EXPECT_EQ(v0_mechanism.reactions.photolysis.size(), 3);
  EXPECT_EQ(v0_mechanism.species.size(), 5);
}

TEST(Parser, Version1Configuration)
{
  auto parsed = mechanism_configuration::Parse("configs/v1/chapman/config.json");
  EXPECT_TRUE(parsed);

  mechanism_configuration::Mechanism v1_mechanism = *parsed;

  EXPECT_EQ(v1_mechanism.name, "Chapman");
  EXPECT_EQ(v1_mechanism.version.major, 1);
  EXPECT_EQ(v1_mechanism.version.minor, 0);
  EXPECT_EQ(v1_mechanism.version.patch, 0);
  EXPECT_EQ(v1_mechanism.reactions.arrhenius.size(), 4);
  EXPECT_EQ(v1_mechanism.reactions.photolysis.size(), 3);
  EXPECT_EQ(v1_mechanism.species.size(), 6);
}

TEST(Parser, CanParseChapmanV0)
{
  musica::Chemistry chemistry = musica::ConvertChemistry(musica::ReadMechanism("configs/v0/chapman"));
  EXPECT_EQ(chemistry.system.gas_phase_.phase_species_.size(), 5);
  EXPECT_EQ(chemistry.processes.size(), 7);
  EXPECT_EQ(chemistry.system.gas_phase_.phase_species_[0].species_.name_, "M");
  EXPECT_TRUE(chemistry.system.gas_phase_.phase_species_[0].species_.IsParameterized());
  EXPECT_EQ(chemistry.system.gas_phase_.phase_species_[1].species_.name_, "O2");
  EXPECT_EQ(chemistry.system.gas_phase_.phase_species_[2].species_.name_, "O");
  EXPECT_EQ(chemistry.system.gas_phase_.phase_species_[3].species_.name_, "O1D");
  EXPECT_EQ(chemistry.system.gas_phase_.phase_species_[4].species_.name_, "O3");
}

TEST(Parser, CanParseCBVV0)
{
  musica::Chemistry const chemistry = musica::ConvertChemistry(musica::ReadMechanism("configs/v0/carbon_bond_5"));
  EXPECT_EQ(chemistry.system.gas_phase_.phase_species_.size(), 67);
  EXPECT_EQ(chemistry.processes.size(), 200);
}

TEST(Parser, CanParseTS1V0)
{
  musica::Chemistry const chemistry = musica::ConvertChemistry(musica::ReadMechanism("configs/v0/TS1"));
  EXPECT_EQ(chemistry.system.gas_phase_.phase_species_.size(), 210);
  EXPECT_EQ(chemistry.processes.size(), 547);
}

TEST(Parser, DetectsInvalidConfigV0)
{
  EXPECT_ANY_THROW(musica::ConvertChemistry(musica::ReadMechanism("configs/v0/invalid")));
}

TEST(Parser, CanParseChapmanV1)
{
  std::vector<std::string> const extensions = { ".json", ".yaml" };

  for (const auto& extension : extensions)
  {
    musica::Chemistry chemistry = musica::ConvertChemistry(musica::ReadMechanism("configs/v1/chapman/config" + extension));
    EXPECT_EQ(chemistry.system.gas_phase_.phase_species_.size(), 6);
    EXPECT_EQ(chemistry.processes.size(), 7);
    EXPECT_EQ(chemistry.system.gas_phase_.phase_species_[0].species_.name_, "M");
    EXPECT_TRUE(chemistry.system.gas_phase_.phase_species_[0].species_.IsParameterized());
    EXPECT_EQ(chemistry.system.gas_phase_.phase_species_[1].species_.name_, "O1D");
    EXPECT_EQ(chemistry.system.gas_phase_.phase_species_[2].species_.name_, "O");
    EXPECT_EQ(chemistry.system.gas_phase_.phase_species_[3].species_.name_, "O2");
    EXPECT_EQ(chemistry.system.gas_phase_.phase_species_[4].species_.name_, "O3");
    EXPECT_EQ(chemistry.system.gas_phase_.phase_species_[5].species_.name_, "N2");

    EXPECT_EQ(chemistry.system.gas_phase_.phase_species_[4].species_.GetProperty<std::string>("__long name"), "ozone");
  }
}

TEST(Parser, CanParseFullV1)
{
  std::vector<std::string> const extensions = { ".json", ".yaml" };

  for (const auto& extension : extensions)
  {
    musica::Chemistry const chemistry =
        musica::ConvertChemistry(musica::ReadMechanism("configs/v1/full_configuration/full_configuration" + extension));

    EXPECT_EQ(chemistry.system.gas_phase_.phase_species_.size(), 6);
    EXPECT_EQ(chemistry.system.gas_phase_.name_, "gas");
    EXPECT_EQ(chemistry.processes.size(), 13);
  }
}

// Unknown species properties must reach micm in the same typed property map
// (string, double, int, or bool) as when the parser stored them as strings, so that
// MICM::GetSpeciesProperty<T> and its C and Fortran wrappers keep working.
TEST(Parser, UnknownSpeciesPropertiesKeepTheirMicmType)
{
  const std::string config = R"(
version: 1.0.0
name: property types
species:
  - name: A
    __plain double: 1.0e-12
    __quoted double: "0.5"
    __plain int: 3
    __quoted int: "7"
    __large int: 3000000000
    __plain bool: true
    __quoted bool: "false"
    __text: ozone
    __quoted text: "ozone"
    __huge: 1e400
    __quoted huge: "1e400"
    __quoted large int: "3000000000"
    __nothing: null
    __nested: { a: [1, 2] }
phases:
  - name: gas
    species: [ A ]
reactions: []
)";
  musica::Chemistry chemistry = musica::ConvertChemistry(musica::ReadMechanismFromString(config));
  const micm::Species& species = chemistry.system.gas_phase_.phase_species_[0].species_;

  EXPECT_DOUBLE_EQ(species.GetProperty<double>("__plain double"), 1.0e-12);
  EXPECT_DOUBLE_EQ(species.GetProperty<double>("__quoted double"), 0.5);
  EXPECT_EQ(species.GetProperty<int>("__plain int"), 3);
  EXPECT_EQ(species.GetProperty<int>("__quoted int"), 7);
  EXPECT_DOUBLE_EQ(species.GetProperty<double>("__large int"), 3000000000.0);
  EXPECT_TRUE(species.GetProperty<bool>("__plain bool"));
  EXPECT_FALSE(species.GetProperty<bool>("__quoted bool"));
  EXPECT_EQ(species.GetProperty<std::string>("__text"), "ozone");
  EXPECT_EQ(species.GetProperty<std::string>("__quoted text"), "ozone");
  EXPECT_EQ(species.GetProperty<std::string>("__huge"), "1e400");
  EXPECT_EQ(species.GetProperty<std::string>("__quoted huge"), "1e400");
  EXPECT_DOUBLE_EQ(species.GetProperty<double>("__quoted large int"), 3000000000.0);
  EXPECT_EQ(species.GetProperty<std::string>("__nothing"), "null");
  EXPECT_EQ(species.GetProperty<std::string>("__nested"), R"({"a":[1,2]})");
}
