// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Integration test: parses configs/v1/cam_cloud_chemistry/config.json

#include <musica/configuration/read_mechanism.hpp>
#include <musica/miam/miam_builder.hpp>
#include <musica/micm/micm.hpp>
#include <musica/micm/state.hpp>
#include <musica/utils/error.hpp>

#include <mechanism_configuration/mechanism.hpp>

#include <gtest/gtest.h>

namespace
{
  constexpr const char* configPath = "configs/v1/cam_cloud_chemistry/config.json";
}

TEST(CamCloudChemistryConfig, ParsesExpectedStructure)
{
  mechanism_configuration::Mechanism mechanism = musica::ReadMechanism(configPath);

  EXPECT_EQ(mechanism.name, "CAM Cloud Chemistry");
  EXPECT_EQ(mechanism.species.size(), 10);
  EXPECT_EQ(mechanism.phases.size(), 2);

  ASSERT_TRUE(mechanism.aerosol.has_value());
  EXPECT_EQ(mechanism.aerosol->representations.size(), 1);
  EXPECT_EQ(mechanism.aerosol->processes.size(), 4);
  EXPECT_EQ(mechanism.aerosol->constraints.size(), 10);
}

TEST(CamCloudChemistryConfig, BuildsMiamSolver)
{
  mechanism_configuration::Mechanism mechanism = musica::ReadMechanism(configPath);

  musica::Error error;
  musica::MICM* micm = musica::CreateMicmWithMiam(mechanism, musica::MICMSolver::RosenbrockDAE4StandardOrder, &error);

  ASSERT_TRUE(musica::IsSuccess(error)) << "Error: " << (error.message_.value_ ? error.message_.value_ : "null");
  ASSERT_NE(micm, nullptr);

  musica::State state(*micm, 1);
  state.SetConditions({ { .temperature_ = 280.0, .pressure_ = 70000.0 } });

  delete micm;
  musica::DeleteError(&error);
}
