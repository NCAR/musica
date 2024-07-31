#include <musica/tuvx/tuvx.hpp>
#include <musica/tuvx/radiator.hpp>

#include <gtest/gtest.h>
#include <vector>
#include <iostream>

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

// TEST_F(TuvxCApiTest, CannotGetConfiguredRadiator)
// {
//   const char* yaml_config_path = "examples/ts1_tsmlt.yml";
//   SetUp(yaml_config_path);
//   Error error;
//   RadiatorMap* radiator_map = GetRadiatorMap(tuvx, &error);
//   ASSERT_TRUE(IsSuccess(error));
//   ASSERT_NE(radiator_map, nullptr);
//   Radiator* radiator = GetRadiator(radiator_map, "foo", &error);
//   ASSERT_FALSE(IsSuccess(error));  // non-host grid
//   ASSERT_EQ(radiator, nullptr);
//   DeleteRadiatorMap(radiator_map, &error);
//   ASSERT_TRUE(IsSuccess(error));
//   DeleteError(&error);
// }

TEST_F(TuvxCApiTest, CanCreateRadiator)
{
  Error error;
  Grid* height = CreateGrid("height", "km", 3, &error);
  Grid* wavelength = CreateGrid("wavelength", "nm", 2, &error);
  Radiator* radiator = CreateRadiator("foo", height, wavelength, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(radiator, nullptr);

  // Test for optical depths
  std::size_t num_vertical_layers = 3;
  std::size_t num_wavelength_bins = 2;

  // Allocate array as 1D
  double* optical_depths_1D = new double[num_wavelength_bins * num_vertical_layers];
  // Allocate an array of pointers to each row
  double** optical_depths = new double* [num_vertical_layers];
  // Fill in the pointers to the rows
  for(int row =0; row<num_vertical_layers; row++)
  {
    optical_depths[row] = &optical_depths_1D[row * num_wavelength_bins];
  }
  // 
  int i = 1;
  for(int row = 0; row < num_vertical_layers; row++)
  {
    for(int col = 0; col < num_wavelength_bins; col++)
    {
      optical_depths[row][col] = 10 * i;
      i++;
    }
  }
  SetRadiatorOpticalDepths(radiator, optical_depths[0], num_vertical_layers, num_wavelength_bins, &error);
  ASSERT_TRUE(IsSuccess(error));
  for(int row = 0; row < num_vertical_layers; row++)
  {
    for(int col = 0; col < num_wavelength_bins; col++)
    {
      optical_depths[row][col] =- 999.0;
    }
  }
  GetRadiatorOpticalDepths(radiator, optical_depths[0], num_vertical_layers, num_wavelength_bins, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(optical_depths[0][0], 10.0);
  ASSERT_EQ(optical_depths[0][1], 20.0);
  ASSERT_EQ(optical_depths[1][0], 30.0);
  ASSERT_EQ(optical_depths[1][1], 40.0);
  ASSERT_EQ(optical_depths[2][0], 50.0);
  ASSERT_EQ(optical_depths[2][1], 60.0);

  // Test for single scattering albedos
  double* albedos_1D = new double[num_wavelength_bins * num_vertical_layers];
  double** albedos = new double* [num_vertical_layers];
  for(int row =0; row<num_vertical_layers; row++)
  {
    albedos[row] = &albedos_1D[row * num_wavelength_bins];
  }
  i = 1;
  for(int row = 0; row < num_vertical_layers; row++)
  {
    for(int col = 0; col < num_wavelength_bins; col++)
    {
      albedos[row][col] = 100 * i;
      i++;
    }
  }
  SetRadiatorSingleScatteringAlbedos(radiator, albedos[0], num_vertical_layers, num_wavelength_bins, &error);
  ASSERT_TRUE(IsSuccess(error));
  for(int row = 0; row < num_vertical_layers; row++)
  {
    for(int col = 0; col < num_wavelength_bins; col++)
    {
      albedos[row][col] =- 999.0;
    }
  }
  GetRadiatorSingleScatteringAlbedos(radiator, albedos[0], num_vertical_layers, num_wavelength_bins, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(albedos[0][0], 100.0);
  ASSERT_EQ(albedos[0][1], 200.0);
  ASSERT_EQ(albedos[1][0], 300.0);
  ASSERT_EQ(albedos[1][1], 400.0);
  ASSERT_EQ(albedos[2][0], 500.0);
  ASSERT_EQ(albedos[2][1], 600.0);

  // Test for asymmetery factors
  double* factors_1D = new double[num_wavelength_bins * num_vertical_layers];
  double** factors = new double* [num_vertical_layers];
  for(int row =0; row<num_vertical_layers; row++)
  {
    factors[row] = &factors_1D[row * num_wavelength_bins];
  }
  i = 1;
  for(int row = 0; row < num_vertical_layers; row++)
  {
    for(int col = 0; col < num_wavelength_bins; col++)
    {
      factors[row][col] = 1 * i;
      i++;
    }
  }
  std::size_t num_streams = 1;
  SetRadiatorAsymmetryFactors(radiator, factors[0], num_vertical_layers, num_wavelength_bins, num_streams, &error);
  ASSERT_TRUE(IsSuccess(error));
  for(int row = 0; row < num_vertical_layers; row++)
  {
    for(int col = 0; col < num_wavelength_bins; col++)
    {
      factors[row][col] =- 999.0;
    }
  }
  GetRadiatorAsymmetryFactors(radiator, factors[0], num_vertical_layers, num_wavelength_bins, num_streams, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(factors[0][0], 1);
  ASSERT_EQ(factors[0][1], 2);
  ASSERT_EQ(factors[1][0], 3);
  ASSERT_EQ(factors[1][1], 4);
  ASSERT_EQ(factors[2][0], 5);
  ASSERT_EQ(factors[2][1], 6);

  // clean up
  DeleteRadiator(radiator, &error); ASSERT_TRUE(IsSuccess(error));
  DeleteGrid(height, &error); ASSERT_TRUE(IsSuccess(error));
  DeleteGrid(wavelength, &error); ASSERT_TRUE(IsSuccess(error));
  DeleteError(&error);
  delete[] optical_depths;
  delete[] optical_depths_1D;
  delete[] albedos;
  delete[] albedos_1D;
  delete[] factors;
  delete[] factors_1D;
}

TEST_F(TuvxCApiTest, CanCreateRadiatorMap)
{
  Error error;
  RadiatorMap* radiator_map = CreateRadiatorMap(&error);
  ASSERT_TRUE(IsSuccess(error));

  Grid* height = CreateGrid("height", "km", 3, &error);
  Grid* wavelength = CreateGrid("wavelength", "nm", 2, &error);
  Radiator* foo_radiator = CreateRadiator("foo", height, wavelength, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(foo_radiator, nullptr);
  // 
  // TODO(test)
  //
  AddRadiator(radiator_map, foo_radiator, &error);
  ASSERT_TRUE(IsSuccess(error));

  Grid* bar_height = CreateGrid("bar_height", "km", 3, &error);
  Grid* bar_wavelength = CreateGrid("bar_wavelength", "nm", 2, &error);
  Radiator* bar_radiator = CreateRadiator("bar", bar_height, bar_wavelength, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(bar_radiator, nullptr);
  AddRadiator(radiator_map, bar_radiator, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(radiator_map, nullptr);
  
  std::size_t num_vertical_layers = 3;
  std::size_t num_wavelength_bins = 2;

  // Test for optical depths
  double* optical_depths_1D = new double[num_wavelength_bins * num_vertical_layers];
  double** optical_depths = new double* [num_vertical_layers];
  for(int row =0; row<num_vertical_layers; row++)
  {
    optical_depths[row] = &optical_depths_1D[row * num_wavelength_bins];
  }
  int i = 1;
  for(int row = 0; row < num_vertical_layers; row++)
  {
    for(int col = 0; col < num_wavelength_bins; col++)
    {
      optical_depths[row][col] = 10 * i;
      i++;
    }
  }
  SetRadiatorOpticalDepths(foo_radiator, optical_depths[0], num_vertical_layers, num_wavelength_bins, &error);
  ASSERT_TRUE(IsSuccess(error));

  // Test for single scattering albedos
  double* albedos_1D = new double[num_wavelength_bins * num_vertical_layers];
  double** albedos = new double* [num_vertical_layers];
  for(int row =0; row<num_vertical_layers; row++)
  {
    albedos[row] = &albedos_1D[row * num_wavelength_bins];
  }
  i = 1;
  for(int row = 0; row < num_vertical_layers; row++)
  {
    for(int col = 0; col < num_wavelength_bins; col++)
    {
      albedos[row][col] = 100 * i;
      i++;
    }
  }
  SetRadiatorSingleScatteringAlbedos(foo_radiator, albedos[0], num_vertical_layers, num_wavelength_bins, &error);
  ASSERT_TRUE(IsSuccess(error));

  // Test for asymmetery factors
  std::size_t num_streams = 1;
  double* factors_1D = new double[num_wavelength_bins * num_vertical_layers];
  double** factors = new double* [num_vertical_layers];
  for(int row =0; row<num_vertical_layers; row++)
  {
    factors[row] = &factors_1D[row * num_wavelength_bins];
  }
  i = 1;
  for(int row = 0; row < num_vertical_layers; row++)
  {
    for(int col = 0; col < num_wavelength_bins; col++)
    {
      factors[row][col] = 1 * i;
      i++;
    }
  }
  SetRadiatorAsymmetryFactors(foo_radiator, factors[0], num_vertical_layers, num_wavelength_bins, num_streams, &error);
  ASSERT_TRUE(IsSuccess(error));

  // Test for optical depths
  for(int row = 0; row < num_vertical_layers; row++)
  {
    for(int col = 0; col < num_wavelength_bins; col++)
    {
      optical_depths[row][col] =- 999.0;
    }
  }
  GetRadiatorOpticalDepths(foo_radiator, optical_depths[0], num_vertical_layers, num_wavelength_bins, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(optical_depths[0][0], 10.0);
  ASSERT_EQ(optical_depths[0][1], 20.0);
  ASSERT_EQ(optical_depths[1][0], 30.0);
  ASSERT_EQ(optical_depths[1][1], 40.0);
  ASSERT_EQ(optical_depths[2][0], 50.0);
  ASSERT_EQ(optical_depths[2][1], 60.0);

  // Test for single scattering albedos
  for(int row = 0; row < num_vertical_layers; row++)
  {
    for(int col = 0; col < num_wavelength_bins; col++)
    {
      albedos[row][col] =- 999.0;
    }
  }
  GetRadiatorSingleScatteringAlbedos(foo_radiator, albedos[0], num_vertical_layers, num_wavelength_bins, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(albedos[0][0], 100.0);
  ASSERT_EQ(albedos[0][1], 200.0);
  ASSERT_EQ(albedos[1][0], 300.0);
  ASSERT_EQ(albedos[1][1], 400.0);
  ASSERT_EQ(albedos[2][0], 500.0);
  ASSERT_EQ(albedos[2][1], 600.0);

  // Test for asymmetry factors
  for(int row = 0; row < num_vertical_layers; row++)
  {
    for(int col = 0; col < num_wavelength_bins; col++)
    {
      factors[row][col] =- 999.0;
    }
  }
  GetRadiatorAsymmetryFactors(foo_radiator, factors[0], num_vertical_layers, num_wavelength_bins, num_streams, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(factors[0][0], 1);
  ASSERT_EQ(factors[0][1], 2);
  ASSERT_EQ(factors[1][0], 3);
  ASSERT_EQ(factors[1][1], 4);
  ASSERT_EQ(factors[2][0], 5);
  ASSERT_EQ(factors[2][1], 6);

  Radiator* foo_copy = GetRadiator(radiator_map, "foo", &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(foo_copy, nullptr);
  // TODO
  // Seg fault for the following call
  //
  GetRadiatorOpticalDepths(foo_copy, optical_depths[0], num_vertical_layers, num_wavelength_bins, &error);
  ASSERT_TRUE(IsSuccess(error));
  // ASSERT_EQ(optical_depths[0][0], 10.0);
  // ASSERT_EQ(optical_depths[0][1], 20.0);
  // ASSERT_EQ(optical_depths[1][0], 30.0);
  // ASSERT_EQ(optical_depths[1][1], 40.0);
  // ASSERT_EQ(optical_depths[2][0], 50.0);
  // ASSERT_EQ(optical_depths[2][1], 60.0);
  // GetRadiatorSingleScatteringAlbedos(foo_copy, albedos[0], num_vertical_layers, num_wavelength_bins, &error);
  // ASSERT_TRUE(IsSuccess(error));
  // ASSERT_EQ(albedos[0][0], 100.0);
  // ASSERT_EQ(albedos[0][1], 200.0);
  // ASSERT_EQ(albedos[1][0], 300.0);
  // ASSERT_EQ(albedos[1][1], 400.0);
  // ASSERT_EQ(albedos[2][0], 500.0);
  // ASSERT_EQ(albedos[2][1], 600.0);
  // GetRadiatorAsymmetryFactors(foo_copy, factors[0], num_vertical_layers, num_wavelength_bins, 1, &error);
  // ASSERT_TRUE(IsSuccess(error));
  // ASSERT_EQ(factors[0][0], 1);
  // ASSERT_EQ(factors[0][1], 2);
  // ASSERT_EQ(factors[1][0], 3);
  // ASSERT_EQ(factors[1][1], 4);
  // ASSERT_EQ(factors[2][0], 5);
  // ASSERT_EQ(factors[2][1], 6);

  // DeleteRadiator(foo_radiator, &error); ASSERT_TRUE(IsSuccess(error));
  // DeleteRadiator(bar_radiator, &error); ASSERT_TRUE(IsSuccess(error));
  // DeleteRadiator(foo_copy, &error); ASSERT_TRUE(IsSuccess(error));
  // DeleteRadiatorMap(radiator_map, &error); ASSERT_TRUE(IsSuccess(error));
  // DeleteError(&error);
  // delete[] optical_depths;
  // delete[] optical_depths_1D;
  // delete[] albedos;
  // delete[] albedos_1D;
  // delete[] factors;
  // delete[] factors_1D;
}
