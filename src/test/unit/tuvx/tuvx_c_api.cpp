#include <musica/tuvx/tuvx.hpp>

#include <gtest/gtest.h>

using namespace musica;

// Test fixture for the TUVX C API
class TuvxCApiTest : public ::testing::Test
{
 protected:
  TUVX* tuvx;
  GridMap* grids_from_host;
  ProfileMap* profiles_from_host;
  RadiatorMap* radiators_from_host;
  GridMap* grids_in_tuvx;
  ProfileMap* profiles_in_tuvx;
  RadiatorMap* radiators_in_tuvx;

  // the function that google test actually calls before each test
  void SetUp() override
  {
    tuvx = nullptr;
    grids_from_host = nullptr;
    profiles_from_host = nullptr;
    radiators_from_host = nullptr;
    grids_in_tuvx = nullptr;
    profiles_in_tuvx = nullptr;
    radiators_in_tuvx = nullptr;
  }

  void SetUp(const char* config_path)
  {
    Error error;
    grids_from_host = CreateGridMap(&error);
    ASSERT_TRUE(IsSuccess(error));
    profiles_from_host = CreateProfileMap(&error);
    ASSERT_TRUE(IsSuccess(error));
    radiators_from_host = CreateRadiatorMap(&error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx = CreateTuvx(config_path, grids_from_host, profiles_from_host, radiators_from_host, &error);
    if (!IsSuccess(error))
    {
      std::cerr << "Error creating TUVX instance: " << error.message_.value_ << std::endl;
    }
    ASSERT_TRUE(IsSuccess(error));
    grids_in_tuvx = GetGridMap(tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    profiles_in_tuvx = GetProfileMap(tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    radiators_in_tuvx = GetRadiatorMap(tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    DeleteError(&error);
  }

  void SetUp(const char* config_path, GridMap* grids, ProfileMap* profiles, RadiatorMap* radiators)
  {
    Error error;
    grids_from_host = CreateGridMap(&error);
    ASSERT_TRUE(IsSuccess(error));
    profiles_from_host = CreateProfileMap(&error);
    ASSERT_TRUE(IsSuccess(error));
    radiators_from_host = CreateRadiatorMap(&error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx = CreateTuvx(config_path, grids, profiles, radiators, &error);
    if (!IsSuccess(error))
    {
      std::cerr << "Error creating TUVX instance: " << error.message_.value_ << std::endl;
    }
    ASSERT_TRUE(IsSuccess(error));
    grids_in_tuvx = GetGridMap(tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    profiles_in_tuvx = GetProfileMap(tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    radiators_in_tuvx = GetRadiatorMap(tuvx, &error);
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
    DeleteGridMap(grids_from_host, &error);
    ASSERT_TRUE(IsSuccess(error));
    DeleteProfileMap(profiles_from_host, &error);
    ASSERT_TRUE(IsSuccess(error));
    DeleteRadiatorMap(radiators_from_host, &error);
    ASSERT_TRUE(IsSuccess(error));
    DeleteGridMap(grids_in_tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    DeleteProfileMap(profiles_in_tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    DeleteRadiatorMap(radiators_in_tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    DeleteError(&error);
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
  GridMap* grids_from_host = CreateGridMap(&error);
  ProfileMap* profiles_from_host = CreateProfileMap(&error);
  RadiatorMap* radiators_from_host = CreateRadiatorMap(&error);
  TUVX* tuvx = CreateTuvx(config_path, grids_from_host, profiles_from_host, radiators_from_host, &error);
  ASSERT_FALSE(IsSuccess(error));
  DeleteGridMap(grids_from_host, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteProfileMap(profiles_from_host, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteRadiatorMap(radiators_from_host, &error);
  ASSERT_TRUE(IsSuccess(error));
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
  ASSERT_FALSE(IsSuccess(error));  // non-host grid
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
  ASSERT_EQ(GetGridNumSections(grid, &error), 2);
  ASSERT_TRUE(IsSuccess(error));
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
  Grid* foo_copy = GetGrid(grid_map, "foo", "m", &error);
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
  ASSERT_FALSE(IsSuccess(error));  // non-host profile
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
  Profile* foo_copy = GetProfile(profile_map, "foo", "molecule cm-3", &error);
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

TEST_F(TuvxCApiTest, CannotGetConfiguredRadiator)
{
  const char* yaml_config_path = "examples/ts1_tsmlt.yml";
  SetUp(yaml_config_path);
  Error error;
  RadiatorMap* radiator_map = GetRadiatorMap(tuvx, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(radiator_map, nullptr);
  Radiator* radiator = GetRadiator(radiator_map, "foo", &error);
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

  // Test for optical depths
  std::size_t num_vertical_layers = 3;
  std::size_t num_wavelength_bins = 2;
  // Allocate array as 1D
  double* optical_depths_1D = new double[num_wavelength_bins * num_vertical_layers];
  // Allocate an array of pointers to each row
  double** optical_depths = new double*[num_vertical_layers];
  // Fill in the pointers to the rows
  for (int row = 0; row < num_vertical_layers; row++)
  {
    optical_depths[row] = &optical_depths_1D[row * num_wavelength_bins];
  }
  int i = 1;
  for (int row = 0; row < num_vertical_layers; row++)
  {
    for (int col = 0; col < num_wavelength_bins; col++)
    {
      optical_depths[row][col] = 10 * i;
      i++;
    }
  }
  SetRadiatorOpticalDepths(radiator, optical_depths[0], num_vertical_layers, num_wavelength_bins, &error);
  ASSERT_TRUE(IsSuccess(error));
  for (int row = 0; row < num_vertical_layers; row++)
  {
    for (int col = 0; col < num_wavelength_bins; col++)
    {
      optical_depths[row][col] = -999.0;
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
  double** albedos = new double*[num_vertical_layers];
  for (int row = 0; row < num_vertical_layers; row++)
  {
    albedos[row] = &albedos_1D[row * num_wavelength_bins];
  }
  i = 1;
  for (int row = 0; row < num_vertical_layers; row++)
  {
    for (int col = 0; col < num_wavelength_bins; col++)
    {
      albedos[row][col] = 100 * i;
      i++;
    }
  }
  SetRadiatorSingleScatteringAlbedos(radiator, albedos[0], num_vertical_layers, num_wavelength_bins, &error);
  ASSERT_TRUE(IsSuccess(error));
  for (int row = 0; row < num_vertical_layers; row++)
  {
    for (int col = 0; col < num_wavelength_bins; col++)
    {
      albedos[row][col] = -999.0;
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
  double** factors = new double*[num_vertical_layers];
  for (int row = 0; row < num_vertical_layers; row++)
  {
    factors[row] = &factors_1D[row * num_wavelength_bins];
  }
  i = 1;
  for (int row = 0; row < num_vertical_layers; row++)
  {
    for (int col = 0; col < num_wavelength_bins; col++)
    {
      factors[row][col] = 1 * i;
      i++;
    }
  }
  std::size_t num_streams = 1;
  SetRadiatorAsymmetryFactors(radiator, factors[0], num_vertical_layers, num_wavelength_bins, num_streams, &error);
  ASSERT_TRUE(IsSuccess(error));
  for (int row = 0; row < num_vertical_layers; row++)
  {
    for (int col = 0; col < num_wavelength_bins; col++)
    {
      factors[row][col] = -999.0;
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

  // Clean up
  DeleteRadiator(radiator, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteGrid(height, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteGrid(wavelength, &error);
  ASSERT_TRUE(IsSuccess(error));
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
  AddRadiator(radiator_map, foo_radiator, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(radiator_map, nullptr);
  Grid* bar_height = CreateGrid("bar_height", "km", 3, &error);
  Grid* bar_wavelength = CreateGrid("bar_wavelength", "nm", 2, &error);
  Radiator* bar_radiator = CreateRadiator("bar", bar_height, bar_wavelength, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(bar_radiator, nullptr);
  AddRadiator(radiator_map, bar_radiator, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(radiator_map, nullptr);

  // Test for optical depths
  std::size_t num_vertical_layers = 3;
  std::size_t num_wavelength_bins = 2;
  double* optical_depths_1D = new double[num_wavelength_bins * num_vertical_layers];
  double** optical_depths = new double*[num_vertical_layers];
  for (int row = 0; row < num_vertical_layers; row++)
  {
    optical_depths[row] = &optical_depths_1D[row * num_wavelength_bins];
  }
  int i = 1;
  for (int row = 0; row < num_vertical_layers; row++)
  {
    for (int col = 0; col < num_wavelength_bins; col++)
    {
      optical_depths[row][col] = 10 * i;
      i++;
    }
  }
  SetRadiatorOpticalDepths(foo_radiator, optical_depths[0], num_vertical_layers, num_wavelength_bins, &error);
  ASSERT_TRUE(IsSuccess(error));

  // Test for single scattering albedos
  double* albedos_1D = new double[num_wavelength_bins * num_vertical_layers];
  double** albedos = new double*[num_vertical_layers];
  for (int row = 0; row < num_vertical_layers; row++)
  {
    albedos[row] = &albedos_1D[row * num_wavelength_bins];
  }
  i = 1;
  for (int row = 0; row < num_vertical_layers; row++)
  {
    for (int col = 0; col < num_wavelength_bins; col++)
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
  double** factors = new double*[num_vertical_layers];
  for (int row = 0; row < num_vertical_layers; row++)
  {
    factors[row] = &factors_1D[row * num_wavelength_bins];
  }
  i = 1;
  for (int row = 0; row < num_vertical_layers; row++)
  {
    for (int col = 0; col < num_wavelength_bins; col++)
    {
      factors[row][col] = 1 * i;
      i++;
    }
  }
  SetRadiatorAsymmetryFactors(foo_radiator, factors[0], num_vertical_layers, num_wavelength_bins, num_streams, &error);
  ASSERT_TRUE(IsSuccess(error));

  // Test for optical depths
  for (int row = 0; row < num_vertical_layers; row++)
  {
    for (int col = 0; col < num_wavelength_bins; col++)
    {
      optical_depths[row][col] = -999.0;
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
  for (int row = 0; row < num_vertical_layers; row++)
  {
    for (int col = 0; col < num_wavelength_bins; col++)
    {
      albedos[row][col] = -999.0;
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
  for (int row = 0; row < num_vertical_layers; row++)
  {
    for (int col = 0; col < num_wavelength_bins; col++)
    {
      factors[row][col] = -999.0;
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

  // Test copy for radiator map
  Radiator* foo_copy = GetRadiator(radiator_map, "foo", &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(foo_copy, nullptr);
  GetRadiatorOpticalDepths(foo_copy, optical_depths[0], num_vertical_layers, num_wavelength_bins, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(optical_depths[0][0], 10.0);
  ASSERT_EQ(optical_depths[0][1], 20.0);
  ASSERT_EQ(optical_depths[1][0], 30.0);
  ASSERT_EQ(optical_depths[1][1], 40.0);
  ASSERT_EQ(optical_depths[2][0], 50.0);
  ASSERT_EQ(optical_depths[2][1], 60.0);
  GetRadiatorSingleScatteringAlbedos(foo_copy, albedos[0], num_vertical_layers, num_wavelength_bins, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(albedos[0][0], 100.0);
  ASSERT_EQ(albedos[0][1], 200.0);
  ASSERT_EQ(albedos[1][0], 300.0);
  ASSERT_EQ(albedos[1][1], 400.0);
  ASSERT_EQ(albedos[2][0], 500.0);
  ASSERT_EQ(albedos[2][1], 600.0);
  GetRadiatorAsymmetryFactors(foo_copy, factors[0], num_vertical_layers, num_wavelength_bins, 1, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(factors[0][0], 1);
  ASSERT_EQ(factors[0][1], 2);
  ASSERT_EQ(factors[1][0], 3);
  ASSERT_EQ(factors[1][1], 4);
  ASSERT_EQ(factors[2][0], 5);
  ASSERT_EQ(factors[2][1], 6);

  // Clean up
  DeleteRadiator(foo_radiator, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteRadiator(bar_radiator, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteRadiator(foo_copy, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteRadiatorMap(radiator_map, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteGrid(height, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteGrid(wavelength, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteGrid(bar_height, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteGrid(bar_wavelength, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteError(&error);
  delete[] optical_depths;
  delete[] optical_depths_1D;
  delete[] albedos;
  delete[] albedos_1D;
  delete[] factors;
  delete[] factors_1D;
}