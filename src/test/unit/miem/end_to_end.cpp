// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// End-to-end: an emissions config is parsed by MechanismConfiguration,
// translated by musica::ConvertEmissions into a miem::Source, and that
// Source is handed to MIEM's own EmissionsBuilder, which opens the real
// committed fixture (test/data/CAMS-GLOB-ANT_2012_MPAS_bc_subset.nc) and
// produces real surface flux. Mirrors the assertions in miem's own
// test/integration/test_bc_pipeline.cpp, but drives the pipeline through
// MUSICA's parse/translate layer instead of hand-building the miem::Source.

#include <musica/configuration/emissions.hpp>

#include <gtest/gtest.h>
#include <miem/emissions.hpp>
#include <miem/emissions_builder.hpp>
#include <miem/emissions_state.hpp>

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

TEST(MiemEndToEnd, MechanismConfigThroughMusicaToRealMiemFlux)
{
  musica::Emissions parsed = musica::ReadEmissionsConfigurationFromString(EmissionsConfigYaml());
  ASSERT_EQ(parsed.sources.size(), 1);

  miem::Emissions emissions =
      miem::EmissionsBuilder().SetGridDimensions(kNCells, /*n_vert_levels=*/1).AddSource(parsed.sources[0]).Build();

  ASSERT_EQ(emissions.NumSpecies(), 1);
  const auto& names = emissions.SpeciesNames();
  ASSERT_NE(std::find(names.begin(), names.end(), "BC"), names.end());

  const miem::EmissionsState state = emissions.Run(kEpoch20120701, /*dt=*/3600.0);

  bool any_positive = false;
  for (int ic = 0; ic < kNCells; ++ic)
  {
    const double flux = static_cast<double>(state.surface_flux_(ic, "BC"));
    EXPECT_FALSE(std::isnan(flux));
    EXPECT_GE(flux, 0.0);
    any_positive = any_positive || (flux > 0.0);
  }
  EXPECT_TRUE(any_positive);
}
