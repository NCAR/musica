// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// End-to-end: an emissions config is parsed by MechanismConfiguration,
// translated by musica::ConvertEmissions into a miem::Source, and that
// Source is handed to MIEM's own EmissionsBuilder, which opens the real
// committed fixture (test/data/x1.163842_2024_nox_subset.nc) and produces
// real surface flux. Same shape as end_to_end.cpp's black-carbon test, but
// against a second real species fixture (fewer sectors, a different year)
// to check the translation layer isn't accidentally tuned to one file.
//
// Also exercises a one-to-many species mapping (the pattern from the
// sample configs/v1/emissions/config.yaml): the single nox_anth_sum
// inventory species is split across two mechanism species, NO (0.9) and
// NO2 (0.1), rather than passed through 1:1.

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
  constexpr int kNCells = 4097;                    // test/data/x1.163842_2024_nox_subset.nc
  constexpr double kEpoch20240701 = 1719792000.0;  // 2024-07-01 00:00:00 UTC

  std::string EmissionsConfigYaml()
  {
    std::ostringstream yaml;
    yaml << "version: 1.0.0\n"
            "species: []\n"
            "phases: []\n"
            "reactions: []\n"
            "emissions:\n"
            "  inventories:\n"
            "    - name: nox subset\n"
            "      directory: \"\"\n"
            "      file pattern: \""
         << MIEM_REAL_NOX_FIXTURE_PATH
         << "\"\n"
            "      convention: uptempo\n"
            "  species maps:\n"
            "    - name: nox map\n"
            "      mappings:\n"
            "        - inventory species: nox_anth_sum\n"
            "          mechanism species: NO\n"
            "          scaling factor: 0.9\n"
            "        - inventory species: nox_anth_sum\n"
            "          mechanism species: NO2\n"
            "          scaling factor: 0.1\n"
            "  regridding:\n"
            "    type: none\n"
            "  sources:\n"
            "    - name: nox source\n"
            "      mode: offline\n"
            "      type: anthropogenic\n"
            "      inventory: nox subset\n"
            "      species map: nox map\n"
            "      temporal interpolation: linear\n"
            "      vertical injection: surface\n"
            "      category: 0\n"
            "      hierarchy: 1\n"
            "      scaling factor: 1.0\n"
            "      sector: anthropogenic\n";
    return yaml.str();
  }
}  // namespace

TEST(MiemEndToEndNox, MechanismConfigThroughMusicaToRealMiemFlux)
{
  musica::Emissions parsed = musica::ReadEmissionsConfigurationFromString(EmissionsConfigYaml());
  ASSERT_EQ(parsed.sources.size(), 1);

  miem::Emissions emissions =
      miem::EmissionsBuilder().SetGridDimensions(kNCells, /*n_vert_levels=*/1).AddSource(parsed.sources[0]).Build();

  ASSERT_EQ(emissions.NumSpecies(), 2);
  const auto& names = emissions.SpeciesNames();
  ASSERT_NE(std::find(names.begin(), names.end(), "NO"), names.end());
  ASSERT_NE(std::find(names.begin(), names.end(), "NO2"), names.end());

  const miem::EmissionsState state = emissions.Run(kEpoch20240701, /*dt=*/3600.0);

  // nox_anth_sum is split 0.9 (NO) / 0.1 (NO2) from the same underlying
  // inventory flux, so NO should be exactly 9x NO2 in every cell.
  bool any_positive = false;
  double sum_no = 0.0;
  double sum_no2 = 0.0;
  for (int ic = 0; ic < kNCells; ++ic)
  {
    const double flux_no = static_cast<double>(state.surface_flux_(ic, "NO"));
    const double flux_no2 = static_cast<double>(state.surface_flux_(ic, "NO2"));
    EXPECT_FALSE(std::isnan(flux_no));
    EXPECT_FALSE(std::isnan(flux_no2));
    EXPECT_GE(flux_no, 0.0);
    EXPECT_GE(flux_no2, 0.0);
    EXPECT_NEAR(flux_no, 9.0 * flux_no2, 1e-12);
    any_positive = any_positive || (flux_no > 0.0);
    sum_no += flux_no;
    sum_no2 += flux_no2;
  }
  EXPECT_TRUE(any_positive);
  EXPECT_NEAR(sum_no, 9.0 * sum_no2, sum_no * 1e-6);
}
