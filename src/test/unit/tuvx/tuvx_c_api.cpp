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
  std::vector<double> midpoints = { 50.0, 150.0 };
  SetGridMidpoints(grid, midpoints.data(), midpoints.size(), &error);
  ASSERT_TRUE(IsSuccess(error));
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
  SetGridEdges(foo_grid, edge_values, 3, &error);
  SetGridMidpoints(foo_grid, midpoint_values, 2, &error);
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

TEST_F(TuvxCApiTest, CannotGetConfiguredProfile)
{
  const char* yaml_config_path = "examples/ts1_tsmlt.yml";
  SetUp(yaml_config_path);
  Error error;
  ProfileMap* profile_map = GetProfileMap(tuvx, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(profile_map, nullptr);
  Profile* profile = GetProfile(profile_map, "air", "molecule cm-3", &error);
  ASSERT_FALSE(IsSuccess(error)); // non-host profile
  ASSERT_EQ(profile, nullptr);
  DeleteProfileMap(profile_map, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteError(&error);
}

TEST_F(TuvxCApiTest, CanCreateProfile)
{
  Error error;
  Grid* grid = CreateGrid("foo", "m", 2, &error);
  ASSERT_TRUE(IsSuccess(error));
  Profile* profile = CreateProfile("bar", "molecule cm-3", grid, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(profile, nullptr);
  std::vector<double> edge_values = { 0.0, 1.0, 2.0 };
  SetProfileEdgeValues(profile, edge_values.data(), edge_values.size(), &error);
  ASSERT_TRUE(IsSuccess(error));
  for (auto& edge : edge_values)
  {
    edge = -100.0;
  }
  GetProfileEdgeValues(profile, edge_values.data(), edge_values.size(), &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(edge_values[0], 0.0);
  ASSERT_EQ(edge_values[1], 1.0);
  ASSERT_EQ(edge_values[2], 2.0);
  std::vector<double> midpoint_values = { 0.5, 1.5 };
  SetProfileMidpointValues(profile, midpoint_values.data(), midpoint_values.size(), &error);
  ASSERT_TRUE(IsSuccess(error));
  for (auto& midpoint : midpoint_values)
  {
    midpoint = -100.0;
  }
  GetProfileMidpointValues(profile, midpoint_values.data(), midpoint_values.size(), &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(midpoint_values[0], 0.5);
  ASSERT_EQ(midpoint_values[1], 1.5);
  std::vector<double> densities = { 1.0, 2.0 };
  SetProfileLayerDensities(profile, densities.data(), densities.size(), &error);
  ASSERT_TRUE(IsSuccess(error));
  for (auto& density : densities)
  {
    density = -100.0;
  }
  GetProfileLayerDensities(profile, densities.data(), densities.size(), &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(densities[0], 1.0);
  ASSERT_EQ(densities[1], 2.0);
  SetProfileExoLayerDensity(profile, 3.0, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(GetProfileExoLayerDensity(profile, &error), 3.0);
  GetProfileLayerDensities(profile, densities.data(), densities.size(), &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(densities[0], 1.0);
  ASSERT_EQ(densities[1], 2.0 + 3.0);
  CalculateProfileExoLayerDensity(profile, 1.0, &error);
  ASSERT_TRUE(IsSuccess(error));
  // This should be updated once we do all conversions to/from non-SI units
  // in the internal TUV-x functions
  ASSERT_EQ(GetProfileExoLayerDensity(profile, &error), 200.0);
  GetProfileLayerDensities(profile, densities.data(), densities.size(), &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(densities[0], 1.0);
  // This should be updated once we do all conversions to/from non-SI units
  // in the internal TUV-x functions
  ASSERT_EQ(densities[1], 2.0 + 200.0);
  DeleteProfile(profile, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteGrid(grid, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteError(&error);
}

TEST_F(TuvxCApiTest, CanCreateProfileMap)
{
  Error error;
  ProfileMap* profile_map = CreateProfileMap(&error);
  ASSERT_TRUE(IsSuccess(error));
  Grid* foo_grid = CreateGrid("foo", "m", 2, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(foo_grid, nullptr);
  Profile* foo_profile = CreateProfile("foo", "molecule cm-3", foo_grid, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(foo_profile, nullptr);
  AddProfile(profile_map, foo_profile, &error);
  ASSERT_TRUE(IsSuccess(error));
  Grid* bar_grid = CreateGrid("bar", "m", 3, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(bar_grid, nullptr);
  Profile* bar_profile = CreateProfile("bar", "molecule cm-3", bar_grid, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(bar_profile, nullptr);
  AddProfile(profile_map, bar_profile, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(profile_map, nullptr);
  double edge_values[] = { 0.0, 1.0, 2.0 };
  double midpoint_values[] = { 0.5, 1.5 };
  SetProfileEdgeValues(foo_profile, edge_values, 3, &error);
  SetProfileMidpointValues(foo_profile, midpoint_values, 2, &error);
  ASSERT_TRUE(IsSuccess(error));
  for (auto& edge : edge_values)
  {
    edge = -100.0;
  }
  for (auto& midpoint : midpoint_values)
  {
    midpoint = -100.0;
  }
  GetProfileEdgeValues(foo_profile, edge_values, 3, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(edge_values[0], 0.0);
  ASSERT_EQ(edge_values[1], 1.0);
  ASSERT_EQ(edge_values[2], 2.0);
  GetProfileMidpointValues(foo_profile, midpoint_values, 2, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(midpoint_values[0], 0.5);
  ASSERT_EQ(midpoint_values[1], 1.5);
  Profile *foo_copy = GetProfile(profile_map, "foo", "molecule cm-3", &error);
  for (auto& edge : edge_values)
  {
    edge = -100.0;
  }
  for (auto& midpoint : midpoint_values)
  {
    midpoint = -100.0;
  }
  GetProfileEdgeValues(foo_copy, edge_values, 3, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(edge_values[0], 0.0);
  ASSERT_EQ(edge_values[1], 1.0);
  ASSERT_EQ(edge_values[2], 2.0);
  GetProfileMidpointValues(foo_copy, midpoint_values, 2, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(midpoint_values[0], 0.5);
  ASSERT_EQ(midpoint_values[1], 1.5);
  edge_values[0] = 5.0;
  edge_values[1] = 10.0;
  edge_values[2] = 20.0;
  midpoint_values[0] = 7.5;
  midpoint_values[1] = 15.0;
  SetProfileEdgeValues(foo_copy, edge_values, 3, &error);
  SetProfileMidpointValues(foo_copy, midpoint_values, 2, &error);
  ASSERT_TRUE(IsSuccess(error));
  for (auto& edge : edge_values)
  {
    edge = -100.0;
  }
  for (auto& midpoint : midpoint_values)
  {
    midpoint = -100.0;
  }
  GetProfileEdgeValues(foo_copy, edge_values, 3, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(edge_values[0], 5.0);
  ASSERT_EQ(edge_values[1], 10.0);
  ASSERT_EQ(edge_values[2], 20.0);
  GetProfileMidpointValues(foo_copy, midpoint_values, 2, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(midpoint_values[0], 7.5);
  ASSERT_EQ(midpoint_values[1], 15.0);
  for (auto& edge : edge_values)
  {
    edge = -100.0;
  }
  for (auto& midpoint : midpoint_values)
  {
    midpoint = -100.0;
  }
  GetProfileEdgeValues(foo_profile, edge_values, 3, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(edge_values[0], 5.0);
  ASSERT_EQ(edge_values[1], 10.0);
  ASSERT_EQ(edge_values[2], 20.0);
  GetProfileMidpointValues(foo_profile, midpoint_values, 2, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(midpoint_values[0], 7.5);
  ASSERT_EQ(midpoint_values[1], 15.0);
  DeleteProfile(foo_profile, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteProfile(bar_profile, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteProfile(foo_copy, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteGrid(foo_grid, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteGrid(bar_grid, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteProfileMap(profile_map, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteError(&error);
}
