// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Direct tests of the single shared read path (musica::ReadMechanism /
// ReadMechanismFromString). This file intentionally does not depend on
// musica/configuration/parse.hpp or emissions.hpp (and therefore not on
// MICM or MIEM) to confirm parsing works with neither enabled.

#include <musica/configuration/read_mechanism.hpp>
#include <musica/utils/error.hpp>

#include <gtest/gtest.h>

#include <fstream>
#include <string>

TEST(ReadMechanism, BadConfigurationFilePathThrows)
{
  EXPECT_ANY_THROW(musica::ReadMechanism("bad config path"));
}

TEST(ReadMechanism, ReadsFromFile)
{
  mechanism_configuration::Mechanism mechanism = musica::ReadMechanism("configs/v1/chapman/config.json");

  EXPECT_EQ(mechanism.name, "Chapman");
  EXPECT_EQ(mechanism.species.size(), 6);
  EXPECT_EQ(mechanism.reactions.arrhenius.size(), 4);
}

TEST(ReadMechanism, ReadsFromString)
{
  std::ifstream file("configs/v1/chapman/config.json");
  std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

  mechanism_configuration::Mechanism mechanism = musica::ReadMechanismFromString(contents);

  EXPECT_EQ(mechanism.name, "Chapman");
  EXPECT_EQ(mechanism.species.size(), 6);
  EXPECT_EQ(mechanism.reactions.arrhenius.size(), 4);
}

TEST(ReadMechanism, ReadFromFileAndStringMatch)
{
  mechanism_configuration::Mechanism from_file = musica::ReadMechanism("configs/v1/chapman/config.json");

  std::ifstream file("configs/v1/chapman/config.json");
  std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  mechanism_configuration::Mechanism from_string = musica::ReadMechanismFromString(contents);

  EXPECT_EQ(from_file.name, from_string.name);
  EXPECT_EQ(from_file.species.size(), from_string.species.size());
  EXPECT_EQ(from_file.reactions.arrhenius.size(), from_string.reactions.arrhenius.size());
}

TEST(ReadMechanism, BadConfigurationStringThrows)
{
  EXPECT_ANY_THROW(musica::ReadMechanismFromString(""));
}
