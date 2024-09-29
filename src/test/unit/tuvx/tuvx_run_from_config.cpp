#include <musica/tuvx/tuvx.hpp>

#include <gtest/gtest.h>

using namespace musica;


// Expected values for photolysis rate constants and heating rates
// were determined by running the stand-alone TUV-x model with the fixed configuration.
const double expected_photolysis_rate_constants[2][4] = {
  { 8.91393763338872e-28, 1.64258192104497e-20, 8.48391527327371e-14, 9.87420948924703e-08 },
  { 2.49575956372508e-27, 4.58686176250519e-20, 2.22679622672858e-13, 2.29392676897831e-07 }
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
    double *photolysis_rate_constants;
    double *heating_rates;
  
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
      photolysis_rate_constants = nullptr;
      heating_rates = nullptr;
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
      number_of_layers = 3;
      number_of_wavelengths = 5;
      number_of_reactions = 2;
      photolysis_rate_constants = new double[(number_of_layers + 1) * number_of_reactions];
      heating_rates = new double[(number_of_layers + 1) * number_of_reactions];
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
      delete[] photolysis_rate_constants;
      delete[] heating_rates;
      DeleteError(&error);
    }
};

TEST_F(TuvxRunTest, CreateTuvxInstanceWithJsonConfig)
{
  const char* json_config_path = "test/data/tuvx/fixed/config.json";
  SetUp(json_config_path);
  ASSERT_NE(tuvx, nullptr);
  Error error;
  RunTuvx(tuvx, 0.1, 1.1, photolysis_rate_constants, heating_rates, &error);
  ASSERT_TRUE(IsSuccess(error));
  for (int i = 0; i < number_of_reactions; i++)
  {
    for (int j = 0; j < number_of_layers + 1; j++)
    {
      ASSERT_NEAR(photolysis_rate_constants[i * (number_of_layers + 1) + j], expected_photolysis_rate_constants[i][j], expected_photolysis_rate_constants[i][j] * 1.0e-5);
      ASSERT_NEAR(heating_rates[i * (number_of_layers + 1) + j], expected_heating_rates[i][j], expected_heating_rates[i][j] * 1.0e-5);
    }
  }
  DeleteError(&error);
}