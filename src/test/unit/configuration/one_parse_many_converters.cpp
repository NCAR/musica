// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Proves that a single parsed mechanism_configuration::Mechanism can feed
// multiple converters (ConvertChemistry for MICM, ConvertEmissions for
// MIEM) without needing to parse the configuration a second time.

#include <musica/configuration/emissions.hpp>
#include <musica/configuration/parse.hpp>
#include <musica/configuration/read_mechanism.hpp>

#include <gtest/gtest.h>

TEST(OneParseManyConverters, FeedsBothChemistryAndEmissionsConverters)
{
  // configs/v1/chapman/config.json has real species/reactions and no
  // emissions section, so this also proves missing emissions is not an error.
  mechanism_configuration::Mechanism mechanism = musica::ReadMechanism("configs/v1/chapman/config.json");

  musica::Chemistry chemistry = musica::ConvertChemistry(mechanism);
  musica::Emissions emissions = musica::ConvertEmissions(mechanism);

  EXPECT_EQ(chemistry.system.gas_phase_.phase_species_.size(), 6);
  EXPECT_EQ(chemistry.processes.size(), 7);

  EXPECT_TRUE(emissions.sources.empty());
  EXPECT_EQ(emissions.regridding.type_, miem::RegriddingType::None);
}

TEST(OneParseManyConverters, EmissionsConfigFeedsBothConvertersToo)
{
  // configs/v1/emissions/config.yaml has an emissions section but empty
  // species/phases/reactions, so ConvertChemistry should succeed with an
  // empty Chemistry while ConvertEmissions produces real sources, all from
  // the same parsed Mechanism.
  mechanism_configuration::Mechanism mechanism = musica::ReadMechanism("configs/v1/emissions/config.yaml");

  musica::Chemistry chemistry = musica::ConvertChemistry(mechanism);
  musica::Emissions emissions = musica::ConvertEmissions(mechanism);

  EXPECT_EQ(chemistry.system.gas_phase_.phase_species_.size(), 0);
  EXPECT_EQ(emissions.sources.size(), 2);
}
