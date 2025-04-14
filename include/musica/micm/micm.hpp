// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the defintion of the MICM class, which represents a multi-component reactive transport model.
// It also includes functions for creating and deleting MICM instances with c bindings.
#pragma once

#include <musica/micm/chemistry.hpp>
#include <musica/micm/parse.hpp>
#include <musica/micm/state.hpp>
#include <musica/util.hpp>

#include <micm/CPU.hpp>

#include <chrono>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace musica
{
  /// @brief Types of MICM solver
  enum MICMSolver
  {
    UndefinedSolver = 0,         // Undefined solver
    Rosenbrock,                  // Vector-ordered Rosenbrock solver
    RosenbrockStandardOrder,     // Standard-ordered Rosenbrock solver
    BackwardEuler,               // Vector-ordered BackwardEuler solver
    BackwardEulerStandardOrder,  // Standard-ordered BackwardEuler solver
  };

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
    /// @brief The final time the solver iterated to
    double final_time_{};
  };

  class MICM
  {
    /// @brief Variant that holds all solver types
    using SolverVariant = std::variant<
        std::unique_ptr<micm::Rosenbrock>,
        std::unique_ptr<micm::RosenbrockStandard>,
        std::unique_ptr<micm::BackwardEuler>,
        std::unique_ptr<micm::BackwardEulerStandard>>;


   public:
    SolverVariant solver_variant_;

    MICM(const Chemistry &chemistry, MICMSolver solver_type, int num_grid_cells);
    MICM() = default;
    ~MICM() = default;

    /// @brief Solve the system
    /// @param micm Pointer to MICM object
    /// @param state Pointer to state object
    /// @param time_step Time [s] to advance the state by
    /// @param solver_state State of the solver
    /// @param solver_stats Statistics of the solver
    /// @param error Error struct to indicate success or failure
    void Solve(
        MICM *micm,
        musica::State *state,
        double time_step,
        String *solver_state,
        SolverResultStats *solver_stats,
        Error *error);
    
    std::size_t NumberOfGridCells() const {
      return std::visit([](auto &solver) -> std::size_t { return solver->GetNumberOfGridCells(); }, solver_variant_);
    }

    /// @brief Get a property for a chemical species
    /// @param species_name Name of the species
    /// @param property_name Name of the property
    /// @param error Error struct to indicate success or failure
    /// @return Value of the property
    template<class T>
    T GetSpeciesProperty(const std::string &species_name, const std::string &property_name, Error *error)
    {
      *error = NoError();
      micm::System system = std::visit([](auto &solver) -> micm::System { return solver->GetSystem(); }, solver_variant_);
      for (const auto &species : system.gas_phase_.species_)
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
  };

}  // namespace musica
