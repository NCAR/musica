// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the defintion of the MICM class, which represents a multi-component reactive transport model.
// It also includes functions for creating and deleting MICM instances with c bindings.
#pragma once

#include <musica/micm/chemistry.hpp>
#include <musica/micm/parse.hpp>
#include <musica/micm/solver_interface.hpp>

#include <micm/solver/solver_result.hpp>
#include <micm/system/system.hpp>

#include <chrono>
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

enum class MusicaErrCode
{
  SpeciesNotFound = MUSICA_ERROR_CODE_SPECIES_NOT_FOUND,
  SolverTypeNotFound = MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND,
  MappingNotFound = MUSICA_ERROR_CODE_MAPPING_NOT_FOUND,
  MappingOptionsUndefined = MUSICA_ERROR_CODE_MAPPING_OPTIONS_UNDEFINED,
  Unknown = MUSICA_ERROR_CODE_UNKNOWN,
  UnsupportedSolverStatePair = MUSICA_ERROR_CODE_UNSUPPORTED_SOLVER_STATE_PAIR,
};

namespace std
{
  template<>
  struct is_error_condition_enum<MusicaErrCode> : true_type
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
      switch (static_cast<MusicaErrCode>(ev))
      {
        case MusicaErrCode::SpeciesNotFound: return "Species not found";
        case MusicaErrCode::SolverTypeNotFound: return "Solver type not found";
        case MusicaErrCode::MappingNotFound: return "Mapping not found";
        case MusicaErrCode::MappingOptionsUndefined: return "Mapping options undefined";
        case MusicaErrCode::Unknown: return "Unknown error";
        case MusicaErrCode::UnsupportedSolverStatePair: return "Unsupported solver/state combination";
        default: return "Unknown error";
      }
    }
  };

  const MusicaErrorCategory MUSICA_ERROR{};
}  // namespace

inline std::error_code make_error_code(MusicaErrCode e)
{
  return { static_cast<int>(e), MUSICA_ERROR };
}

namespace musica
{
  class State;   // forward declaration to break circular include
  class IState;  // forward declaration for interface

  /// @brief Types of MICM solver
  enum MICMSolver
  {
    UndefinedSolver = 0,         // Undefined solver
    Rosenbrock,                  // Vector-ordered Rosenbrock solver
    RosenbrockStandardOrder,     // Standard-ordered Rosenbrock solver
    BackwardEuler,               // Vector-ordered BackwardEuler solver
    BackwardEulerStandardOrder,  // Standard-ordered BackwardEuler solver
    CudaRosenbrock,              // Cuda Rosenbrock solver
  };

  std::string ToString(MICMSolver solver_type);

  using SolverResultStats = micm::SolverStats;

  /// @brief Type-erased solver pointer that can hold both CPU and CUDA solvers
  /// with different deleters
  using SolverPtr = std::unique_ptr<IMicmSolver, std::function<void(IMicmSolver*)>>;

  class MICM
  {
   public:
    MICM(const Chemistry& chemistry, MICMSolver solver_type);
    MICM(std::string config_path, MICMSolver solver_type);
    MICM() = default;
    ~MICM();

    /// @brief Solve the system
    /// @param state Pointer to state object
    /// @param time_step Time [s] to advance the state by
    micm::SolverResult Solve(musica::State* state, double time_step);

    /// @brief Get a property for a chemical species
    /// @param species_name Name of the species
    /// @param property_name Name of the property
    /// @return Value of the property
    template<class T>
    T GetSpeciesProperty(const std::string& species_name, const std::string& property_name)
    {
      micm::System system = solver_->GetSystem();
      for (const auto& phase_species : system.gas_phase_.phase_species_)
      {
        const auto& species = phase_species.species_;
        if (species.name_ == species_name)
        {
          return species.GetProperty<T>(property_name);
        }
      }
      throw std::system_error(make_error_code(MusicaErrCode::SpeciesNotFound), "Species '" + species_name + "' not found");
    }

    /// @brief Get the maximum number of grid cells per state
    /// @return Maximum number of grid cells
    std::size_t GetMaximumNumberOfGridCells();

    /// @brief Create a new state object for this solver
    /// @param number_of_grid_cells Number of grid cells for the state
    /// @return Unique pointer to a new IState implementation
    std::unique_ptr<IState> CreateState(std::size_t number_of_grid_cells);

    /// @brief Get the chemical system configuration
    /// @return The MICM System object
    micm::System GetSystem() const;

    /// @brief Get the species ordering map
    /// @return Map of species names to their indices
    std::unordered_map<std::string, std::size_t> GetSpeciesOrdering() const;

    /// @brief Get the rate parameter ordering map
    /// @return Map of rate parameter names to their indices
    std::unordered_map<std::string, std::size_t> GetRateParameterOrdering() const;

    /// @brief Get the solver type
    /// @return The solver type enum value
    MICMSolver GetSolverType() const;

    /// @brief Get the vector size for this solver type
    /// @return Vector dimension for vector-ordered solvers, 1 for standard-ordered solvers
    std::size_t GetVectorSize() const;

    /// @brief Get access to the underlying solver interface
    /// @return Pointer to the IMicmSolver implementation
    IMicmSolver* GetSolverInterface();

   private:
    SolverPtr solver_;
    MICMSolver solver_type_ = UndefinedSolver;
  };

}  // namespace musica
