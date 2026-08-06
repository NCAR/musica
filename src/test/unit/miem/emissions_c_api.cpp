// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Tests for the miem C interface, reusing the same real NOx fixture and 9:1
// NO:NO2 species-split config as emissions_model_end_to_end_nox.cpp.

#include <musica/configuration/read_mechanism_c_interface.hpp>
#include <musica/miem/emissions_c_interface.hpp>

#include "synthetic_nc.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

using namespace musica;

namespace
{
  constexpr int kNCells = 4097;                    // configs/miem/x1.163842_2024_nox_subset.nc
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
            "      file pattern: \"configs/miem/x1.163842_2024_nox_subset.nc\"\n"
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

  std::string SelectedEmissionsConfigYaml(const std::string& ground_path, const std::string& stack_path)
  {
    std::ostringstream yaml;
    yaml << "version: 1.0.0\n"
            "species: []\n"
            "phases: []\n"
            "reactions: []\n"
            "emissions:\n"
            "  inventories:\n"
            "    - name: ground inventory\n"
            "      directory: \"\"\n"
            "      file pattern: \""
         << ground_path
         << "\"\n"
            "      convention: uptempo\n"
            "    - name: stack inventory\n"
            "      directory: \"\"\n"
            "      file pattern: \""
         << stack_path
         << "\"\n"
            "      convention: uptempo\n"
            "  species maps:\n"
            "    - name: nox map\n"
            "      mappings:\n"
            "        - inventory species: NOx\n"
            "          mechanism species: NO\n"
            "          scaling factor: 0.9\n"
            "        - inventory species: NOx\n"
            "          mechanism species: NO2\n"
            "          scaling factor: 0.1\n"
            "  regridding:\n"
            "    type: none\n"
            "  sources:\n"
            "    - name: ground source\n"
            "      mode: offline\n"
            "      type: anthropogenic\n"
            "      inventory: ground inventory\n"
            "      species map: nox map\n"
            "      temporal interpolation: linear\n"
            "      vertical injection: surface\n"
            "      category: 0\n"
            "      hierarchy: 1\n"
            "      sector: ground\n"
            "    - name: stack source\n"
            "      mode: offline\n"
            "      type: anthropogenic\n"
            "      inventory: stack inventory\n"
            "      species map: nox map\n"
            "      temporal interpolation: linear\n"
            "      vertical injection: profile\n"
            "      vertical profile: [0.0, 0.25, 0.75]\n"
            "      category: 1\n"
            "      hierarchy: 1\n"
            "      sector: stack\n";
    return yaml.str();
  }

  std::string MakeSelectedInventory(miem_test::TempDir& dir, const std::string& name, double flux)
  {
    constexpr int global_cells = 6;
    const std::string path = dir.File(name);
    miem_test::UptempoNcOptions options;
    options.write_grid_metadata = true;
    miem_test::CreateUptempoTestNetCDF(
        path,
        2,
        global_cells,
        { "1970-01-01_00:00:00", "1970-01-01_01:00:00" },
        { "NOx" },
        { std::vector<double>(2 * global_cells, flux) },
        options);
    return path;
  }
}  // namespace

class EmissionsCApiTestFixture : public ::testing::Test
{
 protected:
  Mechanism* mechanism = nullptr;
  EmissionsModel* emissions = nullptr;

  void SetUp() override
  {
    Error error;
    NoError(&error);
    mechanism = ReadMechanismFromStringC(EmissionsConfigYaml().c_str(), &error);
    ASSERT_TRUE(IsSuccess(error));
    ASSERT_NE(mechanism, nullptr);

    emissions = CreateEmissions(mechanism, kNCells, /*n_vert_levels=*/1, &error);
    ASSERT_TRUE(IsSuccess(error));
    ASSERT_NE(emissions, nullptr);
    DeleteError(&error);
  }

  void TearDown() override
  {
    Error error;
    DeleteEmissions(emissions, &error);
    ASSERT_TRUE(IsSuccess(error));
    DeleteMechanism(mechanism, &error);
    ASSERT_TRUE(IsSuccess(error));
    DeleteError(&error);
  }
};

