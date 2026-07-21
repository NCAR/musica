// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// End-to-end: an emissions config is parsed by MechanismConfiguration and
// built into a musica::EmissionsModel via FromMechanism, which internally
// translates it (ConvertEmissions) and hands it to MIEM's EmissionsBuilder.
// The model opens the real committed fixture
// (test/data/CAMS-GLOB-ANT_2012_MPAS_bc_subset.nc) and produces real
// surface flux. Mirrors the assertions in miem's own
// test/integration/test_bc_pipeline.cpp, but drives the pipeline through
// MUSICA's musica::EmissionsModel interface instead of hand-building the
// miem::Source and miem::EmissionsBuilder.

#include <musica/configuration/read_mechanism.hpp>
#include <musica/miem/emissions.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>

namespace
{
  constexpr int kNCells = 4097;                    // test/data/CAMS-GLOB-ANT_2012_MPAS_bc_subset.nc
  constexpr double kEpoch20120701 = 1341100800.0;  // 2012-07-01 00:00:00 UTC

  std::string EmissionsConfigYaml()
  {
    std::ostringstream yaml;
    yaml << "version: 1.0.0\n"
            "species: []\n"
            "phases: []\n"
            "reactions: []\n"
            "emissions:\n"
            "  inventories:\n"
            "    - name: cams bc subset\n"
            "      directory: \"\"\n"
            "      file pattern: \""
         << MIEM_REAL_FIXTURE_PATH
         << "\"\n"
            "      convention: uptempo\n"
            "  species maps:\n"
            "    - name: bc map\n"
            "      mappings:\n"
            "        - inventory species: bc_anth_sum\n"
            "          mechanism species: BC\n"
            "          scaling factor: 1.0\n"
            "  regridding:\n"
            "    type: none\n"
            "  sources:\n"
            "    - name: cams bc source\n"
            "      mode: offline\n"
            "      type: anthropogenic\n"
            "      inventory: cams bc subset\n"
            "      species map: bc map\n"
            "      temporal interpolation: linear\n"
            "      vertical injection: surface\n"
            "      category: 0\n"
            "      hierarchy: 1\n"
            "      scaling factor: 1.0\n"
            "      sector: anthropogenic\n";
    return yaml.str();
  }
}  // namespace

TEST(EmissionsModelEndToEnd, MechanismConfigThroughMusicaToRealMiemFlux)
{
  musica::EmissionsModel model = musica::EmissionsModel::FromMechanism(
      musica::ReadMechanismFromString(EmissionsConfigYaml()), kNCells, /*n_vert_levels=*/1);

  ASSERT_EQ(model.NumSpecies(), 1);
  const auto& names = model.SpeciesNames();
  ASSERT_NE(std::find(names.begin(), names.end(), "BC"), names.end());

  model.Run(kEpoch20120701, /*dt=*/3600.0);

  bool any_positive = false;
  for (int ic = 0; ic < kNCells; ++ic)
  {
    const double flux = model.SurfaceFlux(ic, "BC");
    EXPECT_FALSE(std::isnan(flux));
    EXPECT_GE(flux, 0.0);
    any_positive = any_positive || (flux > 0.0);
  }
  EXPECT_TRUE(any_positive);
}
