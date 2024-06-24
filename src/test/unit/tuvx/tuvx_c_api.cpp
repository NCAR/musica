#include <musica/tuvx.hpp>

#include <gtest/gtest.h>

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

TEST_F(TuvxCApiTest, CreateTuvxInstanceWithYamlConfig)
{
  const char* yaml_config_path = "examples/ts1_tsmlt.yml";
  SetUp(yaml_config_path);
  ASSERT_NE(tuvx, nullptr);
}

TEST_F(TuvxCApiTest, CreateTuvxInstanceWithJsonConfig)
{
  const char* json_config_path = "examples/ts1_tsmlt.json";
  SetUp(json_config_path);
  ASSERT_NE(tuvx, nullptr);
}

TEST_F(TuvxCApiTest, DetectsNonexistentConfigFile)
{
  const char* config_path = "nonexisting.yml";
  Error error;
  TUVX* tuvx = CreateTuvx(config_path, &error);
  ASSERT_FALSE(IsSuccess(error));
  DeleteError(&error);
}

TEST_F(TuvxCApiTest, CanGetGrid)
{
  const char* yaml_config_path = "examples/ts1_tsmlt.yml";
  SetUp(yaml_config_path);
  Error error;
  GridMap* grid_map = GetGridMap(tuvx, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(grid_map, nullptr);
  Grid* grid = GetGrid(grid_map, "height", "km", &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(grid, nullptr);
  std::vector<double> edges = { 0.0, 1.0, 2.0 };
  ASSERT_NO_THROW(SetEdges(grid, edges.data(), edges.size(), &error););
  std::vector<double> midpoints = { 0.5, 1.5 };
  ASSERT_NO_THROW(SetMidpoints(grid, midpoints.data(), midpoints.size(), &error););
  DeleteError(&error);
}
