#include <musica/carma/carma.hpp>
#include <musica/carma/carma_c_interface.hpp>

#include <gtest/gtest.h>

using namespace musica;

class CarmaCApiTest : public ::testing::Test
{
 protected:
  void SetUp() override
  {
    // Code to set up the test environment
  }

  void TearDown() override
  {
    // Code to clean up the test environment
  }
};

TEST_F(CarmaCApiTest, GetCarmaVersion)
{
  std::string version = CARMA::GetVersion();
  ASSERT_FALSE(version.empty());

  char *version_ptr = GetCarmaVersion();
  ASSERT_NE(version_ptr, nullptr);

  ASSERT_STREQ(version_ptr, version.c_str());
  delete[] version_ptr;  // Free the memory allocated by GetCarmaVersion
}

TEST_F(CarmaCApiTest, RunCarmaWithDefaultParameters)
{
  CARMAParameters default_params;
  CARMA carma{ default_params };

  // Test that we can run CARMA with default parameters without throwing
  ASSERT_NO_THROW(carma.Run());
}
TEST_F(CarmaCApiTest, RunCarmaWithAluminumTestParams)
{
  CARMAParameters params = CARMA::CreateAluminumTestParams();
  CARMA carma{ params };

  // Verify the aluminum test parameters are set correctly
  EXPECT_EQ(params.nz, 1);
  EXPECT_EQ(params.nbin, 5);
  EXPECT_EQ(params.nsolute, 0);
  EXPECT_EQ(params.ngas, 0);
  EXPECT_EQ(params.dtime, 1800.0);
  EXPECT_EQ(params.deltaz, 1000.0);
  EXPECT_EQ(params.zmin, 16500.0);
  EXPECT_EQ(params.wavelength_bins.size(), 5);
  EXPECT_EQ(params.number_of_refractive_indices, 1);

  // Run CARMA and get the output
  CARMAOutput output;
  ASSERT_NO_THROW(output = carma.Run());

  // Verify that the basic dimensions are correct
  EXPECT_EQ(output.lat.size(), 1);
  EXPECT_EQ(output.lon.size(), 1);
  EXPECT_EQ(output.vertical_center.size(), params.nz);
  EXPECT_EQ(output.pressure.size(), params.nz);
  EXPECT_EQ(output.temperature.size(), params.nz);
  EXPECT_EQ(output.air_density.size(), params.nz);

  // Verify that the new nucleation_rate field is properly sized (3D: nz x nbin x ngroup)
  EXPECT_EQ(output.nucleation_rate.size(), params.nz);
  if (!output.nucleation_rate.empty())
  {
    EXPECT_EQ(output.nucleation_rate[0].size(), params.nbin);
    if (!output.nucleation_rate[0].empty())
    {
      EXPECT_EQ(output.nucleation_rate[0][0].size(), params.groups.size());
    }
  }

  // Verify that the new deposition_velocity field is properly sized (3D: nz x nbin x ngroup)
  EXPECT_EQ(output.deposition_velocity.size(), params.nz);
  if (!output.deposition_velocity.empty())
  {
    EXPECT_EQ(output.deposition_velocity[0].size(), params.nbin);
    if (!output.deposition_velocity[0].empty())
    {
      EXPECT_EQ(output.deposition_velocity[0][0].size(), params.groups.size());
    }
  }

  // Verify that the new 1D group mapping arrays are properly sized
  EXPECT_EQ(output.group_particle_number_concentration.size(), params.groups.size());
  EXPECT_EQ(output.constituent_type.size(), params.groups.size());
  EXPECT_EQ(output.max_prognostic_bin.size(), params.groups.size());
}