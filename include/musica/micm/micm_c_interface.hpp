// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the defintion of the MICM class, which represents a multi-component reactive transport model.
// It also includes functions for creating and deleting MICM instances with c bindings.
#pragma once

#include <musica/micm/chemistry.hpp>
#include <musica/micm/micm.hpp>
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
#ifdef __cplusplus
  extern "C"
  {
#endif
    /// @brief Create a MICM object by specifying solver type to use and providing a path to the configuration file
    /// @param config_path Path to configuration file or directory containing configuration file
    /// @param solver_type Type of MICMSolver
    /// @param error Error struct to indicate success or failure
    /// @return Pointer to MICM object
    MICM *CreateMicm(const char *config_path, MICMSolver solver_type, Error *error);

    /// @brief Create a MICM object by specifying the solver type and providing a Chemistry object
    /// @param chemistry Chemistry object
    /// @param solver_type Type of MICMSolver
    /// @param error Error struct to indicate success or failure
    /// @return Pointer to MICM object
    MICM *CreateMicmFromChemistryMechanism(const Chemistry *chemistry, MICMSolver solver_type, Error *error);

    /// @brief Deletes a MICM object
    /// @param micm Pointer to MICM object
    /// @param error Error struct to indicate success or failure
    void DeleteMicm(const MICM *micm, Error *error);

    /// @brief Solve the system
    /// @param micm Pointer to MICM object
    /// @param state Pointer to state object
    /// @param time_step Time [s] to advance the state by
    /// @param solver_state State of the solver
    /// @param solver_stats Statistics of the solver
    /// @param error Error struct to indicate success or failure
    void MicmSolve(
        MICM *micm,
        musica::State *state,
        double time_step,
        String *solver_state,
        SolverResultStats *solver_stats,
        Error *error);

    /// @brief Get the MICM version
    /// @return MICM version
    String MicmVersion();

    /// @brief Get a property for a chemical species
    /// @param micm Pointer to MICM object
    /// @param species_name Name of the species
    /// @param property_name Name of the property
    /// @param error Error struct to indicate success or failure
    /// @return Value of the property
    String GetSpeciesPropertyString(MICM *micm, const char *species_name, const char *property_name, Error *error);
    double GetSpeciesPropertyDouble(MICM *micm, const char *species_name, const char *property_name, Error *error);
    int GetSpeciesPropertyInt(MICM *micm, const char *species_name, const char *property_name, Error *error);
    bool GetSpeciesPropertyBool(MICM *micm, const char *species_name, const char *property_name, Error *error);

    /// @brief Get the maximum number of grid cells per state
    /// @param micm Pointer to MICM object
    /// @return Maximum number of grid cells
    size_t GetMaximumNumberOfGridCells(MICM *micm);

    bool _IsCudaAvailable(Error *error);
#ifdef __cplusplus
  }
#endif
}  // namespace musica
