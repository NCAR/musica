#include <musica/tuvx/tuvx.hpp>

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

TEST_F(TuvxCApiTest, CannotGetConfiguredGrid)
{
  const char* yaml_config_path = "examples/ts1_tsmlt.yml";
  SetUp(yaml_config_path);
  Error error;
  GridMap* grid_map = GetGridMap(tuvx, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(grid_map, nullptr);
  Grid* grid = GetGrid(grid_map, "height", "km", &error);
  ASSERT_FALSE(IsSuccess(error)); // non-host grid
  ASSERT_EQ(grid, nullptr);
  DeleteError(&error);
}

TEST_F(TuvxCApiTest, CanCreateGrid)
{
  Error error;
  Grid* grid = CreateGrid("foo", "m", 2, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(grid, nullptr);
  std::vector<double> edges = { 0.0, 100.0, 200.0 };
  SetGridEdges(grid, edges.data(), edges.size(), &error);
  ASSERT_TRUE(IsSuccess(error));
  for (auto& edge : edges)
  {
    edge = -100.0;
  }
  GetGridEdges(grid, edges.data(), edges.size(), &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(edges[0], 0.0);
  ASSERT_EQ(edges[1], 100.0);
  ASSERT_EQ(edges[2], 200.0);
  edges = { 0.0, 90.0, 180.0 };
  std::vector<double> midpoints = { 50.0, 150.0 };
  SetGridEdgesAndMidpoints(grid, edges.data(), edges.size(), midpoints.data(), midpoints.size(), &error);
  ASSERT_TRUE(IsSuccess(error));
  for (auto& edge : edges)
  {
    edge = -100.0;
  }
  GetGridEdges(grid, edges.data(), edges.size(), &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(edges[0], 0.0);
  ASSERT_EQ(edges[1], 90.0);
  ASSERT_EQ(edges[2], 180.0);
  for (auto& midpoint : midpoints)
  {
    midpoint = -100.0;
  }
  GetGridMidpoints(grid, midpoints.data(), midpoints.size(), &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(midpoints[0], 50.0);
  ASSERT_EQ(midpoints[1], 150.0);
  DeleteGrid(grid, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteError(&error);
}

TEST_F(TuvxCApiTest, CanGetProfile)
{
  const char* yaml_config_path = "examples/ts1_tsmlt.yml";
  SetUp(yaml_config_path);
  Error error;
  ProfileMap* profile_map = GetProfileMap(tuvx, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(profile_map, nullptr);
  Profile* profile = GetProfile(profile_map, "air", "molecule cm-3", &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(profile, nullptr);
  std::vector<double> edge_values = { 0.0, 1.0, 2.0 };
  ASSERT_NO_THROW(SetProfileEdgeValues(profile, edge_values.data(), edge_values.size(), &error););
  std::vector<double> midpoint_values = { 0.5, 1.5 };
  ASSERT_NO_THROW(SetProfileMidpointValues(profile, midpoint_values.data(), midpoint_values.size(), &error););
  std::vector<double> densities = { 1.0, 2.0 };
  ASSERT_NO_THROW(SetProfileLayerDensities(profile, densities.data(), densities.size(), &error););
  ASSERT_NO_THROW(SetProfileExoLayerDensity(profile, 3.0, &error););
  ASSERT_NO_THROW(CalculateProfileExoLayerDensity(profile, 1.0, &error););
  DeleteError(&error);
}
