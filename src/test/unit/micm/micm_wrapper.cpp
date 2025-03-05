#include <musica/micm/micm.hpp>

#include <gtest/gtest.h>

#include <iostream>

TEST(MICMWrapper, CanParse) 
{
  musica::MICM micm;
  musica::Chemistry chemistry = micm.ReadConfiguration("configs/chapman", nullptr);
  EXPECT_EQ(chemistry.system.gas_phase_.species_.size(), 5);
  EXPECT_EQ(chemistry.processes.size(), 7);
}