#include <musica/micm/micm.hpp>

#include <gtest/gtest.h>

#include <iostream>

TEST(MICMWrapper, CanParse) 
{
  musica::MICM micm;
  musica::Chemistry chemistry = micm.ReadConfiguration("configs/chapman", nullptr);
  EXPECT_EQ(chemistry.system.gas_phase_.species_.size(), 5);
  EXPECT_EQ(chemistry.processes.size(), 7);
  EXPECT_EQ(chemistry.system.gas_phase_.species_[0].name_, "M");
  EXPECT_NE(chemistry.system.gas_phase_.species_[0].parameterize_, nullptr);
  EXPECT_EQ(chemistry.system.gas_phase_.species_[1].name_, "O2");
  EXPECT_EQ(chemistry.system.gas_phase_.species_[2].name_, "O");
  EXPECT_EQ(chemistry.system.gas_phase_.species_[3].name_, "O1D");
  EXPECT_EQ(chemistry.system.gas_phase_.species_[4].name_, "O3");
}