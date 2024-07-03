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
  DeleteGridMap(grid_map, &error);
  ASSERT_TRUE(IsSuccess(error));
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

TEST_F(TuvxCApiTest, CanCreateGridMap)
{
  Error error;
  GridMap* grid_map = CreateGridMap(&error);
  ASSERT_TRUE(IsSuccess(error));
  Grid* foo_grid = CreateGrid("foo", "m", 2, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(foo_grid, nullptr);
  AddGrid(grid_map, foo_grid, &error);
  ASSERT_TRUE(IsSuccess(error));
  Grid* bar_grid = CreateGrid("bar", "m", 3, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(bar_grid, nullptr);
  AddGrid(grid_map, bar_grid, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(grid_map, nullptr);
  double edge_values[] = { 0.0, 1.0, 2.0 };
  double midpoint_values[] = { 0.5, 1.5 };
  SetGridEdgesAndMidpoints(foo_grid, edge_values, 3, midpoint_values, 2, &error);
  ASSERT_TRUE(IsSuccess(error));
  for (auto& edge : edge_values)
  {
    edge = -100.0;
  }
  for (auto& midpoint : midpoint_values)
  {
    midpoint = -100.0;
  }
  GetGridEdges(foo_grid, edge_values, 3, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(edge_values[0], 0.0);
  ASSERT_EQ(edge_values[1], 1.0);
  ASSERT_EQ(edge_values[2], 2.0);
  GetGridMidpoints(foo_grid, midpoint_values, 2, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(midpoint_values[0], 0.5);
  ASSERT_EQ(midpoint_values[1], 1.5);
  Grid *foo_copy = GetGrid(grid_map, "foo", "m", &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(foo_copy, nullptr);
  for (auto& edge : edge_values)
  {
    edge = -100.0;
  }
  for (auto& midpoint : midpoint_values)
  {
    midpoint = -100.0;
  }
  GetGridEdges(foo_copy, edge_values, 3, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(edge_values[0], 0.0);
  ASSERT_EQ(edge_values[1], 1.0);
  ASSERT_EQ(edge_values[2], 2.0);
  GetGridMidpoints(foo_copy, midpoint_values, 2, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(midpoint_values[0], 0.5);
  ASSERT_EQ(midpoint_values[1], 1.5);
  DeleteGrid(foo_grid, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteGrid(bar_grid, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteGrid(foo_copy, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteGridMap(grid_map, &error);
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
