#include <musica/micm/micm.hpp>
#include <musica/micm/micm_c_interface.hpp>
#include <musica/micm/state.hpp>
#include <musica/micm/state_c_interface.hpp>
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
    micm = CreateMicm(config_path, MICMSolver::Rosenbrock, &error);
    state = CreateMicmState(micm, num_grid_cells, &error);

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
  auto state = CreateMicmState(micm, 1, &error);
  ASSERT_EQ(state, nullptr);
  ASSERT_TRUE(IsError(error, MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND));
  DeleteError(&error);
}

// Test case for creating state successfully
TEST_F(MicmCApiTestFixture, CreateStateSuccess)
{
  Error error;
  musica::MICM* micm = CreateMicm(config_path, MICMSolver::Rosenbrock, &error);
  auto state = CreateMicmState(micm, num_grid_cells, &error);
  ASSERT_NE(state, nullptr);
  delete state;
  delete micm;
}

// Test case for bad configuration file path
TEST(MicmCApiTest, BadConfigurationFilePath)
{
  Error error = NoError();
  auto micm_bad_config = CreateMicm("bad config path", MICMSolver::Rosenbrock, &error);
  ASSERT_EQ(micm_bad_config, nullptr);
  ASSERT_TRUE(IsError(error, MUSICA_ERROR_CATEGORY_PARSING, MUSICA_PARSE_INVALID_CONFIG_FILE));
  DeleteError(&error);
}

