// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the MICM class, which represents a
// multi-component reactive transport model. It also includes functions for
// creating and deleting MICM instances, creating solvers, and solving the model.
#include <musica/micm.hpp>
#include <musica/util.hpp>

#include <micm/solver/rosenbrock_solver_parameters.hpp>
#include <micm/solver/backward_euler_solver_parameters.hpp>
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
    else if (solver_type == MICMSolver::BackwardEuler)
    {
      micm->SetSolverType(MICMSolver::BackwardEuler);
    }
    else if (solver_type == MICMSolver::RosenbrockStandardOrder)
    {
      micm->SetSolverType(MICMSolver::RosenbrockStandardOrder);
      micm->CreateRosenbrockStandardOrder(std::string(config_path), error);
    }
    else if (solver_type == MICMSolver::BackwardEulerStandardOrder)
    {
      micm->SetSolverType(MICMSolver::BackwardEulerStandardOrder);
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
      double temperature,
      double pressure,
      double air_density,
      int num_concentrations,
      double *concentrations,
      int num_custom_rate_parameters,
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
          num_concentrations,
          concentrations,
          num_custom_rate_parameters,
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
          num_concentrations,
          concentrations,
          num_custom_rate_parameters,
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

  Mapping *GetSpeciesOrdering(MICM *micm, std::size_t *array_size, Error *error)
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
    if (!IsSuccess(*error))
    {
      return nullptr;
    }

    Mapping *species_ordering = new Mapping[map.size()];

    // Copy data from the map to the array of structs
    std::size_t i = 0;
    for (const auto &entry : map)
    {
      species_ordering[i] = ToMapping(entry.first.c_str(), entry.second);
      ++i;
    }

    // Set the size of the array
    *array_size = map.size();
    return species_ordering;
  }

  Mapping *GetUserDefinedReactionRatesOrdering(MICM *micm, std::size_t *array_size, Error *error)
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
    if (!IsSuccess(*error))
    {
      return nullptr;
    }

    Mapping *reactionRates = new Mapping[map.size()];

    // Copy data from the map to the array of structs
    std::size_t i = 0;
    for (const auto &entry : map)
    {
      reactionRates[i] = ToMapping(entry.first.c_str(), entry.second);
      ++i;
    }

    // Set the size of the array
    *array_size = map.size();
    return reactionRates;
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
    try
    {
      micm::SolverConfig<> solver_config;
      solver_config.ReadAndParse(std::filesystem::path(config_path));
      solver_parameters_ = std::make_unique<micm::SolverParameters>(solver_config.GetSolverParams());

      rosenbrock_ = std::make_unique<Rosenbrock>(
          micm::SolverBuilder<
              micm::RosenbrockSolverParameters,
              micm::VectorMatrix<double, MICM_VECTOR_MATRIX_SIZE>,
              micm::SparseMatrix<double, micm::SparseMatrixVectorOrdering<MICM_VECTOR_MATRIX_SIZE>>,
              micm::ProcessSet,
              micm::LinearSolver<
                  micm::SparseMatrix<double, micm::SparseMatrixVectorOrdering<MICM_VECTOR_MATRIX_SIZE>>,
                  micm::LuDecomposition>>(micm::RosenbrockSolverParameters::ThreeStageRosenbrockParameters())
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
                  micm::LuDecomposition>>(micm::BackwardEulerSolverParameters())
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

  void MICM::CreateRosenbrockStandardOrder(const std::string &config_path, Error *error)
  {
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

  void MICM::Solve(
      auto &solver,
      double time_step,
      double temperature,
      double pressure,
      double air_density,
      int num_concentrations,
      double *concentrations,
      int num_custom_rate_parameters,
      double *custom_rate_parameters,
      String *solver_state,
      SolverResultStats *solver_stats,
      Error *error)
  {
    try
    {
      micm::State state = solver->GetState();

      for (int cell{}; cell < num_grid_cells_; cell++)
      {
        state.conditions_[cell].temperature_ = temperature;
        state.conditions_[cell].pressure_ = pressure;
        state.conditions_[cell].air_density_ = air_density;
      }

      state.variables_.AsVector().assign(concentrations, concentrations + num_concentrations);

      state.custom_rate_parameters_.AsVector().assign(
          custom_rate_parameters, custom_rate_parameters + num_custom_rate_parameters);

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

      for (int i = 0; i < state.variables_.AsVector().size(); i++)
      {
        concentrations[i] = state.variables_.AsVector()[i];
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
