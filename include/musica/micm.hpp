// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the defintion of the MICM class, which represents a multi-component reactive transport model.
// It also includes functions for creating and deleting MICM instances with c bindings.
#pragma once

#include <musica/util.hpp>

#include <micm/configure/solver_config.hpp>
#include <micm/process/process_set.hpp>
#include <micm/solver/rosenbrock.hpp>
#include <micm/solver/rosenbrock_solver_parameters.hpp>
#include <micm/solver/solver.hpp>
#include <micm/util/matrix.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace musica
{

  class MICM;

#ifdef __cplusplus
  extern "C"
  {
#endif

    struct SolverResultStats
    {
      /// @brief The number of forcing function calls
      int64_t function_calls_{};
      /// @brief The number of jacobian function calls
      int64_t jacobian_updates_{};
      /// @brief The total number of internal time steps taken
      int64_t number_of_steps_{};
      /// @brief The number of accepted integrations
      int64_t accepted_{};
      /// @brief The number of rejected integrations
      int64_t rejected_{};
      /// @brief The number of LU decompositions
      int64_t decompositions_{};
      /// @brief The number of linear solves
      int64_t solves_{};
      /// @brief The number of times a singular matrix is detected.
      int64_t singular_{};
      /// @brief The final time the solver iterated to
      double final_time_{};
      /// @brief The final state the solver was in

      SolverResultStats()
          : function_calls_(0),
            jacobian_updates_(0),
            number_of_steps_(0),
            accepted_(0),
            rejected_(0),
            decompositions_(0),
            solves_(0),
            singular_(0),
            final_time_(0.0)
      {
      }

      SolverResultStats(
          int64_t func_calls,
          int64_t jacobian,
          int64_t num_steps,
          int64_t accepted,
          int64_t rejected,
          int64_t decompositions,
          int64_t solves,
          int64_t singular,
          double final_time)
          : function_calls_(func_calls),
            jacobian_updates_(jacobian),
            number_of_steps_(num_steps),
            accepted_(accepted),
            rejected_(rejected),
            decompositions_(decompositions),
            solves_(solves),
            singular_(singular),
            final_time_(final_time)
      {
      }
    };

    MICM *CreateMicm(const char *config_path, Error *error);
    void DeleteMicm(const MICM *micm, Error *error);
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
        Error *error);
    String MicmVersion();
    Mapping *GetSpeciesOrdering(MICM *micm, std::size_t *array_size, Error *error);
    Mapping *GetUserDefinedReactionRatesOrdering(MICM *micm, std::size_t *array_size, Error *error);
    String GetSpeciesPropertyString(MICM *micm, const char *species_name, const char *property_name, Error *error);
    double GetSpeciesPropertyDouble(MICM *micm, const char *species_name, const char *property_name, Error *error);
    int GetSpeciesPropertyInt(MICM *micm, const char *species_name, const char *property_name, Error *error);
    bool GetSpeciesPropertyBool(MICM *micm, const char *species_name, const char *property_name, Error *error);
#ifdef __cplusplus
  }
#endif

  class MICM
  {
   public:
    /// @brief Create a solver by reading and parsing configuration file
    /// @param config_path Path to configuration file or directory containing configuration file
    /// @param error Error struct to indicate success or failure
    void Create(const std::string &config_path, Error *error);

    /// @brief Solve the system
    /// @param time_step Time [s] to advance the state by
    /// @param temperature Temperature [K]
    /// @param pressure Pressure [Pa]
    /// @param air_density Air density [mol m-3]
    /// @param num_concentrations The size of the concentrations array
    /// @param concentrations Array of species' concentrations
    /// @param num_custom_rate_parameters The size of the custom_rate_parameters array
    /// @param custom_rate_parameters Array of custom rate parameters
    /// @param error Error struct to indicate success or failure
    void Solve(
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
        Error *error);

    /// @brief Get the ordering of species
    /// @param error Error struct to indicate success or failure
    /// @return Map of species names to their indices
    std::map<std::string, std::size_t> GetSpeciesOrdering(Error *error);

    /// @brief Get the ordering of user-defined reaction rates
    /// @param error Error struct to indicate success or failure
    /// @return Map of reaction rate names to their indices
    std::map<std::string, std::size_t> GetUserDefinedReactionRatesOrdering(Error *error);

    /// @brief Get a property for a chemical species
    /// @param species_name Name of the species
    /// @param property_name Name of the property
    /// @param error Error struct to indicate success or failure
    /// @return Value of the property
    template<class T>
    T GetSpeciesProperty(const std::string &species_name, const std::string &property_name, Error *error);

    static constexpr std::size_t NUM_GRID_CELLS = 1;

   private:
    using DenseMatrixPolicy = micm::Matrix<double>;
    using SparseMatrixPolicy = micm::SparseMatrix<double, micm::SparseMatrixStandardOrdering>;
    using SolverPolicy = typename micm::RosenbrockSolverParameters::
        template SolverType<micm::ProcessSet, micm::LinearSolver<SparseMatrixPolicy, micm::LuDecomposition>>;
    using Rosenbrock = micm::Solver<SolverPolicy, micm::State<DenseMatrixPolicy, SparseMatrixPolicy>>;

    std::unique_ptr<Rosenbrock> solver_;

    std::unique_ptr<micm::SolverParameters> solver_parameters_;
  };

  template<class T>
  inline T MICM::GetSpeciesProperty(const std::string &species_name, const std::string &property_name, Error *error)
  {
    *error = NoError();
    for (const auto &species : solver_parameters_->system_.gas_phase_.species_)
    {
      if (species.name_ == species_name)
      {
        try
        {
          return species.GetProperty<T>(property_name);
        }
        catch (const std::system_error &e)
        {
          DeleteError(error);
          *error = ToError(e);
          return T();
        }
      }
    }
    std::string msg = "Species '" + species_name + "' not found";
    DeleteError(error);
    *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SPECIES_NOT_FOUND, msg.c_str());
    return T();
  }
}  // namespace musica
