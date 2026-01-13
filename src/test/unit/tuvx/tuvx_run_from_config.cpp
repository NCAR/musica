#include <musica/tuvx/tuvx.hpp>
#include <musica/tuvx/tuvx_c_interface.hpp>

#include <gtest/gtest.h>

using namespace musica;

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

static void tuvx_log(const std::string& msg)
{
  using namespace std::chrono;
  auto now = system_clock::now();
  auto t = system_clock::to_time_t(now);
  long ms = duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000;
  std::tm tm = *std::localtime(&t);
  std::cout << "[tuvx] [" << std::put_time(&tm, "%F %T") << "." << std::setfill('0') << std::setw(3) << ms << "] " << msg
            << std::endl;
}

// Expected values for photolysis rate constants and heating rates
// were determined by running the stand-alone TUV-x model with the fixed configuration.
const double expected_photolysis_rate_constants[3][4] = {
  { 8.91393763338872e-28, 1.64258192104497e-20, 8.48391527327371e-14, 9.87420948924703e-08 },
  { 2.49575956372508e-27, 4.58686176250519e-20, 2.22679622672858e-13, 2.29392676897831e-07 },
  { 1.78278752667774e-27, 3.28516384208994e-20, 1.69678305465474e-13, 1.97484189784941e-07 }
};
const double expected_heating_rates[2][4] = {
  { 1.12394047546984e-46, 2.04518267143613e-39, 7.44349752571804e-33, 5.42628100199216e-28 },
  { 5.14970120496081e-46, 9.37067648164478e-39, 3.41659389501112e-32, 5.46672356294259e-27 }
};

