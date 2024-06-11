#include <musica/tuvx.hpp>

#include <gtest/gtest.h>

using namespace musica;

// Test fixture for the TUVX C API
class TuvxCApiTest : public ::testing::Test
{
 protected:
  TUVX* tuvx;
  int error_code;
  const char* config_path;

  void SetUp(const char* path_to_config)
  {
    tuvx = nullptr;
    error_code = 0;
    config_path = path_to_config;  // Set the config path based on the parameter
    tuvx = CreateTuvx(config_path, &error_code);
  }

  void TearDown() override
  {
    DeleteTuvx(tuvx);
  }
};

TEST_F(TuvxCApiTest, CreateTuvxInstanceWithYamlConfig)
{
  const char* yaml_config_path = "examples/ts1_tsmlt.yml";
  SetUp(yaml_config_path);
  ASSERT_EQ(error_code, 0);
  ASSERT_NE(tuvx, nullptr);
}

TEST_F(TuvxCApiTest, CreateTuvxInstanceWithJsonConfig)
{
  const char* json_config_path = "examples/ts1_tsmlt.json";
  SetUp(json_config_path);
  ASSERT_EQ(error_code, 0);
  ASSERT_NE(tuvx, nullptr);
}

TEST_F(TuvxCApiTest, DetectsNonexistentYamlConfigFile)
{
  const char* yaml_config_path = "nonexisting.yml";
  SetUp(yaml_config_path);
  ASSERT_EQ(error_code, 2);
  ASSERT_EQ(tuvx, nullptr);
}

TEST_F(TuvxCApiTest, DetectsNonexistentJSONConfigFile)
{
  const char* json_config_path = "nonexisting.json";
  SetUp(json_config_path);
  ASSERT_EQ(error_code, 2);
  ASSERT_EQ(tuvx, nullptr);
}
