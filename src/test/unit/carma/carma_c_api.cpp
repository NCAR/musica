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