// Test fixture for calculating photolysis rate constants
// using the TUVX C API with a fixed configuration file
class TuvxRunTest : public ::testing::Test
{
 protected:
  TUVX* tuvx;
  GridMap* grids_from_host;
  ProfileMap* profiles_from_host;
  RadiatorMap* radiators_from_host;
  GridMap* grids_in_tuvx;
  ProfileMap* profiles_in_tuvx;
  RadiatorMap* radiators_in_tuvx;
  int number_of_layers;
  int number_of_wavelengths;
  int number_of_reactions;
  int number_of_heating_rates;
  double* photolysis_rate_constants;
  double* heating_rates;

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
    number_of_layers = 0;
    number_of_wavelengths = 0;
    number_of_reactions = 0;
    number_of_heating_rates = 0;
    photolysis_rate_constants = nullptr;
    heating_rates = nullptr;
    tuvx_log("SetUp(): initialized members");
  }

  void SetUp(const char* config_path)
  {
    Error error;
    tuvx_log(std::string("SetUp(config): starting for config: ") + (config_path ? config_path : "(null)"));
    grids_from_host = CreateGridMap(&error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("SetUp(config): CreateGridMap succeeded");
    profiles_from_host = CreateProfileMap(&error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("SetUp(config): CreateProfileMap succeeded");
    radiators_from_host = CreateRadiatorMap(&error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("SetUp(config): CreateRadiatorMap succeeded");
    tuvx = CreateTuvx(config_path, grids_from_host, profiles_from_host, radiators_from_host, &error);
    if (!IsSuccess(error))
    {
      std::cerr << "Error creating TUVX instance: " << error.message_.value_ << std::endl;
      tuvx_log(
          std::string("SetUp(config): CreateTuvx failed: ") +
          (error.message_.value_ ? error.message_.value_ : "(no message)"));
    }
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("SetUp(config): CreateTuvx succeeded");
    grids_in_tuvx = GetGridMap(tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("SetUp(config): GetGridMap succeeded");
    profiles_in_tuvx = GetProfileMap(tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("SetUp(config): GetProfileMap succeeded");
    radiators_in_tuvx = GetRadiatorMap(tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("SetUp(config): GetRadiatorMap succeeded");
    number_of_layers = 3;
    number_of_wavelengths = 5;
    number_of_reactions = 3;
    number_of_heating_rates = 2;
    photolysis_rate_constants = new double[(number_of_layers + 1) * number_of_reactions];
    heating_rates = new double[(number_of_layers + 1) * number_of_heating_rates];
    tuvx_log("SetUp(config): allocated photolysis_rate_constants and heating_rates");
    DeleteError(&error);
  }

  void SetUp(const char* config_path, GridMap* grids, ProfileMap* profiles, RadiatorMap* radiators)
  {
    Error error;
    grids_from_host = grids;
    profiles_from_host = profiles;
    radiators_from_host = radiators;
    tuvx_log(std::string("SetUp(config,host): starting for config: ") + (config_path ? config_path : "(null)"));
    tuvx = CreateTuvx(config_path, grids, profiles, radiators, &error);
    if (!IsSuccess(error))
    {
      std::cerr << "Error creating TUVX instance: " << error.message_.value_ << std::endl;
      tuvx_log(
          std::string("SetUp(config,host): CreateTuvx failed: ") +
          (error.message_.value_ ? error.message_.value_ : "(no message)"));
    }
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("SetUp(config,host): CreateTuvx succeeded");
    grids_in_tuvx = GetGridMap(tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("SetUp(config,host): GetGridMap succeeded");
    profiles_in_tuvx = GetProfileMap(tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("SetUp(config,host): GetProfileMap succeeded");
    radiators_in_tuvx = GetRadiatorMap(tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("SetUp(config,host): GetRadiatorMap succeeded");
    number_of_layers = 3;
    number_of_wavelengths = 5;
    number_of_reactions = 3;
    number_of_heating_rates = 2;
    photolysis_rate_constants = new double[(number_of_layers + 1) * number_of_reactions];
    heating_rates = new double[(number_of_layers + 1) * number_of_heating_rates];
    tuvx_log("SetUp(config,host): allocated photolysis_rate_constants and heating_rates");
    DeleteError(&error);
  }

  void TearDown() override
  {
    if (tuvx == nullptr)
    {
      return;
    }
    tuvx_log("TearDown(): starting cleanup");
    Error error;
    DeleteTuvx(tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("TearDown(): DeleteTuvx succeeded");
    DeleteGridMap(grids_from_host, &error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("TearDown(): DeleteGridMap(grids_from_host) succeeded");
    DeleteProfileMap(profiles_from_host, &error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("TearDown(): DeleteProfileMap(profiles_from_host) succeeded");
    DeleteRadiatorMap(radiators_from_host, &error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("TearDown(): DeleteRadiatorMap(radiators_from_host) succeeded");
    DeleteGridMap(grids_in_tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("TearDown(): DeleteGridMap(grids_in_tuvx) succeeded");
    DeleteProfileMap(profiles_in_tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("TearDown(): DeleteProfileMap(profiles_in_tuvx) succeeded");
    DeleteRadiatorMap(radiators_in_tuvx, &error);
    ASSERT_TRUE(IsSuccess(error));
    tuvx_log("TearDown(): DeleteRadiatorMap(radiators_in_tuvx) succeeded");
    delete[] photolysis_rate_constants;
    delete[] heating_rates;
    DeleteError(&error);
    tuvx_log("TearDown(): finished cleanup");
  }
};

TEST_F(TuvxRunTest, CreateTuvxInstanceWithJsonConfig)
{
  const char* json_config_path = "configs/tuvx/fixed/config.json";
  tuvx_log(std::string("Test CreateTuvxInstanceWithJsonConfig: calling SetUp with ") + json_config_path);
  SetUp(json_config_path);
  ASSERT_NE(tuvx, nullptr);
  Error error;
  tuvx_log("Test CreateTuvxInstanceWithJsonConfig: calling RunTuvx");
  RunTuvx(tuvx, 0.1, 1.1, photolysis_rate_constants, heating_rates, nullptr, &error);
  tuvx_log("Test CreateTuvxInstanceWithJsonConfig: RunTuvx returned");
  ASSERT_TRUE(IsSuccess(error));
  for (int i = 0; i < number_of_reactions; i++)
  {
    for (int j = 0; j < number_of_layers + 1; j++)
    {
      EXPECT_NEAR(
          photolysis_rate_constants[i * (number_of_layers + 1) + j],
          expected_photolysis_rate_constants[i][j],
          expected_photolysis_rate_constants[i][j] * 1.0e-5);
    }
  }
  for (int i = 0; i < number_of_heating_rates; i++)
  {
    for (int j = 0; j < number_of_layers + 1; j++)
    {
      EXPECT_NEAR(
          heating_rates[i * (number_of_layers + 1) + j],
          expected_heating_rates[i][j],
          expected_heating_rates[i][j] * 1.0e-5);
    }
  }
  Mappings photo_rate_labels;
  GetPhotolysisRateConstantsOrdering(tuvx, &photo_rate_labels, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(photo_rate_labels.size_, 3);
  ASSERT_STREQ(photo_rate_labels.mappings_[0].name_.value_, "jfoo");
  ASSERT_EQ(photo_rate_labels.mappings_[0].index_, 0);
  ASSERT_STREQ(photo_rate_labels.mappings_[1].name_.value_, "jbar");
  ASSERT_EQ(photo_rate_labels.mappings_[1].index_, 1);
  ASSERT_STREQ(photo_rate_labels.mappings_[2].name_.value_, "jbaz");
  ASSERT_EQ(photo_rate_labels.mappings_[2].index_, 2);
  Mappings heating_rate_labels;
  GetHeatingRatesOrdering(tuvx, &heating_rate_labels, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(heating_rate_labels.size_, 2);
  ASSERT_STREQ(heating_rate_labels.mappings_[0].name_.value_, "jfoo");
  ASSERT_EQ(heating_rate_labels.mappings_[0].index_, 0);
  ASSERT_STREQ(heating_rate_labels.mappings_[1].name_.value_, "jbar");
  ASSERT_EQ(heating_rate_labels.mappings_[1].index_, 1);
  DeleteMappings(&photo_rate_labels);
  DeleteMappings(&heating_rate_labels);
  DeleteError(&error);
}

TEST_F(TuvxRunTest, CreateTuvxInstanceWithJsonConfigAndHostData)
{
  const char* json_config_path = "configs/tuvx/from_host/config.json";
  Error error;
  tuvx_log(
      std::string("Test CreateTuvxInstanceWithJsonConfigAndHostData: preparing host grids/profiles for ") +
      json_config_path);
  GridMap* grids = CreateGridMap(&error);
  ASSERT_TRUE(IsSuccess(error));
  tuvx_log("Created host GridMap");
  ProfileMap* profiles = CreateProfileMap(&error);
  ASSERT_TRUE(IsSuccess(error));
  tuvx_log("Created host ProfileMap");
  RadiatorMap* radiators = CreateRadiatorMap(&error);
  Grid* heights = CreateGrid("height", "km", 3, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(heights, nullptr);
  tuvx_log("Created host Grid 'height'");
  double height_edges[4] = { 0.0, 1.0, 2.0, 3.0 };
  SetGridEdges(heights, height_edges, 4, &error);
  ASSERT_TRUE(IsSuccess(error));
  double height_midpoints[3] = { 0.5, 1.5, 2.5 };
  SetGridMidpoints(heights, height_midpoints, 3, &error);
  ASSERT_TRUE(IsSuccess(error));
  AddGrid(grids, heights, &error);
  tuvx_log("Added 'height' grid to host GridMap");
  Grid* wavelengths = CreateGrid("wavelength", "nm", 5, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(wavelengths, nullptr);
  double wavelength_edges[6] = { 300.0, 400.0, 500.0, 600.0, 700.0, 800.0 };
  double wavelength_midpoints[5] = { 350.0, 450.0, 550.0, 650.0, 750.0 };
  SetGridEdges(wavelengths, wavelength_edges, 6, &error);
  ASSERT_TRUE(IsSuccess(error));
  SetGridMidpoints(wavelengths, wavelength_midpoints, 5, &error);
  ASSERT_TRUE(IsSuccess(error));
  AddGrid(grids, wavelengths, &error);
  ASSERT_TRUE(IsSuccess(error));
  tuvx_log("Added 'wavelength' grid to host GridMap");
  Profile* temperature = CreateProfile("temperature", "K", heights, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(temperature, nullptr);
  tuvx_log("Created host Profile 'temperature'");
  AddProfile(profiles, temperature, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteProfile(temperature, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteGrid(heights, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteGrid(wavelengths, &error);
  ASSERT_TRUE(IsSuccess(error));
  tuvx_log("Calling SetUp(config, host data)");
  SetUp(json_config_path, grids, profiles, radiators);
  ASSERT_NE(tuvx, nullptr);
  tuvx_log("SetUp completed, TUVX instance created");
  heights = GetGrid(grids_in_tuvx, "height", "km", &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(heights, nullptr);
  wavelengths = GetGrid(grids_in_tuvx, "wavelength", "nm", &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(wavelengths, nullptr);
  temperature = GetProfile(profiles_in_tuvx, "temperature", "K", &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_NE(temperature, nullptr);
  double temperature_edge_values[4] = { 300.0, 275.0, 260.0, 255.0 };
  SetProfileEdgeValues(temperature, temperature_edge_values, 4, &error);
  ASSERT_TRUE(IsSuccess(error));
  double temperature_midpoint_values[3] = { 287.5, 267.5, 257.5 };
  SetProfileMidpointValues(temperature, temperature_midpoint_values, 3, &error);
  ASSERT_TRUE(IsSuccess(error));
  RunTuvx(tuvx, 0.1, 1.1, photolysis_rate_constants, heating_rates, nullptr, &error);
  ASSERT_TRUE(IsSuccess(error));
  for (int i = 0; i < number_of_reactions; i++)
  {
    for (int j = 0; j < number_of_layers + 1; j++)
    {
      EXPECT_NEAR(
          photolysis_rate_constants[i * (number_of_layers + 1) + j],
          expected_photolysis_rate_constants[i][j],
          expected_photolysis_rate_constants[i][j] * 1.0e-5);
    }
  }
  for (int i = 0; i < number_of_heating_rates; i++)
  {
    for (int j = 0; j < number_of_layers + 1; j++)
    {
      EXPECT_NEAR(
          heating_rates[i * (number_of_layers + 1) + j],
          expected_heating_rates[i][j],
          expected_heating_rates[i][j] * 1.0e-5);
    }
  }
  Mappings photo_rate_labels;
  GetPhotolysisRateConstantsOrdering(tuvx, &photo_rate_labels, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(photo_rate_labels.size_, 3);
  ASSERT_STREQ(photo_rate_labels.mappings_[0].name_.value_, "jfoo");
  ASSERT_EQ(photo_rate_labels.mappings_[0].index_, 0);
  ASSERT_STREQ(photo_rate_labels.mappings_[1].name_.value_, "jbar");
  ASSERT_EQ(photo_rate_labels.mappings_[1].index_, 1);
  ASSERT_STREQ(photo_rate_labels.mappings_[2].name_.value_, "jbaz");
  ASSERT_EQ(photo_rate_labels.mappings_[2].index_, 2);
  Mappings heating_rate_labels;
  GetHeatingRatesOrdering(tuvx, &heating_rate_labels, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(heating_rate_labels.size_, 2);
  ASSERT_STREQ(heating_rate_labels.mappings_[0].name_.value_, "jfoo");
  ASSERT_EQ(heating_rate_labels.mappings_[0].index_, 0);
  ASSERT_STREQ(heating_rate_labels.mappings_[1].name_.value_, "jbar");
  ASSERT_EQ(heating_rate_labels.mappings_[1].index_, 1);
  DeleteMappings(&photo_rate_labels);
  DeleteMappings(&heating_rate_labels);
  GetGridEdges(heights, height_edges, 4, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(height_edges[0], 0.0);
  ASSERT_EQ(height_edges[1], 1.0);
  ASSERT_EQ(height_edges[2], 2.0);
  ASSERT_EQ(height_edges[3], 3.0);
  GetGridMidpoints(heights, height_midpoints, 3, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(height_midpoints[0], 0.5);
  ASSERT_EQ(height_midpoints[1], 1.5);
  ASSERT_EQ(height_midpoints[2], 2.5);
  GetGridEdges(wavelengths, wavelength_edges, 6, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(wavelength_edges[0], 300.0);
  ASSERT_EQ(wavelength_edges[1], 400.0);
  ASSERT_EQ(wavelength_edges[2], 500.0);
  ASSERT_EQ(wavelength_edges[3], 600.0);
  ASSERT_EQ(wavelength_edges[4], 700.0);
  ASSERT_EQ(wavelength_edges[5], 800.0);
  GetGridMidpoints(wavelengths, wavelength_midpoints, 5, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(wavelength_midpoints[0], 350.0);
  ASSERT_EQ(wavelength_midpoints[1], 450.0);
  ASSERT_EQ(wavelength_midpoints[2], 550.0);
  ASSERT_EQ(wavelength_midpoints[3], 650.0);
  ASSERT_EQ(wavelength_midpoints[4], 750.0);
  GetProfileEdgeValues(temperature, temperature_edge_values, 4, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(temperature_edge_values[0], 300.0);
  ASSERT_EQ(temperature_edge_values[1], 275.0);
  ASSERT_EQ(temperature_edge_values[2], 260.0);
  ASSERT_EQ(temperature_edge_values[3], 255.0);
  GetProfileMidpointValues(temperature, temperature_midpoint_values, 3, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(temperature_midpoint_values[0], 287.5);
  ASSERT_EQ(temperature_midpoint_values[1], 267.5);
  ASSERT_EQ(temperature_midpoint_values[2], 257.5);
  DeleteGrid(heights, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteGrid(wavelengths, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteProfile(temperature, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteError(&error);
}