// Test case for bad input for solver type
TEST(MicmCApiTest, BadSolverType)
{
  short solver_type = 999;
  Error error = NoError();
  auto micm_bad_solver_type = CreateMicm("configs/chapman", static_cast<MICMSolver>(solver_type), &error);
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
  Mappings species_ordering = GetSpeciesOrdering(state, &error);
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
TEST_F(MicmCApiTestFixture, GetUserDefinedRateParametersOrdering)
{
  Error error;
  Mappings reaction_rates_ordering = GetUserDefinedRateParametersOrdering(state, &error);
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
  const std::size_t num_user_defined_rate_parameters = 3;
  Error error;

  std::vector<micm::Conditions> conditions(1);
  std::vector<double>& concentrations_vector = state->GetOrderedConcentrations();
  std::vector<double>& user_defined_rate_parameters = state->GetOrderedRateParameters();

  // Get species ordering
  Mappings species_ordering = GetSpeciesOrdering(state, &error);
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
  Mappings reaction_rates_ordering = GetUserDefinedRateParametersOrdering(state, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(reaction_rates_ordering.size_, num_user_defined_rate_parameters);
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
  user_defined_rate_parameters[jO2_index] = 2.7e-19;
  user_defined_rate_parameters[jO3_O_index] = 1.13e-9;
  user_defined_rate_parameters[jO3_O1D_index] = 5.8e-8;

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
  MICM* micm = CreateMicm(config_path, MICMSolver::RosenbrockStandardOrder, &error);
  musica::State* state = CreateMicmState(micm, num_grid_cells, &error);

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
  MICM* micm = CreateMicm(config_path, MICMSolver::BackwardEulerStandardOrder, &error);
  musica::State* state = CreateMicmState(micm, num_grid_cells, &error);

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

void TestSolver(MICM* micm, const size_t num_grid_cells, const double time_step, const double test_accuracy)
{
  const size_t num_concentrations = 6;
  const size_t num_user_defined_rate_parameters = 2;
  constexpr double GAS_CONSTANT = 8.31446261815324;  // J mol-1 K-1

  Error error;

  // Create MICM states
  size_t state_1_size = std::min(num_grid_cells, micm->GetMaximumNumberOfGridCells());
  size_t state_2_size = (num_grid_cells - state_1_size) % micm->GetMaximumNumberOfGridCells();
  musica::State* state_1 = CreateMicmState(micm, state_1_size, &error);
  ASSERT_TRUE(IsSuccess(error));
  musica::State* state_2 = nullptr;
  if (state_2_size > 0)
  {
    state_2 = CreateMicmState(micm, state_2_size, &error);
    ASSERT_TRUE(IsSuccess(error));
  }

  // Get species indices in concentration array
  Mappings species_ordering = GetSpeciesOrdering(state_1, &error);
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
  Mappings rate_ordering = GetUserDefinedRateParametersOrdering(state_1, &error);
  ASSERT_TRUE(IsSuccess(error));
  ASSERT_EQ(rate_ordering.size_, num_user_defined_rate_parameters);
  std::size_t R1_index = FindMappingIndex(rate_ordering, "USER.reaction 1", &error);
  ASSERT_TRUE(IsSuccess(error));
  std::size_t R2_index = FindMappingIndex(rate_ordering, "USER.reaction 2", &error);
  ASSERT_TRUE(IsSuccess(error));
  DeleteMappings(&rate_ordering);

  for (int i_state = 0; i_state < std::ceil(static_cast<double>(num_grid_cells) / state_1_size); ++i_state)
  {
    // Get the right state and set dimensions
    size_t state_size = std::min(num_grid_cells - i_state * state_1_size, state_1_size);
    musica::State* current_state = (state_size == state_1_size) ? state_1 : state_2;
    ASSERT_TRUE(current_state != nullptr);
    ASSERT_EQ(GetNumberOfGridCells(current_state, &error), state_size);
    ASSERT_TRUE(IsSuccess(error));
    ASSERT_EQ(GetNumberOfSpecies(current_state, &error), num_concentrations);
    ASSERT_TRUE(IsSuccess(error));
    ASSERT_EQ(GetNumberOfUserDefinedRateParameters(current_state, &error), num_user_defined_rate_parameters);
    ASSERT_TRUE(IsSuccess(error));

    // Get pointers to the data
    size_t array_size;
    micm::Conditions* conditions = GetConditionsPointer(current_state, &array_size, &error);
    ASSERT_TRUE(IsSuccess(error));
    ASSERT_EQ(array_size, state_size);
    double* concentrations = GetOrderedConcentrationsPointer(current_state, &array_size, &error);
    ASSERT_TRUE(IsSuccess(error));
    ASSERT_GE(array_size, state_size * num_concentrations);
    double* user_defined_params = GetOrderedRateParametersPointer(current_state, &array_size, &error);
    ASSERT_TRUE(IsSuccess(error));
    ASSERT_GE(array_size, state_size * num_user_defined_rate_parameters);

    // Get matrix strides
    size_t grid_cell_stride_species;
    size_t species_stride;
    size_t grid_cell_stride_params;
    size_t params_stride;
    GetConcentrationsStrides(current_state, &error, &grid_cell_stride_species, &species_stride);
    ASSERT_TRUE(IsSuccess(error));
    GetUserDefinedRateParametersStrides(current_state, &error, &grid_cell_stride_params, &params_stride);
    ASSERT_TRUE(IsSuccess(error));

    // Set up an intial concentration vector
    std::vector<double> initial_concentrations(state_size * num_concentrations);

    for (int i = 0; i < state_size; ++i)
    {
      conditions[i].temperature_ = 275.0 + (rand() % 20 - 10);
      conditions[i].pressure_ = 101253.3 + (rand() % 1000 - 500);
      conditions[i].air_density_ = conditions[i].pressure_ / (GAS_CONSTANT * conditions[i].temperature_);
      concentrations[i * grid_cell_stride_species + A_index * species_stride] = 0.75 + (rand() % 10 - 5) * 0.01;
      concentrations[i * grid_cell_stride_species + B_index * species_stride] = 0.0;
      concentrations[i * grid_cell_stride_species + C_index * species_stride] = 0.4 + (rand() % 10 - 5) * 0.01;
      concentrations[i * grid_cell_stride_species + D_index * species_stride] = 0.8 + (rand() % 10 - 5) * 0.01;
      concentrations[i * grid_cell_stride_species + E_index * species_stride] = 0.0;
      concentrations[i * grid_cell_stride_species + F_index * species_stride] = 0.1 + (rand() % 10 - 5) * 0.01;
      user_defined_params[i * grid_cell_stride_params + R1_index * params_stride] = 0.001 + (rand() % 10 - 5) * 0.0001;
      user_defined_params[i * grid_cell_stride_params + R2_index * params_stride] = 0.002 + (rand() % 10 - 5) * 0.0001;
      for (int j = 0; j < num_concentrations; ++j)
      {
        initial_concentrations[i * num_concentrations + j] =
            concentrations[i * grid_cell_stride_species + j * species_stride];
      }
    }

    String solver_state;
    SolverResultStats solver_stats;
    MicmSolve(micm, current_state, time_step, &solver_state, &solver_stats, &error);
    ASSERT_TRUE(IsSuccess(error));
    DeleteError(&error);

    // update the concentrations pointer because MICM solve function may have swapped temporary state vectors
    concentrations = GetOrderedConcentrationsPointer(current_state, &array_size, &error);
    ASSERT_TRUE(IsSuccess(error));

    // Add assertions to check the solved concentrations
    ArrheniusReaction arr1;
    arr1.A_ = 0.004;
    arr1.C_ = 50.0;
    ArrheniusReaction arr2{ 0.012, -2, 75, 50, 1.0e-6 };

    ASSERT_STREQ(solver_state.value_, "Converged");
    DeleteString(&solver_state);

    for (int i_cell = 0; i_cell < state_size; ++i_cell)
    {
      double initial_A = initial_concentrations[i_cell * num_concentrations + A_index];
      double initial_C = initial_concentrations[i_cell * num_concentrations + C_index];
      double initial_D = initial_concentrations[i_cell * num_concentrations + D_index];
      double initial_F = initial_concentrations[i_cell * num_concentrations + F_index];
      double k1 = user_defined_params[i_cell * grid_cell_stride_params + R1_index * params_stride];
      double k2 = user_defined_params[i_cell * grid_cell_stride_params + R2_index * params_stride];
      double k3 = CalculateArrhenius(arr1, conditions[i_cell].temperature_, conditions[i_cell].pressure_);
      double k4 = CalculateArrhenius(arr2, conditions[i_cell].temperature_, conditions[i_cell].pressure_);
      double A = initial_A * std::exp(-k3 * time_step);
      double B = initial_A * (k3 / (k4 - k3)) * (std::exp(-k3 * time_step) - std::exp(-k4 * time_step));
      double C =
          initial_C + initial_A * (1.0 + (k3 * std::exp(-k4 * time_step) - k4 * std::exp(-k3 * time_step)) / (k4 - k3));
      double D = initial_D * std::exp(-k1 * time_step);
      double E = initial_D * (k1 / (k2 - k1)) * (std::exp(-k1 * time_step) - std::exp(-k2 * time_step));
      double F =
          initial_F + initial_D * (1.0 + (k1 * std::exp(-k2 * time_step) - k2 * std::exp(-k1 * time_step)) / (k2 - k1));
      ASSERT_NEAR(concentrations[i_cell * grid_cell_stride_species + A_index * species_stride], A, test_accuracy);
      ASSERT_NEAR(concentrations[i_cell * grid_cell_stride_species + B_index * species_stride], B, test_accuracy);
      ASSERT_NEAR(concentrations[i_cell * grid_cell_stride_species + C_index * species_stride], C, test_accuracy);
      ASSERT_NEAR(concentrations[i_cell * grid_cell_stride_species + D_index * species_stride], D, test_accuracy);
      ASSERT_NEAR(concentrations[i_cell * grid_cell_stride_species + E_index * species_stride], E, test_accuracy);
      ASSERT_NEAR(concentrations[i_cell * grid_cell_stride_species + F_index * species_stride], F, test_accuracy);
    }
  }
  DeleteState(state_1, &error);
  ASSERT_TRUE(IsSuccess(error));
  if (state_2 != nullptr)
  {
    DeleteState(state_2, &error);
    ASSERT_TRUE(IsSuccess(error));
  }
  DeleteError(&error);
}

// Test case for solving multiple grid cells using vector-ordered Rosenbrock solver
TEST_F(MicmCApiTestFixture, SolveMultipleGridCellsUsingVectorOrderedRosenbrock)
{
  constexpr double time_step = 200.0;
  constexpr double test_accuracy = 5.0e-3;
  const char* config_path = "configs/analytical";
  Error error;
  DeleteMicm(micm, &error);
  ASSERT_TRUE(IsSuccess(error));
  micm = CreateMicm(config_path, MICMSolver::Rosenbrock, &error);
  ASSERT_TRUE(IsSuccess(error));
  size_t max_cells = GetMaximumNumberOfGridCells(micm);
  ASSERT_GT(max_cells, 0);
  for (int num_grid_cells = 1; num_grid_cells <= max_cells * 3;
       num_grid_cells += static_cast<int>(std::floor(max_cells / 3)))
  {
    TestSolver(micm, num_grid_cells, time_step, test_accuracy);
    DeleteError(&error);
  }
}

// Test case for solving multiple grid cells using standard-ordered Rosenbrock solver
TEST_F(MicmCApiTestFixture, SolveMultipleGridCellsUsingStandardOrderedRosenbrock)
{
  constexpr double time_step = 200.0;
  constexpr double test_accuracy = 5.0e-3;
  const char* config_path = "configs/analytical";
  Error error;
  DeleteMicm(micm, &error);
  ASSERT_TRUE(IsSuccess(error));
  micm = CreateMicm(config_path, MICMSolver::RosenbrockStandardOrder, &error);
  ASSERT_TRUE(IsSuccess(error));
  size_t max_cells = GetMaximumNumberOfGridCells(micm);
  ASSERT_GT(max_cells, 1e8);
  for (int num_grid_cells = 1; num_grid_cells <= 20; num_grid_cells += 5)
  {
    TestSolver(micm, num_grid_cells, time_step, test_accuracy);
    DeleteError(&error);
  }
}

// Test case for solving multiple grid cells using vector-ordered Backward Euler solver
TEST_F(MicmCApiTestFixture, SolveMultipleGridCellsUsingVectorOrderedBackwardEuler)
{
  constexpr double time_step = 10.0;
  constexpr double test_accuracy = 0.1;
  const char* config_path = "configs/analytical";
  Error error;
  DeleteMicm(micm, &error);
  ASSERT_TRUE(IsSuccess(error));
  micm = CreateMicm(config_path, MICMSolver::BackwardEuler, &error);
  ASSERT_TRUE(IsSuccess(error));
  size_t max_cells = GetMaximumNumberOfGridCells(micm);
  ASSERT_GT(max_cells, 0);
  for (int num_grid_cells = 1; num_grid_cells <= max_cells * 3; num_grid_cells += std::floor(max_cells / 3))
  {
    TestSolver(micm, num_grid_cells, time_step, test_accuracy);
    DeleteError(&error);
  }
}

// Test case for solving multiple grid cells using standard-ordered Backward Euler solver
TEST_F(MicmCApiTestFixture, SolveMultipleGridCellsUsingStandardOrderedBackwardEuler)
{
  constexpr double time_step = 10.0;
  constexpr double test_accuracy = 0.1;
  const char* config_path = "configs/analytical";
  Error error;
  DeleteMicm(micm, &error);
  ASSERT_TRUE(IsSuccess(error));
  micm = CreateMicm(config_path, MICMSolver::BackwardEulerStandardOrder, &error);
  ASSERT_TRUE(IsSuccess(error));
  size_t max_cells = GetMaximumNumberOfGridCells(micm);
  ASSERT_GT(max_cells, 1.0e8);
  for (int num_grid_cells = 1; num_grid_cells <= 20; num_grid_cells += 5)
  {
    TestSolver(micm, num_grid_cells, time_step, test_accuracy);
    DeleteError(&error);
  }
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