TEST_F(EmissionsCApiTestFixture, NumSpeciesAndOrdering)
{
  Error error;
  NoError(&error);
  int num_species = GetNumSpecies(emissions, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(num_species, 2);

  Mappings species_ordering;
  GetEmissionsSpeciesOrdering(emissions, &species_ordering, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(species_ordering.size_, 2);

  bool found_no = false, found_no2 = false;
  for (std::size_t i = 0; i < species_ordering.size_; ++i)
  {
    std::string name(species_ordering.mappings_[i].name_.value_);
    if (name == "NO")
      found_no = true;
    if (name == "NO2")
      found_no2 = true;
  }
  EXPECT_TRUE(found_no);
  EXPECT_TRUE(found_no2);

  DeleteMappings(&species_ordering);
  DeleteError(&error);
}

TEST_F(EmissionsCApiTestFixture, LegacyConstructorReportsFullGridDimensionsAndNoSelection)
{
  Error error;
  NoError(&error);
  EXPECT_EQ(GetEmissionsNumGlobalCells(emissions, &error), kNCells);
  ASSERT_TRUE(IsSuccess(error));
  EXPECT_EQ(GetEmissionsNumCells(emissions, &error), kNCells);
  ASSERT_TRUE(IsSuccess(error));
  EXPECT_EQ(GetEmissionsNumVertLevels(emissions, &error), 1);
  ASSERT_TRUE(IsSuccess(error));

  size_t selection_size = 99;
  int* selected_ids = GetEmissionsSelectedGlobalCellIdsPointer(emissions, &selection_size, &error);
  ASSERT_TRUE(IsSuccess(error));
  EXPECT_EQ(selection_size, 0);
  EXPECT_EQ(selected_ids, nullptr);
  DeleteError(&error);
}

TEST_F(EmissionsCApiTestFixture, SurfaceFluxPointerAndStridesMatchExpectedSplit)
{
  Error error;
  NoError(&error);

  Mappings species_ordering;
  GetEmissionsSpeciesOrdering(emissions, &species_ordering, &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t no_index = 0, no2_index = 0;
  for (std::size_t i = 0; i < species_ordering.size_; ++i)
  {
    std::string name(species_ordering.mappings_[i].name_.value_);
    if (name == "NO")
      no_index = species_ordering.mappings_[i].index_;
    if (name == "NO2")
      no2_index = species_ordering.mappings_[i].index_;
  }
  DeleteMappings(&species_ordering);

  size_t cell_stride = 0, species_stride = 0;
  GetSurfaceFluxStrides(emissions, &cell_stride, &species_stride, &error);
  ASSERT_TRUE(IsSuccess(error));
  EXPECT_EQ(cell_stride, 1);
  EXPECT_EQ(species_stride, static_cast<size_t>(kNCells));

  EmissionsRun(emissions, kEpoch20240701, /*dt_seconds=*/3600.0, &error);
  ASSERT_TRUE(IsSuccess(error));

  size_t array_size = 0;
  double* flux = GetSurfaceFluxPointer(emissions, &array_size, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(flux, nullptr);
  ASSERT_EQ(array_size, static_cast<size_t>(2 * kNCells));

  double sum_no = 0.0;
  double sum_no2 = 0.0;
  for (int cell = 0; cell < kNCells; ++cell)
  {
    double flux_no = flux[no_index * species_stride + cell * cell_stride];
    double flux_no2 = flux[no2_index * species_stride + cell * cell_stride];
    EXPECT_FALSE(std::isnan(flux_no));
    EXPECT_FALSE(std::isnan(flux_no2));
    EXPECT_GE(flux_no, 0.0);
    EXPECT_GE(flux_no2, 0.0);
    EXPECT_NEAR(flux_no, 9.0 * flux_no2, 1e-12);
    sum_no += flux_no;
    sum_no2 += flux_no2;
  }
  EXPECT_GT(sum_no, 0.0);
  EXPECT_NEAR(sum_no, 9.0 * sum_no2, sum_no * 1e-6);

  DeleteError(&error);
}

// Checks that GetSurfaceFluxPointer keeps returning correct, current data
// across repeated EmissionsRun calls.
TEST_F(EmissionsCApiTestFixture, SurfaceFluxPointerStaysCorrectAcrossRepeatedRuns)
{
  Error error;
  NoError(&error);

  size_t cell_stride = 0, species_stride = 0;
  GetSurfaceFluxStrides(emissions, &cell_stride, &species_stride, &error);
  ASSERT_TRUE(IsSuccess(error));

  Mappings species_ordering;
  GetEmissionsSpeciesOrdering(emissions, &species_ordering, &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t no_index = 0, no2_index = 0;
  for (std::size_t i = 0; i < species_ordering.size_; ++i)
  {
    std::string name(species_ordering.mappings_[i].name_.value_);
    if (name == "NO")
      no_index = species_ordering.mappings_[i].index_;
    if (name == "NO2")
      no2_index = species_ordering.mappings_[i].index_;
  }
  DeleteMappings(&species_ordering);

  for (double epoch : { kEpoch20240701, kEpoch20240701 + 3600.0, kEpoch20240701 + 7200.0 })
  {
    EmissionsRun(emissions, epoch, /*dt_seconds=*/3600.0, &error);
    ASSERT_TRUE(IsSuccess(error));

    size_t array_size = 0;
    double* flux = GetSurfaceFluxPointer(emissions, &array_size, &error);
    ASSERT_TRUE(IsSuccess(error));
    ASSERT_NE(flux, nullptr);

    double flux_no = flux[no_index * species_stride];
    double flux_no2 = flux[no2_index * species_stride];
    EXPECT_GE(flux_no, 0.0);
    EXPECT_GE(flux_no2, 0.0);
    EXPECT_NEAR(flux_no, 9.0 * flux_no2, 1e-12);
  }

  DeleteError(&error);
}

TEST(EmissionsCApiTest, NullMechanismPointerIsAnError)
{
  Error error;
  NoError(&error);
  EmissionsModel* emissions = CreateEmissions(nullptr, kNCells, /*n_vert_levels=*/1, &error);
  EXPECT_EQ(emissions, nullptr);
  EXPECT_EQ(error.code_, MUSICA_MIEM_ERROR_CODE_NULL_POINTER);
  EXPECT_STREQ(error.category_.value_, MUSICA_MIEM_ERROR_CATEGORY);
  DeleteError(&error);
}

TEST(EmissionsCApiTest, BadConfigStringThrows)
{
  Error error;
  NoError(&error);
  Mechanism* mechanism = ReadMechanismFromStringC("", &error);
  EXPECT_EQ(mechanism, nullptr);
  EXPECT_FALSE(IsSuccess(error));
  DeleteError(&error);
}

TEST(EmissionsCApiScalabilityTest, SelectedLayersDiagnosticsAndExactMetadataAreInteroperable)
{
  constexpr int global_cells = 6;
  constexpr int levels = 3;
  const int selected_ids[] = { 6, 2, 3 };
  const int categories[] = { 0, 1 };

  miem_test::TempDir dir;
  const std::string ground_path = MakeSelectedInventory(dir, "ground.nc", 2.0e-9);
  const std::string stack_path = MakeSelectedInventory(dir, "stack.nc", 6.0e-9);

  Error error;
  NoError(&error);
  Mechanism* mechanism =
      ReadMechanismFromStringC(SelectedEmissionsConfigYaml(ground_path, stack_path).c_str(), &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(mechanism, nullptr);

  Mappings sectors{};
  CreateMappings(2, &sectors);
  ToMapping("ground", 0, &sectors.mappings_[0]);
  ToMapping("stack", 1, &sectors.mappings_[1]);
  EmissionsModel* selected = CreateEmissionsSelected(
      mechanism,
      global_cells,
      levels,
      selected_ids,
      3,
      &sectors,
      categories,
      2,
      /*layered_diagnostics=*/1,
      /*max_diagnostic_fields=*/24,
      &error);
  DeleteMappings(&sectors);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(selected, nullptr);

  EXPECT_EQ(GetEmissionsNumGlobalCells(selected, &error), global_cells);
  ASSERT_TRUE(IsSuccess(error));
  EXPECT_EQ(GetEmissionsNumCells(selected, &error), 3);
  ASSERT_TRUE(IsSuccess(error));
  EXPECT_EQ(GetEmissionsNumVertLevels(selected, &error), levels);
  ASSERT_TRUE(IsSuccess(error));

  size_t array_size = 0;
  int* selected_id_pointer = GetEmissionsSelectedGlobalCellIdsPointer(selected, &array_size, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(array_size, 3);
  EXPECT_EQ(std::vector<int>(selected_id_pointer, selected_id_pointer + array_size), (std::vector<int>{ 6, 2, 3 }));

  Mappings sector_ordering{};
  GetEmissionsSectorOrdering(selected, &sector_ordering, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(sector_ordering.size_, 2);
  EXPECT_STREQ(sector_ordering.mappings_[0].name_.value_, "ground");
  EXPECT_STREQ(sector_ordering.mappings_[1].name_.value_, "stack");
  DeleteMappings(&sector_ordering);

  int* category_pointer = GetEmissionsCategoryIdsPointer(selected, &array_size, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(array_size, 2);
  EXPECT_EQ(std::vector<int>(category_pointer, category_pointer + array_size), (std::vector<int>{ 0, 1 }));

  EmissionsGridMetadata metadata{};
  GetEmissionsGridMetadata(selected, &metadata, &error);
  EXPECT_FALSE(IsSuccess(error));
  EXPECT_EQ(metadata.available_, 0);

  EmissionsRun(selected, /*epoch_seconds=*/1800.0, /*dt_seconds=*/60.0, &error);
  ASSERT_TRUE(IsSuccess(error));

  size_t cell_stride = 0, level_stride = 0, species_stride = 0;
  GetLayerFluxStrides(selected, &cell_stride, &level_stride, &species_stride, &error);
  ASSERT_TRUE(IsSuccess(error));
  EXPECT_EQ(cell_stride, 1);
  EXPECT_EQ(level_stride, 3);
  EXPECT_EQ(species_stride, 9);

  double* layer_flux = GetLayerFluxPointer(selected, &array_size, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(layer_flux, nullptr);
  ASSERT_EQ(array_size, 18);
  for (int cell = 0; cell < 3; ++cell)
  {
    EXPECT_NEAR(layer_flux[0 * species_stride + 0 * level_stride + cell], 1.8e-9, 1.0e-18);
    EXPECT_NEAR(layer_flux[0 * species_stride + 1 * level_stride + cell], 1.35e-9, 1.0e-18);
    EXPECT_NEAR(layer_flux[0 * species_stride + 2 * level_stride + cell], 4.05e-9, 1.0e-18);
  }

  double* stack_column = GetSectorFluxPointer(selected, 1, &array_size, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(array_size, 6);
  double* category_one_layers = GetCategoryLayerFluxPointer(selected, 1, &array_size, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(array_size, 18);
  for (int cell = 0; cell < 3; ++cell)
  {
    EXPECT_NEAR(stack_column[cell], 5.4e-9, 1.0e-18);
    EXPECT_DOUBLE_EQ(category_one_layers[cell], 0.0);
    EXPECT_NEAR(category_one_layers[level_stride + cell], 1.35e-9, 1.0e-18);
    EXPECT_NEAR(category_one_layers[2 * level_stride + cell], 4.05e-9, 1.0e-18);
  }

  GetEmissionsGridMetadata(selected, &metadata, &error);
  ASSERT_TRUE(IsSuccess(error));
  EXPECT_EQ(metadata.available_, 1);
  EXPECT_EQ(metadata.exact_grid_, 1);
  EXPECT_EQ(metadata.global_n_cells_, global_cells);
  EXPECT_EQ(metadata.geometry_, MUSICA_EMISSIONS_GRID_GEOMETRY_PLANAR);
  EXPECT_STREQ(metadata.on_a_sphere_.value_, "NO");
  EXPECT_STREQ(metadata.fingerprint_algorithm_.value_, "chempas-mesh-sha256-v1");
  EXPECT_EQ(metadata.fingerprint_.size_, 64);
  EXPECT_NE(metadata.field_mask_ & (std::uint32_t{ 1 } << MUSICA_EMISSIONS_GRID_FIELD_AREA_CELL), 0U);
  EXPECT_NE(metadata.field_mask_ & (std::uint32_t{ 1 } << MUSICA_EMISSIONS_GRID_FIELD_X_CELL), 0U);

  std::int64_t* index_to_cell_id = GetEmissionsGridIndexToCellIdPointer(selected, &array_size, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(array_size, 3);
  EXPECT_EQ(
      std::vector<std::int64_t>(index_to_cell_id, index_to_cell_id + array_size),
      (std::vector<std::int64_t>{ 6, 2, 3 }));

  double* area = GetEmissionsGridFieldPointer(selected, MUSICA_EMISSIONS_GRID_FIELD_AREA_CELL, &array_size, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(array_size, 3);
  EXPECT_EQ(std::vector<double>(area, area + array_size), (std::vector<double>{ 1005.0, 1001.0, 1002.0 }));
  String area_units{};
  GetEmissionsGridFieldUnits(selected, MUSICA_EMISSIONS_GRID_FIELD_AREA_CELL, &area_units, &error);
  ASSERT_TRUE(IsSuccess(error));
  EXPECT_STREQ(area_units.value_, "m2");
  DeleteString(&area_units);

  DeleteEmissionsGridMetadata(&metadata);
  DeleteEmissions(selected, &error);
  EXPECT_TRUE(IsSuccess(error));
  DeleteMechanism(mechanism, &error);
  EXPECT_TRUE(IsSuccess(error));
  DeleteError(&error);
}

TEST(EmissionsCApiScalabilityTest, SelectedConstructorRejectsNullCellIds)
{
  Error error;
  NoError(&error);
  Mechanism* mechanism = ReadMechanismFromStringC(EmissionsConfigYaml().c_str(), &error);
  ASSERT_TRUE(IsSuccess(error));

  EmissionsModel* selected = CreateEmissionsSelected(
      mechanism,
      kNCells,
      1,
      nullptr,
      1,
      nullptr,
      nullptr,
      0,
      0,
      0,
      &error);
  EXPECT_EQ(selected, nullptr);
  EXPECT_EQ(error.code_, MUSICA_MIEM_ERROR_CODE_NULL_POINTER);

  DeleteMechanism(mechanism, &error);
  EXPECT_TRUE(IsSuccess(error));
  DeleteError(&error);
}
