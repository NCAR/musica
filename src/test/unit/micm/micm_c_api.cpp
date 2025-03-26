#include <musica/micm/micm.hpp>
#include <musica/micm/state.hpp>
#include <musica/util.hpp>

#include <micm/util/error.hpp>

#include <gtest/gtest.h>

#include <iostream>
#include <stdlib.h>

using namespace musica;

// Test fixture for the MICM C API
class MicmCApiTestFixture : public ::testing::Test
{
 protected:
  MICM* micm;
  musica::State* state;
  const char* config_path = "configs/chapman";
  int num_grid_cells = 1;

  void SetUp() override
  {
    micm = nullptr;
    state = nullptr;
    Error error;
    micm = CreateMicm(config_path, MICMSolver::Rosenbrock, num_grid_cells, &error);
    state = CreateMicmState(micm, &error);

    ASSERT_TRUE(IsSuccess(error));
    DeleteError(&error);
  }

  void TearDown() override
  {
    Error error;
    DeleteMicm(micm, &error);
    DeleteState(state, &error);
    ASSERT_TRUE(IsSuccess(error));
    DeleteError(&error);
  }
};

// Test case for bad solver
TEST_F(MicmCApiTestFixture, BadSolver)
{
  MICM* micm = nullptr;
  Error error;
  auto state = CreateMicmState(micm, &error);
  ASSERT_EQ(state, nullptr);
  ASSERT_TRUE(IsError(error, MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND));
  DeleteError(&error);
}

// Test case for creating state successfully
TEST_F(MicmCApiTestFixture, CreateStateSuccess)
{
  Error error;
  musica::MICM* micm = CreateMicm(config_path, MICMSolver::Rosenbrock, num_grid_cells, &error);
  auto state = CreateMicmState(micm, &error);
  ASSERT_NE(state, nullptr);
  delete state;
  delete micm;
}

// Test case for bad configuration file path
TEST(MicmCApiTest, BadConfigurationFilePath)
{
  int num_grid_cells = 1;
  Error error = NoError();
  auto micm_bad_config = CreateMicm("bad config path", MICMSolver::Rosenbrock, num_grid_cells, &error);
  ASSERT_EQ(micm_bad_config, nullptr);
  ASSERT_TRUE(IsError(error, MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_CONFIG_PARSE_FAILED));
  DeleteError(&error);
}

// Test case for bad input for solver type
TEST(MicmCApiTest, BadSolverType)
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
TEST_F(MicmCApiTestFixture, MissingSpeciesProperty)
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
TEST_F(MicmCApiTestFixture, CreateMicmInstance)
{
  ASSERT_NE(micm, nullptr);
}

