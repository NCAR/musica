// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the MICM class, which represents a
// multi-component reactive transport model. It also includes functions for
// creating and deleting MICM instances, creating solvers, and solving the model.
#include <musica/micm.hpp>
#include <musica/util.hpp>

#include <micm/solver/rosenbrock_solver_parameters.hpp>
#include <micm/solver/solver_builder.hpp>
#include <micm/system/species.hpp>
#include <micm/version.hpp>

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <string>
#include <system_error>

namespace musica
{

  MICM *CreateMicm(const char *config_path, MICMSolver solver_type, int num_grid_cells, Error *error)
  {
    DeleteError(error);
    MICM *micm = new MICM();
    micm->SetNumGridCells(num_grid_cells);

    if (solver_type == MICMSolver::Rosenbrock)
    {
      micm->SetSolverType(MICMSolver::Rosenbrock);
      micm->CreateRosenbrock(std::string(config_path), error);
    }
    else if (solver_type == MICMSolver::RosenbrockStandardOrder)
    {
      micm->SetSolverType(MICMSolver::RosenbrockStandardOrder);
      micm->CreateRosenbrockStandardOrder(std::string(config_path), error);
    }
    else if (solver_type == MICMSolver::BackwardEuler)
    {
      micm->SetSolverType(MICMSolver::BackwardEuler);
      micm->CreateBackwardEuler(std::string(config_path), error);
    }
    else if (solver_type == MICMSolver::BackwardEulerStandardOrder)
    {
      micm->SetSolverType(MICMSolver::BackwardEulerStandardOrder);
      micm->CreateBackwardEulerStandardOrder(std::string(config_path), error);
    }
    else
    {
      std::string msg = "Solver type '" + std::to_string(solver_type) + "' not found";
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND, msg.c_str());
      delete micm;
      return nullptr;
    }
    if (!IsSuccess(*error))
    {
      delete micm;
      return nullptr;
    }
    return micm;
  }

  void DeleteMicm(const MICM *micm, Error *error)
  {
    DeleteError(error);
    if (micm == nullptr)
    {
      *error = NoError();
      return;
    }
    try
    {
      delete micm;
      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
  }

  void MicmSolve(
      MICM *micm,
      double time_step,
      double *temperature,
      double *pressure,
      double *air_density,
      double *concentrations,
      double *custom_rate_parameters,
      String *solver_state,
      SolverResultStats *solver_stats,
      Error *error)
  {
    DeleteError(error);

    if (micm->solver_type_ == MICMSolver::Rosenbrock)
    {
      micm->Solve(
          micm->rosenbrock_,
          time_step,
          temperature,
          pressure,
          air_density,
          concentrations,
          custom_rate_parameters,
          solver_state,
          solver_stats,
          error);
    }
    else if (micm->solver_type_ == MICMSolver::RosenbrockStandardOrder)
    {
      micm->Solve(
          micm->rosenbrock_standard_,
          time_step,
          temperature,
          pressure,
          air_density,
          concentrations,
          custom_rate_parameters,
          solver_state,
          solver_stats,
          error);
    }
    else if (micm->solver_type_ == MICMSolver::BackwardEuler)
    {
      micm->Solve(
          micm->backward_euler_,
          time_step,
          temperature,
          pressure,
          air_density,
          concentrations,
          custom_rate_parameters,
          solver_state,
          solver_stats,
          error);
    }
    else if (micm->solver_type_ == MICMSolver::BackwardEulerStandardOrder)
    {
      micm->Solve(
          micm->backward_euler_standard_,
          time_step,
          temperature,
          pressure,
          air_density,
          concentrations,
          custom_rate_parameters,
          solver_state,
          solver_stats,
          error);
    }
  };

  String MicmVersion()
  {
    return CreateString(micm::GetMicmVersion());
  }

  Mappings GetSpeciesOrdering(MICM *micm, Error *error)
  {
    DeleteError(error);

    std::map<std::string, std::size_t> map;

    if (micm->solver_type_ == MICMSolver::Rosenbrock)
    {
      map = micm->GetSpeciesOrdering(micm->rosenbrock_, error);
    }
    else if (micm->solver_type_ == MICMSolver::RosenbrockStandardOrder)
    {
      map = micm->GetSpeciesOrdering(micm->rosenbrock_standard_, error);
    }
    else if (micm->solver_type_ == MICMSolver::BackwardEuler)
    {
      map = micm->GetSpeciesOrdering(micm->backward_euler_, error);
    }
    else if (micm->solver_type_ == MICMSolver::BackwardEulerStandardOrder)
    {
      map = micm->GetSpeciesOrdering(micm->backward_euler_standard_, error);
    }
    else
    {
      std::string msg = "Solver type '" + std::to_string(micm->solver_type_) + "' not found";
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND, msg.c_str());
    }
    if (!IsSuccess(*error))
    {
      return Mappings();
    }

    Mappings species_ordering;
    species_ordering.mappings_ = new Mapping[map.size()];
    species_ordering.size_ = map.size();

    // Copy data from the map to the array of structs
    std::size_t i = 0;
    for (const auto &entry : map)
    {
      species_ordering.mappings_[i] = ToMapping(entry.first.c_str(), entry.second);
      ++i;
    }
    return species_ordering;
  }

  Mappings GetUserDefinedReactionRatesOrdering(MICM *micm, Error *error)
  {
    DeleteError(error);

    std::map<std::string, std::size_t> map;

    if (micm->solver_type_ == MICMSolver::Rosenbrock)
    {
      map = micm->GetUserDefinedReactionRatesOrdering(micm->rosenbrock_, error);
    }
    else if (micm->solver_type_ == MICMSolver::RosenbrockStandardOrder)
    {
      map = micm->GetUserDefinedReactionRatesOrdering(micm->rosenbrock_standard_, error);
    }
    else if (micm->solver_type_ == MICMSolver::BackwardEuler)
    {
      map = micm->GetUserDefinedReactionRatesOrdering(micm->backward_euler_, error);
    }
    else if (micm->solver_type_ == MICMSolver::BackwardEulerStandardOrder)
    {
      map = micm->GetUserDefinedReactionRatesOrdering(micm->backward_euler_standard_, error);
    }
    else
    {
      std::string msg = "Solver type '" + std::to_string(micm->solver_type_) + "' not found";
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND, msg.c_str());
    }
    if (!IsSuccess(*error))
    {
      return Mappings();
    }

    Mappings reaction_rates;
    reaction_rates.mappings_ = new Mapping[map.size()];
    reaction_rates.size_ = map.size();

    // Copy data from the map to the array of structs
    std::size_t i = 0;
    for (const auto &entry : map)
    {
      reaction_rates.mappings_[i] = ToMapping(entry.first.c_str(), entry.second);
      ++i;
    }
    return reaction_rates;
  }

  String GetSpeciesPropertyString(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    DeleteError(error);
    std::string species_name_str(species_name);
    std::string property_name_str(property_name);
    const std::string value_str = micm->GetSpeciesProperty<std::string>(species_name_str, property_name_str, error);
    if (!IsSuccess(*error))
    {
      return String();
    }
    return CreateString(value_str.c_str());
  }

  double GetSpeciesPropertyDouble(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    DeleteError(error);
    std::string species_name_str(species_name);
    std::string property_name_str(property_name);
    return micm->GetSpeciesProperty<double>(species_name_str, property_name_str, error);
  }

  int GetSpeciesPropertyInt(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    DeleteError(error);
    std::string species_name_str(species_name);
    std::string property_name_str(property_name);
    return micm->GetSpeciesProperty<int>(species_name_str, property_name_str, error);
  }

  bool GetSpeciesPropertyBool(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    DeleteError(error);
    std::string species_name_str(species_name);
    std::string property_name_str(property_name);
    return micm->GetSpeciesProperty<bool>(species_name_str, property_name_str, error);
  }

  void MICM::CreateRosenbrock(const std::string &config_path, Error *error)
  {
    DeleteError(error);
    try
    {
      micm::SolverConfig<> solver_config;
      solver_config.ReadAndParse(std::filesystem::path(config_path));
      solver_parameters_ = std::make_unique<micm::SolverParameters>(solver_config.GetSolverParams());

      rosenbrock_ = std::make_unique<Rosenbrock>(
          micm::CpuSolverBuilder<
              micm::RosenbrockSolverParameters,
              micm::VectorMatrix<double, MICM_VECTOR_MATRIX_SIZE>,
              micm::SparseMatrix<double, micm::SparseMatrixVectorOrdering<MICM_VECTOR_MATRIX_SIZE>>>(
              micm::RosenbrockSolverParameters::ThreeStageRosenbrockParameters())
              .SetSystem(solver_parameters_->system_)
              .SetReactions(solver_parameters_->processes_)
              .SetNumberOfGridCells(num_grid_cells_)
              .SetIgnoreUnusedSpecies(true)
              .Build());

      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
  }

  void MICM::CreateRosenbrockStandardOrder(const std::string &config_path, Error *error)
  {
    DeleteError(error);
    try
    {
      micm::SolverConfig<> solver_config;
      solver_config.ReadAndParse(std::filesystem::path(config_path));
      solver_parameters_ = std::make_unique<micm::SolverParameters>(solver_config.GetSolverParams());

      rosenbrock_standard_ =
          std::make_unique<RosenbrockStandard>(micm::CpuSolverBuilder<micm::RosenbrockSolverParameters>(
                                                   micm::RosenbrockSolverParameters::ThreeStageRosenbrockParameters())
                                                   .SetSystem(solver_parameters_->system_)
                                                   .SetReactions(solver_parameters_->processes_)
                                                   .SetNumberOfGridCells(num_grid_cells_)
                                                   .SetIgnoreUnusedSpecies(true)
                                                   .Build());

      DeleteError(error);
      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      DeleteError(error);
      *error = ToError(e);
    }
  }

  void MICM::CreateBackwardEuler(const std::string &config_path, Error *error)
  {
    try
    {
      micm::SolverConfig<> solver_config;
      solver_config.ReadAndParse(std::filesystem::path(config_path));
      solver_parameters_ = std::make_unique<micm::SolverParameters>(solver_config.GetSolverParams());

      backward_euler_ = std::make_unique<BackwardEuler>(
          micm::SolverBuilder<
              micm::BackwardEulerSolverParameters,
              micm::VectorMatrix<double, MICM_VECTOR_MATRIX_SIZE>,
              micm::SparseMatrix<double, micm::SparseMatrixVectorOrdering<MICM_VECTOR_MATRIX_SIZE>>,
              micm::ProcessSet,
              micm::LinearSolver<
                  micm::SparseMatrix<double, micm::SparseMatrixVectorOrdering<MICM_VECTOR_MATRIX_SIZE>>,
                  micm::LuDecomposition>,
              VectorState>(micm::BackwardEulerSolverParameters())
              .SetSystem(solver_parameters_->system_)
              .SetReactions(solver_parameters_->processes_)
              .SetNumberOfGridCells(num_grid_cells_)
              .SetIgnoreUnusedSpecies(true)
              .Build());

      DeleteError(error);
      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      DeleteError(error);
      *error = ToError(e);
    }
  }

  void MICM::CreateBackwardEulerStandardOrder(const std::string &config_path, Error *error)
  {
    try
    {
      micm::SolverConfig<> solver_config;
      solver_config.ReadAndParse(std::filesystem::path(config_path));
      solver_parameters_ = std::make_unique<micm::SolverParameters>(solver_config.GetSolverParams());

      backward_euler_standard_ = std::make_unique<BackwardEulerStandard>(
          micm::CpuSolverBuilder<micm::BackwardEulerSolverParameters>(micm::BackwardEulerSolverParameters())
              .SetSystem(solver_parameters_->system_)
              .SetReactions(solver_parameters_->processes_)
              .SetNumberOfGridCells(num_grid_cells_)
              .SetIgnoreUnusedSpecies(true)
              .Build());

      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
  }

  void MICM::Solve(
      auto &solver,
      double time_step,
      double *temperature,
      double *pressure,
      double *air_density,
      double *concentrations,
      double *custom_rate_parameters,
      String *solver_state,
      SolverResultStats *solver_stats,
      Error *error)
  {
    try
    {
      micm::State state = solver->GetState();
      const std::size_t num_species = state.variables_.NumColumns();
      const std::size_t num_custom_rate_parameters = state.custom_rate_parameters_.NumColumns();

      int i_species_elem = 0;
      int i_param_elem = 0;
      for (int i_cell{}; i_cell < num_grid_cells_; ++i_cell)
      {
        state.conditions_[i_cell].temperature_ = temperature[i_cell];
        state.conditions_[i_cell].pressure_ = pressure[i_cell];
        state.conditions_[i_cell].air_density_ = air_density[i_cell];
        for (int i_species{}; i_species < num_species; ++i_species)
        {
          state.variables_[i_cell][i_species] = concentrations[i_species_elem++];
        }
        for (int i_param{}; i_param < num_custom_rate_parameters; ++i_param)
        {
          state.custom_rate_parameters_[i_cell][i_param] = custom_rate_parameters[i_param_elem++];
        }
      }

      solver->CalculateRateConstants(state);
      auto result = solver->Solve(time_step, state);

      *solver_state = CreateString(micm::SolverStateToString(result.state_).c_str());

      *solver_stats = SolverResultStats(
          result.stats_.function_calls_,
          result.stats_.jacobian_updates_,
          result.stats_.number_of_steps_,
          result.stats_.accepted_,
          result.stats_.rejected_,
          result.stats_.decompositions_,
          result.stats_.solves_,
          result.stats_.singular_,
          result.final_time_);

      i_species_elem = 0;
      for (int i_cell{}; i_cell < num_grid_cells_; ++i_cell)
      {
        for (int i_species{}; i_species < num_species; ++i_species)
        {
          concentrations[i_species_elem++] = state.variables_[i_cell][i_species];
        }
      }

      DeleteError(error);
      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      DeleteError(error);
      *error = ToError(e);
    }
  }

}  // namespace musica
