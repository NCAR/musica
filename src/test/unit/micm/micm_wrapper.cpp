#include <musica/micm/micm.hpp>
#include <musica/micm/parse.hpp>
#include <micm/util/error.hpp>

#include <gtest/gtest.h>

#include <iostream>

TEST(MICMWrapper, CanParseChapman) 
{
  musica::MICM micm;
  musica::Error error;
  musica::Chemistry chemistry = ReadConfiguration("configs/chapman", &error);
  ASSERT_TRUE(IsSuccess(error));
  EXPECT_EQ(chemistry.system.gas_phase_.species_.size(), 5);
  EXPECT_EQ(chemistry.processes.size(), 7);
  EXPECT_EQ(chemistry.system.gas_phase_.species_[0].name_, "M");
  EXPECT_NE(chemistry.system.gas_phase_.species_[0].parameterize_, nullptr);
  EXPECT_EQ(chemistry.system.gas_phase_.species_[1].name_, "O2");
  EXPECT_EQ(chemistry.system.gas_phase_.species_[2].name_, "O");
  EXPECT_EQ(chemistry.system.gas_phase_.species_[3].name_, "O1D");
  EXPECT_EQ(chemistry.system.gas_phase_.species_[4].name_, "O3");
  DeleteError(&error);
}

TEST(MICMWrapper, CanParseCBV) 
{
  musica::MICM micm;
  musica::Error error;
  musica::Chemistry chemistry = ReadConfiguration("configs/carbon_bond_5", &error);
  ASSERT_TRUE(IsSuccess(error));
  EXPECT_EQ(chemistry.system.gas_phase_.species_.size(), 67);
  EXPECT_EQ(chemistry.processes.size(), 200);
  DeleteError(&error);
}

TEST(MICMWrapper, CanParseTS1) 
{
  musica::MICM micm;
  musica::Error error;
  musica::Chemistry chemistry = ReadConfiguration("configs/TS1", &error);
  ASSERT_TRUE(IsSuccess(error));
  EXPECT_EQ(chemistry.system.gas_phase_.species_.size(), 210);
  EXPECT_EQ(chemistry.processes.size(), 547);
  DeleteError(&error);
}

TEST(MICMWrapper, DetectsInvalidConfig) 
{
  musica::MICM micm;
  musica::Error error;
  musica::Chemistry chemistry = ReadConfiguration("configs/invalid", &error);
  ASSERT_FALSE(IsSuccess(error));
  ASSERT_TRUE(IsError(error, MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_CONFIG_PARSE_FAILED));
  DeleteError(&error);
}