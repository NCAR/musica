#include <musica/tuvx/tuvx.hpp>
#include <musica/tuvx/radiator.hpp>

#include <gtest/gtest.h>
#include <vector>

using namespace musica;

// Test fixture for the TUVX C API
class TuvxCApiTest : public ::testing::Test
{
 protected:
  TUVX* tuvx;

  // the function that google test actually calls before each test
  void SetUp() override
  {
    tuvx = nullptr;
  }

  void SetUp(const char* config_path)
  {
    Error error;
    tuvx = CreateTuvx(config_path, &error);
    if (!IsSuccess(error))
    {
      std::cerr << "Error creating TUVX instance: " << error.message_.value_ << std::endl;
    }
    ASSERT_TRUE(IsSuccess(error));
    DeleteError(&error);
  }

  void TearDown() override
  {
    if (tuvx == nullptr)
    {
      return;
    }
    Error error;
    DeleteTuvx(tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    DeleteError(&error);
    tuvx = nullptr;
  }
};

TEST_F(TuvxCApiTest, CannotGetConfiguredRadiator)
{
  const char* yaml_config_path = "examples/ts1_tsmlt.yml";
  SetUp(yaml_config_path);
  Error error;
  RadiatorMap* radiator_map = GetRadiatorMap(tuvx, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(radiator_map, nullptr);
  Radiator* radiator = GetRadiator(radiator_map, "Aerosol radiator", &error);
  ASSERT_FALSE(IsSuccess(error));  // non-host grid
  ASSERT_EQ(radiator, nullptr);
  DeleteRadiatorMap(radiator_map, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteError(&error);
}

TEST_F(TuvxCApiTest, CanCreateRadiator)
{
  Error error;
  Grid* height = CreateGrid("height", "km", 3, &error);
  Grid* wavelength = CreateGrid("wavelength", "nm", 2, &error);
  Radiator* radiator = CreateRadiator("foo", height, wavelength, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(radiator, nullptr);
  
  // TODO(jiwon) - Is 1d array of input okay?
  // ! specify optical depths only
  // od(:,1) = (/ 12.5_dk, 42.3_dk,  0.4_dk /)
  // od(:,2) = (/ 49.2_dk, 12.5_dk, 92.1_dk /)
  
  std::vector<double> optical_depths_flattened = {10.0, 20.0, 30.0, 40.0, 50.0, 60.0};
  std::size_t num_vertical_layers = 3;
  std::size_t num_wavelength_bins = 2;
  SetRadiatorOpticalDepths(radiator, optical_depths_flattened.data(), num_vertical_layers, num_wavelength_bins, &error);
  ASSERT_TRUE(IsSuccess(error));

  for(auto& depth : optical_depths_flattened)
  {
    depth =- 10.0;
  }
  GetRadiatorOpticalDepths(radiator, optical_depths_flattened.data(), num_vertical_layers, num_wavelength_bins, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(optical_depths_flattened[0], 0.0);
  ASSERT_EQ(optical_depths_flattened[1], 10.0);
  ASSERT_EQ(optical_depths_flattened[2], 20.0);
  ASSERT_EQ(optical_depths_flattened[0], 30.0);
  ASSERT_EQ(optical_depths_flattened[1], 40.0);
  ASSERT_EQ(optical_depths_flattened[2], 50.0);

  num_vertical_layers = 2;
  num_wavelength_bins = 2;
  std::vector<double> single_scattering_albedos_flattened = { 100.0, 100.0, 200, 200 };
  SetRadiatorSingleScatteringAlbedos(radiator, single_scattering_albedos_flattened.data(), num_vertical_layers, num_wavelength_bins, &error)
  ASSERT_TRUE(IsSuccess(error));
  for (auto& albedos : single_scattering_albedos_flattened)
  {
    albedos =+ 100.0;
  }
  // GetGridMidpoints(grid, midpoints.data(), midpoints.size(), &error);
  // GetRadiatorSingleScatteringAlbedos(radiator, single_scattering_albedos_flattened.data(), num_vertical_layers, num_wavelength_bins, &error)
  // ASSERT_TRUE(IsSuccess(error));
  // ASSERT_EQ(midpoints[0], 50.0);
  // ASSERT_EQ(midpoints[1], 150.0);
  // DeleteGrid(grid, &error);
  // ASSERT_TRUE(IsSuccess(error));
  // DeleteError(&error);
}