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
  std::cout << "CARMA version: " << version << std::endl;

  char *version_ptr = GetCarmaVersion();
  ASSERT_NE(version_ptr, nullptr);

  ASSERT_STREQ(version_ptr, version.c_str());
  delete[] version_ptr;  // Free the memory allocated by GetCarmaVersion
}

TEST_F(CarmaCApiTest, RunCarmaWithDefaultParameters)
{
  CARMA carma;
  CARMAParameters default_params;

  // Test that we can run CARMA with default parameters without throwing
  ASSERT_NO_THROW(carma.Run(default_params));
}

TEST_F(CarmaCApiTest, RunCarmaWithAluminumTestParams)
{
  CARMA carma;
  CARMAParameters params = CARMA::CreateAluminumTestParams();

  // Verify the aluminum test parameters are set correctly
  EXPECT_EQ(params.nz, 1);
  EXPECT_EQ(params.ny, 1);
  EXPECT_EQ(params.nx, 1);
  EXPECT_EQ(params.nbin, 5);
  EXPECT_EQ(params.nsolute, 0);
  EXPECT_EQ(params.ngas, 0);
  EXPECT_EQ(params.nwave, 30);
  EXPECT_EQ(params.dtime, 1800.0);
  EXPECT_EQ(params.deltaz, 1000.0);
  EXPECT_EQ(params.zmin, 16500.0);

  // Test that we can run CARMA with aluminum test parameters
  ASSERT_NO_THROW(carma.Run(params));
}