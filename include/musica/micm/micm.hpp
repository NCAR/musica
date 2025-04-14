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

enum class MusicaErrc
{
  SpeciesNotFound = MUSICA_ERROR_CODE_SPECIES_NOT_FOUND,
  SolverTypeNotFound = MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND,
  MappingNotFound = MUSICA_ERROR_CODE_MAPPING_NOT_FOUND,
  MappingOptionsUndefined = MUSICA_ERROR_CODE_MAPPING_OPTIONS_UNDEFINED,
  Unknown = MUSICA_ERROR_CODE_UNKNOWN,
};

namespace std
{
  template<>
  struct is_error_condition_enum<MusicaErrc> : true_type
  {
  };
}  // namespace std

namespace
{
  class MusicaErrorCategory : public std::error_category
  {
   public:
    const char* name() const noexcept override
    {
      return MUSICA_ERROR_CATEGORY;
    }
    std::string message(int ev) const override
    {
      switch (static_cast<MusicaErrc>(ev))
      {
        case MusicaErrc::SpeciesNotFound: return "Species not found";
        case MusicaErrc::SolverTypeNotFound: return "Solver type not found";
        case MusicaErrc::MappingNotFound: return "Mapping not found";
        case MusicaErrc::MappingOptionsUndefined: return "Mapping options undefined";
        case MusicaErrc::Unknown: return "Unknown error";
        default: return "Unknown error";
      }
    }
  };

  const MusicaErrorCategory MUSICA_ERROR{};
}  // namespace

inline std::error_code make_error_code(MusicaErrc e)
{
  return { static_cast<int>(e), MUSICA_ERROR };
}


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
    void Solve(
        MICM *micm,
        musica::State *state,
        double time_step,
        String *solver_state,
        SolverResultStats *solver_stats);
    
    /// @brief Get a property for a chemical species
    /// @param species_name Name of the species
    /// @param property_name Name of the property
    /// @return Value of the property
    template<class T>
    T GetSpeciesProperty(const std::string &species_name, const std::string &property_name)
    {
      micm::System system = std::visit([](auto &solver) -> micm::System { return solver->GetSystem(); }, solver_variant_);
      for (const auto &species : system.gas_phase_.species_)
      {
        if (species.name_ == species_name)
        {
          return species.GetProperty<T>(property_name);
        }
      }
      throw std::system_error(make_error_code(MusicaErrc::SpeciesNotFound), "Species '" + species_name + "' not found");
    }
  };

}  // namespace musica
