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
    /// @brief Create a MICM object by specifying solver type to use
    /// @param config_path Path to configuration file or directory containing configuration file
    /// @param solver_type Type of MICMSolver
    /// @param num_grid_cells Number of grid cells
    /// @param error Error struct to indicate success or failure
    MICM *CreateMicm(const char *config_path, MICMSolver solver_type, int num_grid_cells, Error *error);

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

    /// @brief Get the ordering of species
    /// @param micm Pointer to MICM object
    /// @param state Pointer to state object
    /// @param error Error struct to indicate success or failure
    /// @return Array of species' name-index pairs
    Mappings GetSpeciesOrdering(MICM *micm, musica::State *state, Error *error);

    /// @brief Get the ordering of user-defined reaction rates
    /// @param micm Pointer to MICM object
    /// @param state Pointer to state object
    /// @param error Error struct to indicate success or failure
    /// @return Array of reaction rate name-index pairs
    Mappings GetUserDefinedReactionRatesOrdering(MICM *micm, musica::State *state, Error *error);

    /// @brief Temporary method to solve the system
    /// @param micm Pointer to MICM object
    /// @param time_step Time [s] to advance the state by
    /// @param temperature Temperature [grid cell] (K)
    /// @param pressure Pressure [grid cell] (Pa)
    /// @param air_density Air density [grid cell] (mol m-3)
    /// @param concentrations Array of species' concentrations [grid cell][species] (mol m-3)
    /// @param custom_rate_parameters Array of custom rate parameters [grid cell][parameter] (various units)
    /// @param solver_state State of the solver
    /// @param solver_stats Statistics of the solver
    /// @param error Error struct to indicate success or failure
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
        Error *error);

    /// @brief Temporary method to Get the ordering of species for Fortran
    /// @param micm Pointer to MICM object
    /// @param error Error struct to indicate success or failure
    /// @return Array of species' name-index pairs
    Mappings GetSpeciesOrderingFortran(MICM *micm, Error *error);

    /// @brief Temporary method to Get the ordering of user-defined reaction rates for Fortran
    /// @param micm Pointer to MICM object
    /// @param error Error struct to indicate success or failure
    /// @return Array of reaction rate name-index pairs
    Mappings GetUserDefinedReactionRatesOrderingFortran(MICM *micm, Error *error);

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
#ifdef __cplusplus
  }
#endif
}  // namespace musica
