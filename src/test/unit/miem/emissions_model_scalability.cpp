// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#include <musica/miem/emissions.hpp>

#include "synthetic_nc.hpp"

#include <miem/util/miem_exception.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

namespace
{
  constexpr int kGlobalCells = 6;
  constexpr int kLevels = 3;
  constexpr double kEpochMidpoint = 1800.0;

  miem::Source MakeSource(
      const std::string& name,
      const std::string& path,
      int category,
      const std::string& sector,
      const std::vector<double>& profile)
  {
    miem::Source source;
    source.name_ = name;
    source.file_pattern_ = path;
    source.convention_ = "uptempo";
    source.temporal_interpolation_ = miem::TemporalInterpolation::Linear;
    source.category_ = category;
    source.hierarchy_ = 1;
    source.sector_ = sector;
    source.species_map_.AddMapping("NOx", "NO", 0.9);
    source.species_map_.AddMapping("NOx", "NO2", 0.1);
    if (!profile.empty())
    {
      source.vertical_injection_ = miem::VerticalInjection::Profile;
      source.vertical_profile_ = profile;
    }
    return source;
  }

  std::string MakeInventory(miem_test::TempDir& dir, const std::string& name, double flux)
  {
    const std::string path = dir.File(name);
    const std::vector<double> data(2 * kGlobalCells, flux);
    miem_test::UptempoNcOptions nc_options;
    nc_options.write_grid_metadata = true;
    miem_test::CreateUptempoTestNetCDF(
        path,
        2,
        kGlobalCells,
        { "1970-01-01_00:00:00", "1970-01-01_01:00:00" },
        { "NOx" },
        { data },
        nc_options);
    return path;
  }
}  // namespace

TEST(EmissionsModelScalability, SelectedCellsMetadataLayersAndBoundedDiagnosticsAreExposed)
{
  miem_test::TempDir dir;
  musica::Emissions input;
  input.sources = {
    MakeSource("ground source", MakeInventory(dir, "ground.nc", 2.0e-9), 0, "ground", {}),
    MakeSource("stack source", MakeInventory(dir, "stack.nc", 6.0e-9), 1, "stack", { 0.0, 0.25, 0.75 }),
  };

  musica::EmissionsOptions options;
  options.global_n_cells = kGlobalCells;
  options.n_vert_levels = kLevels;
  options.selected_global_cell_ids = { 6, 2, 3 };
  options.diagnostics.sectors_ = { "ground", "stack" };
  options.diagnostics.categories_ = { 0, 1 };
  options.diagnostics.layered_output_ = true;
  options.diagnostics.max_fields_ = 24;  // 2 species * 4 groups * 3 levels

  musica::EmissionsModel model(input, options);
  EXPECT_EQ(model.NumGlobalCells(), kGlobalCells);
  EXPECT_EQ(model.NumCells(), 3);
  EXPECT_EQ(model.NumVertLevels(), kLevels);
  EXPECT_EQ(model.SelectedGlobalCellIds(), (std::vector<int>{ 6, 2, 3 }));
  EXPECT_EQ(model.DiagnosticSectorNames(), (std::vector<std::string>{ "ground", "stack" }));
  EXPECT_EQ(model.DiagnosticCategoryIds(), (std::vector<int>{ 0, 1 }));
  EXPECT_TRUE(model.LayeredDiagnosticsEnabled());

  model.Run(kEpochMidpoint, 60.0);

  ASSERT_EQ(model.SurfaceFluxData().size(), 2U * 3U);
  ASSERT_EQ(model.LayerFluxData().size(), 2U * kLevels * 3U);
  for (int cell = 0; cell < model.NumCells(); ++cell)
  {
    EXPECT_NEAR(model.SurfaceFlux(cell, "NO"), 7.2e-9, 1.0e-18);
    EXPECT_NEAR(model.SurfaceFlux(cell, "NO2"), 0.8e-9, 1.0e-18);
    EXPECT_NEAR(model.LayerFlux(cell, 0, "NO"), 1.8e-9, 1.0e-18);
    EXPECT_NEAR(model.LayerFlux(cell, 1, "NO"), 1.35e-9, 1.0e-18);
    EXPECT_NEAR(model.LayerFlux(cell, 2, "NO"), 4.05e-9, 1.0e-18);
  }

  const auto& ground = model.SectorFluxData("ground");
  const auto& stack = model.CategoryFluxData(1);
  const auto& stack_layers = model.SectorLayerFluxData("stack");
  const std::size_t species_stride = static_cast<std::size_t>(model.NumCells());
  const std::size_t level_stride = species_stride;
  for (int cell = 0; cell < model.NumCells(); ++cell)
  {
    EXPECT_NEAR(ground[cell], 1.8e-9, 1.0e-18);
    EXPECT_NEAR(stack[cell], 5.4e-9, 1.0e-18);
    EXPECT_DOUBLE_EQ(stack_layers[cell], 0.0);
    EXPECT_NEAR(stack_layers[level_stride + cell], 1.35e-9, 1.0e-18);
    EXPECT_NEAR(stack_layers[2 * level_stride + cell], 4.05e-9, 1.0e-18);
  }
  EXPECT_EQ(model.CategoryLayerFluxData(1), stack_layers);

  ASSERT_TRUE(model.HasInventoryGridMetadata());
  const auto& metadata = model.GridMetadata();
  EXPECT_TRUE(metadata.IsExactGrid());
  EXPECT_EQ(metadata.global_n_cells_, kGlobalCells);
  EXPECT_EQ(metadata.selected_global_cell_ids_, (std::vector<int>{ 6, 2, 3 }));
  EXPECT_EQ(metadata.index_to_cell_id_, (std::vector<std::int64_t>{ 6, 2, 3 }));
  ASSERT_NE(metadata.FindField("areaCell"), nullptr);
  EXPECT_EQ(metadata.FindField("areaCell")->values_, (std::vector<double>{ 1005.0, 1001.0, 1002.0 }));
}

TEST(EmissionsModelScalability, DiagnosticFieldCapIsEnforcedAtConstruction)
{
  miem_test::TempDir dir;
  musica::Emissions input;
  input.sources = { MakeSource("ground source", MakeInventory(dir, "ground.nc", 2.0e-9), 0, "ground", {}) };

  musica::EmissionsOptions options;
  options.global_n_cells = kGlobalCells;
  options.n_vert_levels = kLevels;
  options.selected_global_cell_ids = { 1 };
  options.diagnostics.sectors_ = { "ground" };
  options.diagnostics.layered_output_ = true;
  options.diagnostics.max_fields_ = 5;  // Needs 2 species * 1 group * 3 levels.

  EXPECT_THROW((void)musica::EmissionsModel(input, options), miem::MiemException);
}
