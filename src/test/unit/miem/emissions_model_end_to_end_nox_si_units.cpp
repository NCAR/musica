// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// End-to-end: same shape as emissions_model_end_to_end_nox.cpp, but against
// the real UPTEMPO-reprocessed fixture that ships nox_anth_sum in
// molecules m-2 s-1 (test/data/x1.163842_2024_nox_SI_units_subset.nc)
// instead of the legacy kg m-2 s-1 fixture. The inventory declares a
// molecular weight for nox_anth_sum (30 g/mol, the exact value named in the
// real file's own `comment` attribute), so MIEM's UptempoReader converts to
// kg m-2 s-1 at read time before the species map ever runs -- proving the
// whole chain (schema field -> musica::ConvertSource glue -> reader
// conversion) end to end, not just the reader in isolation.

#include <musica/configuration/read_mechanism.hpp>
#include <musica/miem/emissions.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>

namespace
{
  constexpr int kNCells = 4097;                    // test/data/x1.163842_2024_nox_SI_units_subset.nc
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
            "    - name: nox si units subset\n"
            "      directory: \"\"\n"
            "      file pattern: \""
         << MIEM_REAL_NOX_SI_FIXTURE_PATH
         << "\"\n"
            "      convention: uptempo\n"
            "      molecular weights:\n"
            "        nox_anth_sum: 0.030\n"
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
            "      inventory: nox si units subset\n"
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

TEST(EmissionsModelEndToEndNoxSiUnits, MolecularFluxConvertedThenSplitThroughRealMiem)
{
  musica::EmissionsModel model = musica::EmissionsModel::FromMechanism(
      musica::ReadMechanismFromString(EmissionsConfigYaml()), kNCells, /*n_vert_levels=*/1);

  ASSERT_EQ(model.NumSpecies(), 2);
  const auto& names = model.SpeciesNames();
  ASSERT_NE(std::find(names.begin(), names.end(), "NO"), names.end());
  ASSERT_NE(std::find(names.begin(), names.end(), "NO2"), names.end());

  model.Run(kEpoch20240701, /*dt=*/3600.0);

  // A loose upper bound that only a real kg m-2 s-1 mass flux can satisfy:
  // the raw molecules m-2 s-1 values in this fixture are ~1e12-1e13, so a
  // missed conversion would blow through this by ~20 orders of magnitude.
  // nox_anth_sum is split 0.9 (NO) / 0.1 (NO2) from the same underlying
  // (converted) inventory flux, so NO should be exactly 9x NO2 per cell.
  constexpr double kMaxPlausibleFlux = 1e-6;  // kg m-2 s-1
  bool any_positive = false;
  double sum_no = 0.0;
  double sum_no2 = 0.0;
  for (int ic = 0; ic < kNCells; ++ic)
  {
    const double flux_no = model.SurfaceFlux(ic, "NO");
    const double flux_no2 = model.SurfaceFlux(ic, "NO2");
    EXPECT_FALSE(std::isnan(flux_no));
    EXPECT_FALSE(std::isnan(flux_no2));
    EXPECT_GE(flux_no, 0.0);
    EXPECT_GE(flux_no2, 0.0);
    EXPECT_LT(flux_no, kMaxPlausibleFlux);
    EXPECT_LT(flux_no2, kMaxPlausibleFlux);
    EXPECT_NEAR(flux_no, 9.0 * flux_no2, 1e-15);
    any_positive = any_positive || (flux_no > 0.0);
    sum_no += flux_no;
    sum_no2 += flux_no2;
  }
  EXPECT_TRUE(any_positive);
  EXPECT_NEAR(sum_no, 9.0 * sum_no2, sum_no * 1e-6);
}
