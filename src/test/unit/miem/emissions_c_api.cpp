// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Tests for the miem C interface, reusing the same real NOx fixture and 9:1
// NO:NO2 species-split config as emissions_model_end_to_end_nox.cpp.

#include <musica/configuration/read_mechanism_c_interface.hpp>
#include <musica/miem/emissions_c_interface.hpp>

#include <gtest/gtest.h>

#include <cmath>
#include <sstream>
#include <string>

using namespace musica;

namespace
{
  constexpr int kNCells = 4097;                    // configs/miem/x1.163842_2024_nox_subset.nc
  constexpr double kEpoch20240701 = 1719792000.0;   // 2024-07-01 00:00:00 UTC

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
  GetSurfaceFluxStrides(emissions, &error, &cell_stride, &species_stride);
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
  GetSurfaceFluxStrides(emissions, &error, &cell_stride, &species_stride);
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
