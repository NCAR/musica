#include <musica/micm.hpp>
#include <musica/util.hpp>

#include <micm/util/error.hpp>

#include <gtest/gtest.h>

#include <iostream>
#include <stdlib.h>

using namespace musica;

// Test fixture for the MICM C API
class MicmCApiTest : public ::testing::Test
{
 protected:
  MICM* micm;
  const char* config_path = "configs/chapman";
  int num_grid_cells = 1;

  void SetUp() override
  {
    micm = nullptr;
    Error error;
    micm = CreateMicm(config_path, MICMSolver::Rosenbrock, num_grid_cells, &error);

    ASSERT_TRUE(IsSuccess(error));
    DeleteError(&error);
  }

  void TearDown() override
  {
    Error error;
    DeleteMicm(micm, &error);
    ASSERT_TRUE(IsSuccess(error));
    DeleteError(&error);
  }
};

// Test case for bad configuration file path
TEST_F(MicmCApiTest, BadConfigurationFilePath)
{
  int num_grid_cells = 1;
  Error error = NoError();
  auto micm_bad_config = CreateMicm("bad config path", MICMSolver::Rosenbrock, num_grid_cells, &error);
  ASSERT_EQ(micm_bad_config, nullptr);
  ASSERT_TRUE(IsError(error, MICM_ERROR_CATEGORY_CONFIGURATION, MICM_CONFIGURATION_ERROR_CODE_INVALID_FILE_PATH));
  DeleteError(&error);
}

// Test case for bad input for solver type
TEST_F(MicmCApiTest, BadSolverType)
{
  short solver_type = 999;
  int num_grid_cells = 1;
  Error error = NoError();
  auto micm_bad_solver_type = CreateMicm("configs/chapman", static_cast<MICMSolver>(solver_type), num_grid_cells, &error);
  ASSERT_EQ(micm_bad_solver_type, nullptr);
  ASSERT_TRUE(IsError(error, MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND));
  DeleteError(&error);
}

// Test case for missing species property
TEST_F(MicmCApiTest, MissingSpeciesProperty)
{
  Error error = NoError();
  String string_value;
  string_value = GetSpeciesPropertyString(micm, "O3", "bad property", &error);
  ASSERT_TRUE(IsError(error, MICM_ERROR_CATEGORY_SPECIES, MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND));
  ASSERT_STREQ(string_value.value_, nullptr);
  DeleteString(&string_value);
  DeleteError(&error);
  error = NoError();
  ASSERT_EQ(GetSpeciesPropertyDouble(micm, "O3", "bad property", &error), 0.0);
  ASSERT_TRUE(IsError(error, MICM_ERROR_CATEGORY_SPECIES, MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND));
  DeleteError(&error);
  error = NoError();
  ASSERT_EQ(GetSpeciesPropertyInt(micm, "O3", "bad property", &error), 0);
  ASSERT_TRUE(IsError(error, MICM_ERROR_CATEGORY_SPECIES, MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND));
  DeleteError(&error);
  error = NoError();
  ASSERT_FALSE(GetSpeciesPropertyBool(micm, "O3", "bad property", &error));
  ASSERT_TRUE(IsError(error, MICM_ERROR_CATEGORY_SPECIES, MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND));
  DeleteError(&error);
}

// Test case for creating the MICM instance
TEST_F(MicmCApiTest, CreateMicmInstance)
{
  ASSERT_NE(micm, nullptr);
}

