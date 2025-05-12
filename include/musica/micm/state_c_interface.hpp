// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the State class.It includes state representations for different
// solver configurations, leveraging both vector-ordered and standard-ordered state type.
// It also includes functions for creating and deleting State instances with c bindings.
#pragma once

#include <musica/micm/micm.hpp>
#include <musica/micm/state.hpp>
#include <musica/util.hpp>

namespace musica
{
#ifdef __cplusplus
  extern "C"
  {
#endif
    /// @brief Create a state object by specifying micm solver object using the solver variant
    /// @param micm Pointer to MICM object
    /// @param number_of_grid_cells Number of grid cells
    /// @param error Error struct to indicate success or failure
    State* CreateMicmState(musica::MICM* micm, size_t number_of_grid_cells, Error* error);

    /// @brief Deletes a state object
    /// @param state Pointer to state object
    /// @param error Error struct to indicate success or failure
    void DeleteState(const State* state, Error* error);

    /// @brief Get the pointer to the conditions struct
    /// @param state Pointer to state object
    /// @param number_of_grid_cells Pointer to num of grid cells
    /// @param error Error struct to indicate success or failure
    micm::Conditions* GetConditionsPointer(musica::State* state, int* number_of_grid_cells, Error* error);

    /// @brief Get the point to the vector of the concentrations for Fortran interface
    /// @param state Pointer to state object
    /// @param number_of_species Pointer to number of species
    /// @param number_of_grid_cells Pointer to num of grid cells
    /// @param error Error struct to indicate success or failure
    /// @return Pointer to the vector
    double* GetOrderedConcentrationsPointer(
        musica::State* state,
        int* number_of_species,
        int* number_of_grid_cells,
        Error* error);

    /// @brief Get the point to the vector of the rates for Fortran interface
    /// @param state Pointer to state object
    /// @param number_of_rate_constants Pointer to number of rate constants
    /// @param number_of_grid_cells Pointer to num of grid cells
    /// @param error Error struct to indicate success or failure
    /// @return Pointer to the vector
    double* GetOrderedRateConstantsPointer(
        musica::State* state,
        int* number_of_rate_constants,
        int* number_of_grid_cells,
        Error* error);

    /// @brief Get the ordering of species
    /// @param state Pointer to state object
    /// @param error Error struct to indicate success or failure
    /// @return Array of species' name-index pairs
    Mappings GetSpeciesOrdering(musica::State* state, Error* error);

    /// @brief Get the ordering of user-defined reaction rates
    /// @param state Pointer to state object
    /// @param error Error struct to indicate success or failure
    /// @return Array of reaction rate name-index pairs
    Mappings GetUserDefinedReactionRatesOrdering(musica::State* state, Error* error);

    /// @brief Returns the number of grid cells in the solver state
    /// @param state Pointer to state object
    /// @param error Error struct to indicate success or failure
    /// @return Number of grid cells
    size_t GetNumberOfGridCells(musica::State* state, Error* error);

    /// @brief Returns the number of species in the solver state
    /// @param state Pointer to state object
    /// @param error Error struct to indicate success or failure
    /// @return Number of species
    size_t GetNumberOfSpecies(musica::State* state, Error* error);

    /// @brief Returns the stride across grid cells for the concentration matrix
    /// @param state Pointer to state object
    /// @param error Error struct to indicate success or failure
    /// @param grid_cell_stride Pointer to the stride across grid cells
    /// @param species_stride Pointer to the stride across species
    void GetConcentrationStrides(musica::State* state, Error* error, size_t* grid_cell_stride, size_t* species_stride); 

    /// @brief Returns the number of user-defined rate parameters
    /// @param state Pointer to state object
    /// @param error Error struct to indicate success or failure
    /// @return Number of user-defined rate parameters
    size_t GetNumberOfUserDefinedRateParameters(musica::State* state, Error* error);

    /// @brief Returns the stride across grid cells for the user-defined rate parameter matrix
    /// @param state Pointer to state object
    /// @param error Error struct to indicate success or failure
    /// @param grid_cell_stride Pointer to the stride across grid cells
    /// @param user_defined_rate_parameter_stride Pointer to the stride across user-defined rate parameters
    void GetUserDefinedRateParameterStrides(musica::State* state, Error* error, 
                                            size_t* grid_cell_stride,
                                            size_t* user_defined_rate_parameter_stride);

#ifdef __cplusplus
  }
#endif
}  // namespace musica