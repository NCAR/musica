// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the MICM class, which represents a
// multi-component reactive transport model. It also includes functions for
// creating and deleting MICM instances, creating solvers, and solving the model.
#include <musica/micm/micm.hpp>
#include <musica/micm/parse.hpp>
#include <musica/micm/state.hpp>
#include <musica/util.hpp>

#include <micm/solver/rosenbrock_solver_parameters.hpp>
#include <micm/solver/solver_builder.hpp>
#include <micm/system/species.hpp>
#include <micm/version.hpp>

#include <mechanism_configuration/parser.hpp>
#include <mechanism_configuration/v0/types.hpp>

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <string>
#include <system_error>

namespace musica
{

  MICM *CreateMicm(const char *config_path, MICMSolver solver_type, int num_grid_cells, Error *error)
  {
    MICM *micm = new MICM();
    micm->SetNumGridCells(num_grid_cells);

    Chemistry chemistry = ReadConfiguration(std::string(config_path), error);
    if (!IsSuccess(*error))
    {
      delete micm;
      return nullptr;
    }

    micm->SetChemistry(chemistry);
    micm->SetSolverType(solver_type);

    if (solver_type == MICMSolver::Rosenbrock)
    {
      micm->CreateRosenbrock(chemistry, error);
    }
    else if (solver_type == MICMSolver::RosenbrockStandardOrder)
    {
      micm->CreateRosenbrockStandardOrder(chemistry, error);
    }
    else if (solver_type == MICMSolver::BackwardEuler)
    {
      micm->CreateBackwardEuler(chemistry, error);
    }
    else if (solver_type == MICMSolver::BackwardEulerStandardOrder)
    {
      micm->CreateBackwardEulerStandardOrder(chemistry, error);
    }
    else
    {
      DeleteError(error);
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
      musica::State *state,
      double time_step,
      String *solver_state,
      SolverResultStats *solver_stats,
      Error *error)
  {
    DeleteError(error);
    micm->Solve(micm, state, time_step, solver_state, solver_stats, error);
  };

  String MicmVersion()
  {
    return CreateString(micm::GetMicmVersion());
  }

  Mappings GetSpeciesOrdering(MICM *micm, musica::State *state, Error *error)
  {
    DeleteError(error);

    std::map<std::string, std::size_t> map;
    bool success = false;  // Track if a valid state was found
    std::visit(
        [&map, &success](auto &state)
        {
          map = state.variable_map_;
          success = true;
        },
        state->state_variant_);

    if (!success)
    {
      std::string msg = "State type not recognized or not supported";
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND, msg.c_str());
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

  Mappings GetUserDefinedReactionRatesOrdering(MICM *micm, musica::State *state, Error *error)
  {
    DeleteError(error);

    std::map<std::string, std::size_t> map;
    bool success = false;  // Track if a valid state was found
    std::visit(
        [&map, &success](auto &state)
        {
          map = state.custom_rate_parameter_map_;
          success = true;
        },
        state->state_variant_);

    if (!success)
    {
      std::string msg = "State type not recognized or not supported";
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND, msg.c_str());
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

  void MICM::CreateRosenbrock(const Chemistry &chemistry, Error *error)
  {
    DeleteError(error);
    try
    {
      solver_variant_ = std::make_unique<Rosenbrock>(
          micm::CpuSolverBuilder<
              micm::RosenbrockSolverParameters,
              micm::VectorMatrix<double, MICM_VECTOR_MATRIX_SIZE>,
              micm::SparseMatrix<double, micm::SparseMatrixVectorOrdering<MICM_VECTOR_MATRIX_SIZE>>>(
              micm::RosenbrockSolverParameters::ThreeStageRosenbrockParameters())
              .SetSystem(chemistry.system)
              .SetReactions(chemistry.processes)
              .SetNumberOfGridCells(num_grid_cells_)
              .SetIgnoreUnusedSpecies(true)
              .Build());

      DeleteError(error);
      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
  }

  void MICM::CreateRosenbrockStandardOrder(const Chemistry &chemistry, Error *error)
  {
    DeleteError(error);
    try
    {
      solver_variant_ =
          std::make_unique<RosenbrockStandard>(micm::CpuSolverBuilder<micm::RosenbrockSolverParameters>(
                                                   micm::RosenbrockSolverParameters::ThreeStageRosenbrockParameters())
                                                   .SetSystem(chemistry.system)
                                                   .SetReactions(chemistry.processes)
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

  void MICM::CreateBackwardEuler(const Chemistry &chemistry, Error *error)
  {
    try
    {
      solver_variant_ = std::make_unique<BackwardEuler>(
          micm::CpuSolverBuilder<
              micm::BackwardEulerSolverParameters,
              micm::VectorMatrix<double, MICM_VECTOR_MATRIX_SIZE>,
              micm::SparseMatrix<double, micm::SparseMatrixVectorOrdering<MICM_VECTOR_MATRIX_SIZE>>,
              micm::LuDecompositionDoolittle>(micm::BackwardEulerSolverParameters())
              .SetSystem(chemistry.system)
              .SetReactions(chemistry.processes)
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

  void MICM::CreateBackwardEulerStandardOrder(const Chemistry &chemistry, Error *error)
  {
    try
    {
      solver_variant_ = std::make_unique<BackwardEulerStandard>(
          micm::CpuSolverBuilder<micm::BackwardEulerSolverParameters>(micm::BackwardEulerSolverParameters())
              .SetSystem(chemistry.system)
              .SetReactions(chemistry.processes)
              .SetNumberOfGridCells(num_grid_cells_)
              .SetIgnoreUnusedSpecies(true)
              .Build());

      DeleteError(error);
      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
  }

  void MICM::Solve(
      MICM *micm,
      musica::State *state,
      double time_step,
      String *solver_state,
      SolverResultStats *solver_stats,
      Error *error)
  {
    try
    {
      auto solve_and_store_results = [&](auto &solver, auto &state)
      {
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
            result.final_time_);
      };

      std::visit(
          [&](auto &solver, auto &state)
          {
            using StateType = std::decay_t<decltype(state)>;
            using SolverType = std::decay_t<decltype(*solver)>;
            if constexpr (
                (std::is_same_v<StateType, StandardState> &&
                 (std::is_same_v<SolverType, RosenbrockStandard> || std::is_same_v<SolverType, BackwardEulerStandard>)) ||
                (std::is_same_v<StateType, VectorState> &&
                 (std::is_same_v<SolverType, Rosenbrock> || std::is_same_v<SolverType, BackwardEuler>)))
            {
              solve_and_store_results(solver, state);
            }
          },
          micm->solver_variant_,
          state->state_variant_);

      DeleteError(error);
      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      DeleteError(error);
      *error = ToError(e);
    }
  }

  Mappings GetSpeciesOrderingFortran(MICM *micm, Error *error)
  {
    musica::State *state = CreateMicmState(micm, error);
    auto speciesOrdering = GetSpeciesOrdering(micm, state, error);
    DeleteState(state, error);
    return speciesOrdering;
  }

  Mappings GetUserDefinedReactionRatesOrderingFortran(MICM *micm, Error *error)
  {
    musica::State *state = CreateMicmState(micm, error);
    auto reactionRates = GetUserDefinedReactionRatesOrdering(micm, state, error);
    DeleteState(state, error);
    return reactionRates;
  }

  void MicmSolveFortran(
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
    musica::State *state = CreateMicmState(micm, error);

    size_t num_conditions = micm->NumGridCells();

    std::vector<micm::Conditions> conditions_vector(num_conditions);
    for (size_t i = 0; i < num_conditions; i++)
    {
      conditions_vector[i].temperature_ = temperature[i];
      conditions_vector[i].pressure_ = pressure[i];
      conditions_vector[i].air_density_ = air_density[i];
    }
    state->SetOrderedConcentrations(concentrations);
    state->SetOrderedRateConstants(custom_rate_parameters);
    state->SetConditions(conditions_vector);
    MicmSolve(micm, state, time_step, solver_state, solver_stats, error);

    std::vector<double> conc = state->GetOrderedConcentrations();
    for (int i = 0; i < conc.size(); i++)
    {
      concentrations[i] = conc[i];
    }
    DeleteState(state, error);
  }
}  // namespace musica