// Test case for getting species ordering
TEST_F(MicmCApiTestFixture, GetSpeciesOrdering)
{
  Error error;
  Mappings species_ordering = GetSpeciesOrdering(micm, state, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(species_ordering.size_, 4);
  DeleteError(&error);
  bool found = false;
  for (std::size_t i = 0; i < species_ordering.size_; i++)
  {
    if (strcmp(species_ordering.mappings_[i].name_.value_, "O3") == 0)
    {
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
  found = false;
  for (std::size_t i = 0; i < species_ordering.size_; i++)
  {
    if (strcmp(species_ordering.mappings_[i].name_.value_, "O") == 0)
    {
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
  found = false;
  for (std::size_t i = 0; i < species_ordering.size_; i++)
  {
    if (strcmp(species_ordering.mappings_[i].name_.value_, "O2") == 0)
    {
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
  found = false;
  for (std::size_t i = 0; i < species_ordering.size_; i++)
  {
    if (strcmp(species_ordering.mappings_[i].name_.value_, "O1D") == 0)
    {
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
  DeleteMappings(&species_ordering);
}

// Test case for getting user-defined reaction rates ordering
TEST_F(MicmCApiTestFixture, GetUserDefinedReactionRatesOrdering)
{
  Error error;
  Mappings reaction_rates_ordering = GetUserDefinedReactionRatesOrdering(micm, state, &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteError(&error);
  ASSERT_EQ(reaction_rates_ordering.size_, 3);
  bool found = false;
  for (std::size_t i = 0; i < reaction_rates_ordering.size_; i++)
  {
    if (strcmp(reaction_rates_ordering.mappings_[i].name_.value_, "PHOTO.jO2") == 0)
    {
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
  found = false;
  for (std::size_t i = 0; i < reaction_rates_ordering.size_; i++)
  {
    if (strcmp(reaction_rates_ordering.mappings_[i].name_.value_, "PHOTO.jO3->O") == 0)
    {
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
  found = false;
  for (std::size_t i = 0; i < reaction_rates_ordering.size_; i++)
  {
    if (strcmp(reaction_rates_ordering.mappings_[i].name_.value_, "PHOTO.jO3->O1D") == 0)
    {
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
  DeleteMappings(&reaction_rates_ordering);
}

void TestSingleGridCell(MICM* micm, musica::State* state)
{
  double time_step = 200.0;
  constexpr double GAS_CONSTANT = 8.31446261815324;  // J mol-1 K-1
  const std::size_t num_concentrations = 4;
  const std::size_t num_user_defined_reaction_rates = 3;
  Error error;

  std::vector<micm::Conditions> conditions(1);
  std::vector<double>& concentrations_vector = state->GetOrderedConcentrations();
  std::vector<double>& user_defined_reaction_rates = state->GetOrderedRateConstants();

  // Get species ordering
  Mappings species_ordering = GetSpeciesOrdering(micm, state, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(species_ordering.size_, num_concentrations);
  std::size_t O2_index = FindMappingIndex(species_ordering, "O2", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t O_index = FindMappingIndex(species_ordering, "O", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t O1D_index = FindMappingIndex(species_ordering, "O1D", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t O3_index = FindMappingIndex(species_ordering, "O3", &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteMappings(&species_ordering);

  // Get user-defined reaction rates ordering
  Mappings reaction_rates_ordering = GetUserDefinedReactionRatesOrdering(micm, state, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(reaction_rates_ordering.size_, num_user_defined_reaction_rates);
  std::size_t jO2_index = FindMappingIndex(reaction_rates_ordering, "PHOTO.jO2", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t jO3_O_index = FindMappingIndex(reaction_rates_ordering, "PHOTO.jO3->O", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t jO3_O1D_index = FindMappingIndex(reaction_rates_ordering, "PHOTO.jO3->O1D", &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteMappings(&reaction_rates_ordering);

  conditions[0].temperature_ = 272.5;
  conditions[0].pressure_ = 101253.4;
  conditions[0].air_density_ = conditions[0].pressure_ / (GAS_CONSTANT * conditions[0].temperature_);
  concentrations_vector[O2_index] = 0.75;
  concentrations_vector[O_index] = 0.0;
  concentrations_vector[O1D_index] = 0.0;
  concentrations_vector[O3_index] = 0.0000081;
  user_defined_reaction_rates[jO2_index] = 2.7e-19;
  user_defined_reaction_rates[jO3_O_index] = 1.13e-9;
  user_defined_reaction_rates[jO3_O1D_index] = 5.8e-8;

  state->SetConditions(conditions);

  String solver_state;
  SolverResultStats solver_stats;
  MicmSolve(micm, state, time_step, &solver_state, &solver_stats, &error);
  ASSERT_TRUE(IsSuccess(error));

  // Add assertions to check the solved concentrations
  ASSERT_NEAR(concentrations_vector[O2_index], 0.75, 1.0e-6);
  ASSERT_GT(concentrations_vector[O_index], 0.0);
  ASSERT_GT(concentrations_vector[O1D_index], 0.0);
  ASSERT_NE(concentrations_vector[O3_index], 0.0000081);

  std::cout << "Solver state: " << solver_state.value_ << std::endl;
  ASSERT_STREQ(solver_state.value_, "Converged");
  std::cout << "Function Calls: " << solver_stats.function_calls_ << std::endl;
  std::cout << "Jacobian updates: " << solver_stats.jacobian_updates_ << std::endl;
  std::cout << "Number of steps: " << solver_stats.number_of_steps_ << std::endl;
  std::cout << "Accepted: " << solver_stats.accepted_ << std::endl;
  std::cout << "Rejected: " << solver_stats.rejected_ << std::endl;
  std::cout << "Decompositions: " << solver_stats.decompositions_ << std::endl;
  std::cout << "Solves: " << solver_stats.solves_ << std::endl;
  std::cout << "Final time: " << solver_stats.final_time_ << std::endl;

  DeleteString(&solver_state);
  DeleteError(&error);
}

// Test case for solving system using standard-ordered Rosenbrock solver
TEST(RosenbrockStandardOrder, SolveUsingStandardOrderedRosenbrock)
{
  const char* config_path = "configs/chapman";
  int num_grid_cells = 1;
  Error error;
  MICM* micm = CreateMicm(config_path, MICMSolver::RosenbrockStandardOrder, num_grid_cells, &error);
  musica::State* state = CreateMicmState(micm, &error);

  TestSingleGridCell(micm, state);

  DeleteMicm(micm, &error);
  DeleteState(state, &error);
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
  musica::State* state = CreateMicmState(micm, &error);

  TestSingleGridCell(micm, state);

  DeleteMicm(micm, &error);
  DeleteState(state, &error);
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

// Common test function for solving multiple grid cells with standard-ordered matrices
void TestStandardMultipleGridCells(
    MICM* micm,
    musica::State* state,
    const size_t num_grid_cells,
    const double time_step,
    const double test_accuracy)
{
  const size_t num_concentrations = 6;
  const size_t num_user_defined_reaction_rates = 2;
  constexpr double GAS_CONSTANT = 8.31446261815324;  // J mol-1 K-1

  std::vector<micm::Conditions> conditions(num_grid_cells);
  std::vector<double>& concentrations_vector = state->GetOrderedConcentrations();
  std::vector<double>& user_defined_reaction_rates = state->GetOrderedRateConstants();
  std::vector<double> initial_concentrations(num_grid_cells * num_concentrations);

  Error error;

  // Get species indices in concentration array
  Mappings species_ordering = GetSpeciesOrdering(micm, state, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(species_ordering.size_, num_concentrations);
  std::size_t A_index = FindMappingIndex(species_ordering, "A", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t B_index = FindMappingIndex(species_ordering, "B", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t C_index = FindMappingIndex(species_ordering, "C", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t D_index = FindMappingIndex(species_ordering, "D", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t E_index = FindMappingIndex(species_ordering, "E", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t F_index = FindMappingIndex(species_ordering, "F", &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteMappings(&species_ordering);

  // Get user-defined reaction rates indices in user-defined reaction rates array
  Mappings rate_ordering = GetUserDefinedReactionRatesOrdering(micm, state, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(rate_ordering.size_, num_user_defined_reaction_rates);
  std::size_t R1_index = FindMappingIndex(rate_ordering, "USER.reaction 1", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t R2_index = FindMappingIndex(rate_ordering, "USER.reaction 2", &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteMappings(&rate_ordering);

  for (int i = 0; i < num_grid_cells; ++i)
  {
    conditions[i].temperature_ = 275.0 + (rand() % 20 - 10);
    conditions[i].pressure_ = 101253.3 + (rand() % 1000 - 500);
    conditions[i].air_density_ = conditions[i].pressure_ / (GAS_CONSTANT * conditions[i].temperature_);
    concentrations_vector[i * num_concentrations + A_index] = 0.75 + (rand() % 10 - 5) * 0.01;
    concentrations_vector[i * num_concentrations + B_index] = 0.0;
    concentrations_vector[i * num_concentrations + C_index] = 0.4 + (rand() % 10 - 5) * 0.01;
    concentrations_vector[i * num_concentrations + D_index] = 0.8 + (rand() % 10 - 5) * 0.01;
    concentrations_vector[i * num_concentrations + E_index] = 0.0;
    concentrations_vector[i * num_concentrations + F_index] = 0.1 + (rand() % 10 - 5) * 0.01;
    user_defined_reaction_rates[i * num_user_defined_reaction_rates + R1_index] = 0.001 + (rand() % 10 - 5) * 0.0001;
    user_defined_reaction_rates[i * num_user_defined_reaction_rates + R2_index] = 0.002 + (rand() % 10 - 5) * 0.0001;
    for (int j = 0; j < num_concentrations; ++j)
    {
      initial_concentrations[i * num_concentrations + j] = concentrations_vector[i * num_concentrations + j];
    }

    DeleteError(&error);
  }
  state->SetConditions(conditions);

  String solver_state;
  SolverResultStats solver_stats;
  MicmSolve(micm, state, time_step, &solver_state, &solver_stats, &error);
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
    double k3 = CalculateArrhenius(arr1, conditions[i_cell].temperature_, conditions[i_cell].pressure_);
    double k4 = CalculateArrhenius(arr2, conditions[i_cell].temperature_, conditions[i_cell].pressure_);
    double A = initial_A * std::exp(-k3 * time_step);
    double B = initial_A * (k3 / (k4 - k3)) * (std::exp(-k3 * time_step) - std::exp(-k4 * time_step));
    double C = initial_C + initial_A * (1.0 + (k3 * std::exp(-k4 * time_step) - k4 * std::exp(-k3 * time_step)) / (k4 - k3));
    double D = initial_D * std::exp(-k1 * time_step);
    double E = initial_D * (k1 / (k2 - k1)) * (std::exp(-k1 * time_step) - std::exp(-k2 * time_step));
    double F = initial_F + initial_D * (1.0 + (k1 * std::exp(-k2 * time_step) - k2 * std::exp(-k1 * time_step)) / (k2 - k1));
    ASSERT_NEAR(concentrations_vector[i_cell * num_concentrations + A_index], A, test_accuracy);
    ASSERT_NEAR(concentrations_vector[i_cell * num_concentrations + B_index], B, test_accuracy);
    ASSERT_NEAR(concentrations_vector[i_cell * num_concentrations + C_index], C, test_accuracy);
    ASSERT_NEAR(concentrations_vector[i_cell * num_concentrations + D_index], D, test_accuracy);
    ASSERT_NEAR(concentrations_vector[i_cell * num_concentrations + E_index], E, test_accuracy);
    ASSERT_NEAR(concentrations_vector[i_cell * num_concentrations + F_index], F, test_accuracy);
  }
}

// Common test function for solving multiple grid cells with vectorizable matrices
void TestVectorMultipleGridCells(
    MICM* micm,
    musica::State* state,
    const size_t num_grid_cells,
    const double time_step,
    const double test_accuracy)
{
  const size_t num_concentrations = 6;
  const size_t num_user_defined_reaction_rates = 2;
  constexpr double GAS_CONSTANT = 8.31446261815324;  // J mol-1 K-1

  std::vector<micm::Conditions> conditions(num_grid_cells);
  std::vector<double>& concentrations_vector = state->GetOrderedConcentrations();
  std::vector<double>& user_defined_reaction_rates = state->GetOrderedRateConstants();
  std::vector<double> initial_concentrations(num_grid_cells * num_concentrations);

  Error error;
  // Get species indices in concentration array
  Mappings species_ordering = GetSpeciesOrdering(micm, state, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(species_ordering.size_, num_concentrations);
  std::size_t A_index = FindMappingIndex(species_ordering, "A", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t B_index = FindMappingIndex(species_ordering, "B", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t C_index = FindMappingIndex(species_ordering, "C", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t D_index = FindMappingIndex(species_ordering, "D", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t E_index = FindMappingIndex(species_ordering, "E", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t F_index = FindMappingIndex(species_ordering, "F", &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteMappings(&species_ordering);

  // Get user-defined reaction rates indices in user-defined reaction rates array
  Mappings rate_ordering = GetUserDefinedReactionRatesOrdering(micm, state, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(rate_ordering.size_, num_user_defined_reaction_rates);
  std::size_t R1_index = FindMappingIndex(rate_ordering, "USER.reaction 1", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t R2_index = FindMappingIndex(rate_ordering, "USER.reaction 2", &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteMappings(&rate_ordering);

  for (int i = 0; i < num_grid_cells; ++i)
  {
    conditions[i].temperature_ = 275.0 + (rand() % 20 - 10);
    conditions[i].pressure_ = 101253.3 + (rand() % 1000 - 500);
    conditions[i].air_density_ = conditions[i].pressure_ / (GAS_CONSTANT * conditions[i].temperature_);
    concentrations_vector[i + A_index * num_grid_cells] = 0.75 + (rand() % 10 - 5) * 0.01;
    concentrations_vector[i + B_index * num_grid_cells] = 0.0;
    concentrations_vector[i + C_index * num_grid_cells] = 0.4 + (rand() % 10 - 5) * 0.01;
    concentrations_vector[i + D_index * num_grid_cells] = 0.8 + (rand() % 10 - 5) * 0.01;
    concentrations_vector[i + E_index * num_grid_cells] = 0.0;
    concentrations_vector[i + F_index * num_grid_cells] = 0.1 + (rand() % 10 - 5) * 0.01;
    user_defined_reaction_rates[i + R1_index * num_grid_cells] = 0.001 + (rand() % 10 - 5) * 0.0001;
    user_defined_reaction_rates[i + R2_index * num_grid_cells] = 0.002 + (rand() % 10 - 5) * 0.0001;
    for (int j = 0; j < num_concentrations; ++j)
    {
      initial_concentrations[i + j * num_grid_cells] = concentrations_vector[i + j * num_grid_cells];
    }
  }

  state->SetConditions(conditions);

  String solver_state;
  SolverResultStats solver_stats;
  MicmSolve(micm, state, time_step, &solver_state, &solver_stats, &error);
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
    double initial_A = initial_concentrations[i_cell + A_index * num_grid_cells];
    double initial_C = initial_concentrations[i_cell + C_index * num_grid_cells];
    double initial_D = initial_concentrations[i_cell + D_index * num_grid_cells];
    double initial_F = initial_concentrations[i_cell + F_index * num_grid_cells];
    double k1 = user_defined_reaction_rates[i_cell + R1_index * num_grid_cells];
    double k2 = user_defined_reaction_rates[i_cell + R2_index * num_grid_cells];
    double k3 = CalculateArrhenius(arr1, conditions[i_cell].temperature_, conditions[i_cell].pressure_);
    double k4 = CalculateArrhenius(arr2, conditions[i_cell].temperature_, conditions[i_cell].pressure_);
    double A = initial_A * std::exp(-k3 * time_step);
    double B = initial_A * (k3 / (k4 - k3)) * (std::exp(-k3 * time_step) - std::exp(-k4 * time_step));
    double C = initial_C + initial_A * (1.0 + (k3 * std::exp(-k4 * time_step) - k4 * std::exp(-k3 * time_step)) / (k4 - k3));
    double D = initial_D * std::exp(-k1 * time_step);
    double E = initial_D * (k1 / (k2 - k1)) * (std::exp(-k1 * time_step) - std::exp(-k2 * time_step));
    double F = initial_F + initial_D * (1.0 + (k1 * std::exp(-k2 * time_step) - k2 * std::exp(-k1 * time_step)) / (k2 - k1));
    ASSERT_NEAR(concentrations_vector[i_cell + A_index * num_grid_cells], A, test_accuracy);
    ASSERT_NEAR(concentrations_vector[i_cell + B_index * num_grid_cells], B, test_accuracy);
    ASSERT_NEAR(concentrations_vector[i_cell + C_index * num_grid_cells], C, test_accuracy);
    ASSERT_NEAR(concentrations_vector[i_cell + D_index * num_grid_cells], D, test_accuracy);
    ASSERT_NEAR(concentrations_vector[i_cell + E_index * num_grid_cells], E, test_accuracy);
    ASSERT_NEAR(concentrations_vector[i_cell + F_index * num_grid_cells], F, test_accuracy);
  }
}

// Test case for solving multiple grid cells using vector-ordered Rosenbrock solver
TEST_F(MicmCApiTestFixture, SolveMultipleGridCellsUsingVectorOrderedRosenbrock)
{
  constexpr size_t num_grid_cells = MICM_VECTOR_MATRIX_SIZE;
  constexpr double time_step = 200.0;
  constexpr double test_accuracy = 5.0e-3;
  const char* config_path = "configs/analytical";
  Error error;
  DeleteMicm(micm, &error);
  DeleteState(state, &error);
  ASSERT_TRUE(IsSuccess(error));
  micm = CreateMicm(config_path, MICMSolver::Rosenbrock, num_grid_cells, &error);
  state = CreateMicmState(micm, &error);
  ASSERT_TRUE(IsSuccess(error));
  TestVectorMultipleGridCells(micm, state, num_grid_cells, time_step, test_accuracy);
  DeleteError(&error);
}

// Test case for solving multiple grid cells using standard-ordered Rosenbrock solver
TEST_F(MicmCApiTestFixture, SolveMultipleGridCellsUsingStandardOrderedRosenbrock)
{
  constexpr size_t num_grid_cells = 3;
  constexpr double time_step = 200.0;
  constexpr double test_accuracy = 5.0e-3;
  const char* config_path = "configs/analytical";
  Error error;
  DeleteMicm(micm, &error);
  DeleteState(state, &error);
  ASSERT_TRUE(IsSuccess(error));
  micm = CreateMicm(config_path, MICMSolver::RosenbrockStandardOrder, num_grid_cells, &error);
  state = CreateMicmState(micm, &error);
  ASSERT_TRUE(IsSuccess(error));
  TestStandardMultipleGridCells(micm, state, num_grid_cells, time_step, test_accuracy);
  DeleteError(&error);
}

// Test case for solving multiple grid cells using vector-ordered Backward Euler solver
TEST_F(MicmCApiTestFixture, SolveMultipleGridCellsUsingVectorOrderedBackwardEuler)
{
  constexpr size_t num_grid_cells = MICM_VECTOR_MATRIX_SIZE;
  constexpr double time_step = 10.0;
  constexpr double test_accuracy = 0.1;
  const char* config_path = "configs/analytical";
  Error error;
  DeleteMicm(micm, &error);
  DeleteState(state, &error);
  ASSERT_TRUE(IsSuccess(error));
  micm = CreateMicm(config_path, MICMSolver::BackwardEuler, num_grid_cells, &error);
  state = CreateMicmState(micm, &error);
  ASSERT_TRUE(IsSuccess(error));
  TestVectorMultipleGridCells(micm, state, num_grid_cells, time_step, test_accuracy);
  DeleteError(&error);
}

// Test case for solving multiple grid cells using standard-ordered Backward Euler solver
TEST_F(MicmCApiTestFixture, SolveMultipleGridCellsUsingStandardOrderedBackwardEuler)
{
  constexpr size_t num_grid_cells = 3;
  constexpr double time_step = 10.0;
  constexpr double test_accuracy = 0.1;
  const char* config_path = "configs/analytical";
  Error error;
  DeleteMicm(micm, &error);
  DeleteState(state, &error);
  ASSERT_TRUE(IsSuccess(error));
  micm = CreateMicm(config_path, MICMSolver::BackwardEulerStandardOrder, num_grid_cells, &error);
  state = CreateMicmState(micm, &error);
  ASSERT_TRUE(IsSuccess(error));
  TestStandardMultipleGridCells(micm, state, num_grid_cells, time_step, test_accuracy);
  DeleteError(&error);
}

// Test case for getting species properties
TEST_F(MicmCApiTestFixture, GetSpeciesProperty)
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
