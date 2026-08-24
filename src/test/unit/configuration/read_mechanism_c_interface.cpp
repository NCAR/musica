// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Tests for the C-visible Mechanism* handle (ReadMechanismC / ReadMechanismFromStringC /
// DeleteMechanism), added so CreateEmissions (musica/miem/emissions_c_interface.hpp) can
// take a Mechanism* from C/Fortran callers.

#include <musica/configuration/read_mechanism_c_interface.hpp>

#include <gtest/gtest.h>

#include <fstream>
#include <string>

using namespace musica;

TEST(ReadMechanismCInterface, ReadsFromFile)
{
  Error error;
  NoError(&error);
  Mechanism* mechanism = ReadMechanismC("configs/v1/chapman/config.json", &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(mechanism, nullptr);
  EXPECT_EQ(mechanism->name, "Chapman");
  EXPECT_EQ(mechanism->species.size(), 6);

  DeleteMechanism(mechanism, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteError(&error);
}

TEST(ReadMechanismCInterface, ReadsFromString)
{
  std::ifstream file("configs/v1/chapman/config.json");
  std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

  Error error;
  NoError(&error);
  Mechanism* mechanism = ReadMechanismFromStringC(contents.c_str(), &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(mechanism, nullptr);
  EXPECT_EQ(mechanism->name, "Chapman");

  DeleteMechanism(mechanism, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteError(&error);
}

TEST(ReadMechanismCInterface, BadConfigurationFilePath)
{
  Error error;
  NoError(&error);
  Mechanism* mechanism = ReadMechanismC("bad config path", &error);
  ASSERT_EQ(mechanism, nullptr);
  ASSERT_EQ(error.code_, MUSICA_PARSE_ERROR_CODE_INVALID_CONFIG_FILE);
  ASSERT_STREQ(error.category_.value_, MUSICA_PARSE_ERROR_CATEGORY);
  DeleteError(&error);
}

TEST(ReadMechanismCInterface, BadConfigurationString)
{
  Error error;
  NoError(&error);
  Mechanism* mechanism = ReadMechanismFromStringC("", &error);
  ASSERT_EQ(mechanism, nullptr);
  ASSERT_FALSE(IsSuccess(error));
  DeleteError(&error);
}