// Test case for getting species ordering
TEST_F(MicmCApiTest, GetSpeciesOrdering)
{
  Error error;
  std::size_t array_size;
  Mapping* species_ordering = GetSpeciesOrdering(micm, &array_size, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteError(&error);
  ASSERT_EQ(array_size, 4);
  bool found = false;
  for (std::size_t i = 0; i < array_size; i++)
  {
    if (strcmp(species_ordering[i].name_.value_, "O3") == 0)
    {
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
  found = false;
  for (std::size_t i = 0; i < array_size; i++)
  {
    if (strcmp(species_ordering[i].name_.value_, "O") == 0)
    {
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
  found = false;
  for (std::size_t i = 0; i < array_size; i++)
  {
    if (strcmp(species_ordering[i].name_.value_, "O2") == 0)
    {
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
  found = false;
  for (std::size_t i = 0; i < array_size; i++)
  {
    if (strcmp(species_ordering[i].name_.value_, "O1D") == 0)
    {
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
  DeleteMappings(species_ordering, array_size);
}

// Test case for getting user-defined reaction rates ordering
TEST_F(MicmCApiTest, GetUserDefinedReactionRatesOrdering)
{
  Error error;
  std::size_t array_size;
  Mapping* reaction_rates_ordering = GetUserDefinedReactionRatesOrdering(micm, &array_size, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteError(&error);
  ASSERT_EQ(array_size, 3);
  bool found = false;
  for (std::size_t i = 0; i < array_size; i++)
  {
    if (strcmp(reaction_rates_ordering[i].name_.value_, "PHOTO.jO2") == 0)
    {
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
  found = false;
  for (std::size_t i = 0; i < array_size; i++)
  {
    if (strcmp(reaction_rates_ordering[i].name_.value_, "PHOTO.jO3->O") == 0)
    {
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
  found = false;
  for (std::size_t i = 0; i < array_size; i++)
  {
    if (strcmp(reaction_rates_ordering[i].name_.value_, "PHOTO.jO3->O1D") == 0)
    {
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
  DeleteMappings(reaction_rates_ordering, array_size);
}

void TestSingleGridCell(MICM* micm)
{
  double time_step = 200.0;
  double temperatures[1];
  double pressures[1];
  double air_densities[1];
  constexpr double GAS_CONSTANT = 8.31446261815324;  // J mol-1 K-1
  const std::size_t num_concentrations = 4;
  double concentrations[num_concentrations];
  const std::size_t num_user_defined_reaction_rates = 3;
  double user_defined_reaction_rates[num_user_defined_reaction_rates];
  std::size_t temp_size;
  Error error;

  // Get species ordering
  Mapping* species_ordering = GetSpeciesOrdering(micm, &temp_size, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(temp_size, num_concentrations);
  std::size_t O2_index = FindMappingIndex(species_ordering, num_concentrations, "O2", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t O_index = FindMappingIndex(species_ordering, num_concentrations, "O", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t O1D_index = FindMappingIndex(species_ordering, num_concentrations, "O1D", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t O3_index = FindMappingIndex(species_ordering, num_concentrations, "O3", &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteMappings(species_ordering, num_concentrations);

  // Get user-defined reaction rates ordering
  Mapping* reaction_rates_ordering = GetUserDefinedReactionRatesOrdering(micm, &temp_size, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(temp_size, num_user_defined_reaction_rates);
  std::size_t jO2_index = FindMappingIndex(reaction_rates_ordering, num_user_defined_reaction_rates, "PHOTO.jO2", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t jO3_O_index =
      FindMappingIndex(reaction_rates_ordering, num_user_defined_reaction_rates, "PHOTO.jO3->O", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t jO3_O1D_index =
      FindMappingIndex(reaction_rates_ordering, num_user_defined_reaction_rates, "PHOTO.jO3->O1D", &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteMappings(reaction_rates_ordering, num_user_defined_reaction_rates);

  temperatures[0] = 272.5;
  pressures[0] = 101253.4;
  air_densities[0] = pressures[0] / (GAS_CONSTANT * temperatures[0]);
  concentrations[O2_index] = 0.75;
  concentrations[O_index] = 0.0;
  concentrations[O1D_index] = 0.0;
  concentrations[O3_index] = 0.0000081;
  user_defined_reaction_rates[jO2_index] = 2.7e-19;
  user_defined_reaction_rates[jO3_O_index] = 1.13e-9;
  user_defined_reaction_rates[jO3_O1D_index] = 5.8e-8;

  String solver_state;
  SolverResultStats solver_stats;
  MicmSolve(
      micm,
      time_step,
      temperatures,
      pressures,
      air_densities,
      concentrations,
      user_defined_reaction_rates,
      &solver_state,
      &solver_stats,
      &error);
  ASSERT_TRUE(IsSuccess(error));

  // Add assertions to check the solved concentrations
  ASSERT_NEAR(concentrations[O2_index], 0.75, 1.0e-6);
  ASSERT_GT(concentrations[O_index], 0.0);
  ASSERT_GT(concentrations[O1D_index], 0.0);
  ASSERT_NE(concentrations[O3_index], 0.0000081);

  std::cout << "Solver state: " << solver_state.value_ << std::endl;
  ASSERT_STREQ(solver_state.value_, "Converged");
  std::cout << "Function Calls: " << solver_stats.function_calls_ << std::endl;
  std::cout << "Jacobian updates: " << solver_stats.jacobian_updates_ << std::endl;
  std::cout << "Number of steps: " << solver_stats.number_of_steps_ << std::endl;
  std::cout << "Accepted: " << solver_stats.accepted_ << std::endl;
  std::cout << "Rejected: " << solver_stats.rejected_ << std::endl;
  std::cout << "Decompositions: " << solver_stats.decompositions_ << std::endl;
  std::cout << "Solves: " << solver_stats.solves_ << std::endl;
  std::cout << "Singular: " << solver_stats.singular_ << std::endl;
  std::cout << "Final time: " << solver_stats.final_time_ << std::endl;

  DeleteString(&solver_state);
  DeleteError(&error);
}

// Test case for solving system using vector-ordered Rosenbrock solver
TEST_F(MicmCApiTest, SolveUsingVectorOrderedRosenbrock)
{
  TestSingleGridCell(micm);
}

// Test case for solving system using standard-ordered Rosenbrock solver
TEST(RosenbrockStandardOrder, SolveUsingStandardOrderedRosenbrock)
{
  const char* config_path = "configs/chapman";
  int num_grid_cells = 1;
  Error error;
  MICM* micm = CreateMicm(config_path, MICMSolver::RosenbrockStandardOrder, num_grid_cells, &error);

  TestSingleGridCell(micm);

  DeleteMicm(micm, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteError(&error);
}

// Test case for solving system using standard-ordered Backward Euler solver
TEST(BackwardEulerStandardOrder, SolveUsingStandardOrderedBackwardEuler)
{
  const char* config_path = "configs/chapman";
  int num_grid_cells = 1;
  Error error;
  MICM* micm = CreateMicm(config_path, MICMSolver::BackwardEulerStandardOrder, num_grid_cells, &error);

  TestSingleGridCell(micm);

  DeleteMicm(micm, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteError(&error);
}

struct ArrheniusReaction
{
  double A_{ 1 };
  double B_{ 0 };
  double C_{ 0 };
  double D_{ 300 };
  double E_{ 0 };
};

double CalculateArrhenius(const ArrheniusReaction parameters, const double temperature, const double pressure)
{
  return parameters.A_ * std::exp(parameters.C_ / temperature) * std::pow(temperature / parameters.D_, parameters.B_) *
         (1.0 + parameters.E_ * pressure);
}

// Common test function for solving multiple grid cells
void TestMultipleGridCells(MICM* micm, const size_t num_grid_cells)
{
  const size_t num_concentrations = 6;
  const size_t num_user_defined_reaction_rates = 2;
  constexpr double GAS_CONSTANT = 8.31446261815324;  // J mol-1 K-1
  const double time_step = 200.0;                    // s

  double* temperature = new double[num_grid_cells];
  double* pressure = new double[num_grid_cells];
  double* air_density = new double[num_grid_cells];
  double* concentrations = new double[num_grid_cells * num_concentrations];
  double* initial_concentrations = new double[num_grid_cells * num_concentrations];
  double* user_defined_reaction_rates = new double[num_grid_cells * num_user_defined_reaction_rates];

  Error error;
  size_t temp_size;

  // Get species indices in concentration array
  Mapping* species_ordering = GetSpeciesOrdering(micm, &temp_size, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(temp_size, num_concentrations);
  std::size_t A_index = FindMappingIndex(species_ordering, num_concentrations, "A", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t B_index = FindMappingIndex(species_ordering, num_concentrations, "B", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t C_index = FindMappingIndex(species_ordering, num_concentrations, "C", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t D_index = FindMappingIndex(species_ordering, num_concentrations, "D", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t E_index = FindMappingIndex(species_ordering, num_concentrations, "E", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t F_index = FindMappingIndex(species_ordering, num_concentrations, "F", &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteMappings(species_ordering, num_concentrations);

  // Get user-defined reaction rates indices in user-defined reaction rates array
  Mapping* rate_ordering = GetUserDefinedReactionRatesOrdering(micm, &temp_size, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(temp_size, num_user_defined_reaction_rates);
  std::size_t R1_index = FindMappingIndex(rate_ordering, num_user_defined_reaction_rates, "USER.reaction 1", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t R2_index = FindMappingIndex(rate_ordering, num_user_defined_reaction_rates, "USER.reaction 2", &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteMappings(rate_ordering, num_user_defined_reaction_rates);

  for (int i = 0; i < num_grid_cells; ++i)
  {
    temperature[i] = 275.0 + (rand() % 20 - 10);
    pressure[i] = 101253.3 + (rand() % 1000 - 500);
    air_density[i] = pressure[i] / (GAS_CONSTANT * temperature[i]);
    concentrations[i * num_concentrations + A_index] = 0.75 + (rand() % 10 - 5) * 0.01;
    concentrations[i * num_concentrations + B_index] = 0.0;
    concentrations[i * num_concentrations + C_index] = 0.4 + (rand() % 10 - 5) * 0.01;
    concentrations[i * num_concentrations + D_index] = 0.8 + (rand() % 10 - 5) * 0.01;
    concentrations[i * num_concentrations + E_index] = 0.0;
    concentrations[i * num_concentrations + F_index] = 0.1 + (rand() % 10 - 5) * 0.01;
    user_defined_reaction_rates[i * num_user_defined_reaction_rates + R1_index] = 0.001 + (rand() % 10 - 5) * 0.0001;
    user_defined_reaction_rates[i * num_user_defined_reaction_rates + R2_index] = 0.002 + (rand() % 10 - 5) * 0.0001;
    for (int j = 0; j < num_concentrations; ++j)
    {
      initial_concentrations[i * num_concentrations + j] = concentrations[i * num_concentrations + j];
    }
  }

  String solver_state;
  SolverResultStats solver_stats;
  MicmSolve(
      micm,
      time_step,
      temperature,
      pressure,
      air_density,
      concentrations,
      user_defined_reaction_rates,
      &solver_state,
      &solver_stats,
      &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteError(&error);

  // Add assertions to check the solved concentrations
  ArrheniusReaction arr1;
  arr1.A_ = 0.004;
  arr1.C_ = 50.0;
  ArrheniusReaction arr2{ 0.012, -2, 75, 50, 1.0e-6 };

  ASSERT_STREQ(solver_state.value_, "Converged");
  DeleteString(&solver_state);

  for (int i_cell = 0; i_cell < num_grid_cells; ++i_cell)
  {
    double initial_A = initial_concentrations[i_cell * num_concentrations + A_index];
    double initial_C = initial_concentrations[i_cell * num_concentrations + C_index];
    double initial_D = initial_concentrations[i_cell * num_concentrations + D_index];
    double initial_F = initial_concentrations[i_cell * num_concentrations + F_index];
    double k1 = user_defined_reaction_rates[i_cell * num_user_defined_reaction_rates + R1_index];
    double k2 = user_defined_reaction_rates[i_cell * num_user_defined_reaction_rates + R2_index];
    double k3 = CalculateArrhenius(arr1, temperature[i_cell], pressure[i_cell]);
    double k4 = CalculateArrhenius(arr2, temperature[i_cell], pressure[i_cell]);
    double A = initial_A * std::exp(-k3 * time_step);
    double B = initial_A * (k3 / (k4 - k3)) * (std::exp(-k3 * time_step) - std::exp(-k4 * time_step));
    double C = initial_C + initial_A * (1.0 + (k3 * std::exp(-k4 * time_step) - k4 * std::exp(-k3 * time_step)) / (k4 - k3));
    double D = initial_D * std::exp(-k1 * time_step);
    double E = initial_D * (k1 / (k2 - k1)) * (std::exp(-k1 * time_step) - std::exp(-k2 * time_step));
    double F = initial_F + initial_D * (1.0 + (k1 * std::exp(-k2 * time_step) - k2 * std::exp(-k1 * time_step)) / (k2 - k1));
    ASSERT_NEAR(concentrations[i_cell * num_concentrations + A_index], A, 5e-3);
    ASSERT_NEAR(concentrations[i_cell * num_concentrations + B_index], B, 5e-3);
    ASSERT_NEAR(concentrations[i_cell * num_concentrations + C_index], C, 5e-3);
    ASSERT_NEAR(concentrations[i_cell * num_concentrations + D_index], D, 5e-3);
    ASSERT_NEAR(concentrations[i_cell * num_concentrations + E_index], E, 5e-3);
    ASSERT_NEAR(concentrations[i_cell * num_concentrations + F_index], F, 5e-3);
  }
  delete[] temperature;
  delete[] pressure;
  delete[] air_density;
  delete[] concentrations;
  delete[] initial_concentrations;
  delete[] user_defined_reaction_rates;
}

// Test case for solving multiple grid cells using vector-ordered Rosenbrock solver
TEST_F(MicmCApiTest, SolveMultipleGridCellsUsingVectorOrderedRosenbrock)
{
  constexpr size_t num_grid_cells = 3;
  const char* config_path = "configs/analytical";
  Error error;
  DeleteMicm(micm, &error);
  ASSERT_TRUE(IsSuccess(error));
  micm = CreateMicm(config_path, MICMSolver::Rosenbrock, num_grid_cells, &error);
  ASSERT_TRUE(IsSuccess(error));
  TestMultipleGridCells(micm, num_grid_cells);
  DeleteError(&error);
}

// Test case for solving multiple grid cells using standard-ordered Rosenbrock solver
TEST_F(MicmCApiTest, SolveMultipleGridCellsUsingStandardOrderedRosenbrock)
{
  constexpr size_t num_grid_cells = 3;
  const char* config_path = "configs/analytical";
  Error error;
  DeleteMicm(micm, &error);
  ASSERT_TRUE(IsSuccess(error));
  micm = CreateMicm(config_path, MICMSolver::RosenbrockStandardOrder, num_grid_cells, &error);
  ASSERT_TRUE(IsSuccess(error));
  TestMultipleGridCells(micm, num_grid_cells);
  DeleteError(&error);
}

// Test case for getting species properties
TEST_F(MicmCApiTest, GetSpeciesProperty)
{
  Error error;
  String string_value;
  string_value = GetSpeciesPropertyString(micm, "O3", "__long name", &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_STREQ(string_value.value_, "ozone");
  DeleteString(&string_value);
  ASSERT_EQ(GetSpeciesPropertyDouble(micm, "O3", "molecular weight [kg mol-1]", &error), 0.048);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_TRUE(GetSpeciesPropertyBool(micm, "O3", "__do advect", &error));
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(GetSpeciesPropertyInt(micm, "O3", "__atoms", &error), 3);
  ASSERT_TRUE(IsSuccess(error));
  GetSpeciesPropertyBool(micm, "bad species", "__is gas", &error);
  ASSERT_TRUE(IsError(error, MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SPECIES_NOT_FOUND));
  GetSpeciesPropertyDouble(micm, "O3", "bad property", &error);
  ASSERT_TRUE(IsError(error, MICM_ERROR_CATEGORY_SPECIES, MICM_SPECIES_ERROR_CODE_PROPERTY_NOT_FOUND));
  DeleteError(&error);
}
