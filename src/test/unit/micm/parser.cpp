#include <musica/micm/parse.hpp>

#include <gtest/gtest.h>
#include <mechanism_configuration/parser.hpp>

TEST(Parser, BadConfigurationFilePath)
{
  mechanism_configuration::UniversalParser parser;
  auto parsed = parser.Parse("bad config path");
  EXPECT_FALSE(parsed);
}

TEST(Parser, Version0Configuration)
{
  mechanism_configuration::UniversalParser parser;
  auto parsed = parser.Parse("configs/chapman");
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
}

TEST(Parser, CanParseChapmanV0)
{
  musica::Chemistry chemistry = musica::ReadConfiguration("configs/chapman");
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
  musica::Chemistry chemistry = musica::ReadConfiguration("configs/carbon_bond_5");
  EXPECT_EQ(chemistry.system.gas_phase_.species_.size(), 67);
  EXPECT_EQ(chemistry.processes.size(), 200);
}

TEST(Parser, CanParseTS1V0)
{
  musica::Chemistry chemistry = musica::ReadConfiguration("configs/TS1");
  EXPECT_EQ(chemistry.system.gas_phase_.species_.size(), 210);
  EXPECT_EQ(chemistry.processes.size(), 547);
}

TEST(Parser, DetectsInvalidConfigV0)
{
  EXPECT_ANY_THROW(musica::ReadConfiguration("configs/invalid"));
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
    musica::Chemistry chemistry = musica::ReadConfiguration("configs/v1/full_configuration" + extension);
    EXPECT_EQ(chemistry.system.gas_phase_.species_.size(), 4);
    EXPECT_EQ(chemistry.system.gas_phase_.name_, "gas");
    EXPECT_EQ(chemistry.system.phases_.size(), 3);
    EXPECT_EQ(chemistry.processes.size(), 9);
  }
}