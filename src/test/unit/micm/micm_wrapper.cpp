#include <musica/micm/micm.hpp>
#include <musica/micm/parse.hpp>

#include <gtest/gtest.h>

#include <iostream>

TEST(MICMWrapper, CanParseChapmanV0)
{
  musica::MICM micm;
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

TEST(MICMWrapper, CanParseCBVV0)
{
  musica::MICM micm;
  musica::Chemistry chemistry = musica::ReadConfiguration("configs/carbon_bond_5");
  EXPECT_EQ(chemistry.system.gas_phase_.species_.size(), 67);
  EXPECT_EQ(chemistry.processes.size(), 200);
}

TEST(MICMWrapper, CanParseTS1V0)
{
  musica::MICM micm;
  musica::Chemistry chemistry = musica::ReadConfiguration("configs/TS1");
  EXPECT_EQ(chemistry.system.gas_phase_.species_.size(), 210);
  EXPECT_EQ(chemistry.processes.size(), 547);
}

TEST(MICMWrapper, DetectsInvalidConfigV0)
{
  musica::MICM micm;
  EXPECT_ANY_THROW(musica::ReadConfiguration("configs/invalid"));
}

TEST(MICMWrapper, CanParseChapmanV1)
{
  std::vector<std::string> extensions = { ".json", ".yaml" };

  for (const auto& extension : extensions)
  {
    musica::MICM micm;
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

TEST(MICMWrapper, CanParseFullV1)
{
  std::vector<std::string> extensions = { ".json", ".yaml" };

  for (const auto& extension : extensions)
  {
    musica::MICM micm;
    musica::Chemistry chemistry = musica::ReadConfiguration("configs/v1/full_configuration" + extension);
    EXPECT_EQ(chemistry.system.gas_phase_.species_.size(), 4);
    EXPECT_EQ(chemistry.system.gas_phase_.name_, "gas");
    EXPECT_EQ(chemistry.system.phases_.size(), 3);
    EXPECT_EQ(chemistry.processes.size(), 9);
  }
}