// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the extern "C" interface to musica::EmissionsModel, so that
// C/Fortran callers can create, run, and query an emissions module.
#pragma once

#include <musica/configuration/read_mechanism_c_interface.hpp>
#include <musica/emissions/emissions.hpp>
#include <musica/utils/util.hpp>

namespace musica
{
#ifdef __cplusplus
  extern "C"
  {
#endif
    /// @brief Pointers to the environment buffers that the EmissionsModel owns.
    /// The caller sets size to sizeof(EmissionsEnvironmentPointers) before the call.
    /// New fields go only at the end. A null field is not used by the configured sources.
    typedef struct EmissionsEnvironmentPointers
    {
      size_t size;
    } EmissionsEnvironmentPointers;

    /// @brief Create an EmissionsModel from a parsed Mechanism's emissions section
    /// @param mechanism Pointer to a Mechanism (e.g. from ReadMechanismC / ReadMechanismFromStringC)
    /// @param n_cells Number of horizontal grid cells
    /// @param n_vert_levels Number of vertical levels
    /// @param error Error struct to indicate success or failure
    /// @return Pointer to EmissionsModel object
    EmissionsModel* CreateEmissions(const Mechanism* mechanism, int n_cells, int n_vert_levels, Error* error);

    /// @brief Deletes an EmissionsModel object
    /// @param emissions Pointer to EmissionsModel object
    /// @param error Error struct to indicate success or failure
    void DeleteEmissions(EmissionsModel* emissions, Error* error);

    /// @brief Advance one time step
    /// @param emissions Pointer to EmissionsModel object
    /// @param epoch_seconds Simulation time as seconds since epoch
    /// @param dt_seconds Time step [s]
    /// @param error Error struct to indicate success or failure
    void EmissionsRun(EmissionsModel* emissions, double epoch_seconds, double dt_seconds, Error* error);

    /// @brief Get the number of aggregated mechanism species across all sources
    /// @param emissions Pointer to EmissionsModel object
    /// @param error Error struct to indicate success or failure
    /// @return Number of species
    int GetNumSpecies(EmissionsModel* emissions, Error* error);

    /// @brief Get the ordering of aggregated mechanism species
    ///
    /// Named GetEmissionsSpeciesOrdering, not GetSpeciesOrdering, to avoid a
    /// link collision with state_c_interface.hpp's own GetSpeciesOrdering.
    /// @param emissions Pointer to EmissionsModel object [input]
    /// @param species_ordering Array of species' name-index pairs [output]
    /// @param error Error struct to indicate success or failure [output]
    void GetEmissionsSpeciesOrdering(EmissionsModel* emissions, Mappings* species_ordering, Error* error);

    /// @brief Get the flattened (n_species * n_cells) surface flux buffer from the most
    /// recent EmissionsRun() call, species-major: index = species_idx * species_stride + cell_idx * cell_stride.
    /// Valid only until the next EmissionsRun() call.
    /// @param emissions Pointer to EmissionsModel object
    /// @param array_size Overall size of the array (output)
    /// @param error Error struct to indicate success or failure
    /// @return Pointer to the flux buffer [kg m-2 s-1]
    double* GetSurfaceFluxPointer(EmissionsModel* emissions, size_t* array_size, Error* error);

    /// @brief Get the strides for indexing the surface flux buffer
    /// @param emissions Pointer to EmissionsModel object
    /// @param cell_stride Pointer to the stride across grid cells
    /// @param species_stride Pointer to the stride across species
    /// @param error Error struct to indicate success or failure
    void GetSurfaceFluxStrides(EmissionsModel* emissions, size_t* cell_stride, size_t* species_stride, Error* error);

    /// @brief Get the flattened (n_species * n_levels * n_cells) flux buffer for each vertical level
    /// from the most recent EmissionsRun() call. Level 0 is the surface.
    /// index = species_idx * species_stride + level_idx * level_stride + cell_idx * cell_stride.
    /// The buffer address does not change between EmissionsRun() calls.
    /// @param emissions Pointer to EmissionsModel object
    /// @param array_size Overall size of the array (output)
    /// @param error Error struct to indicate success or failure
    /// @return Pointer to the flux buffer [kg m-2 s-1]
    double* GetFlux3DPointer(EmissionsModel* emissions, size_t* array_size, Error* error);

    /// @brief Get the strides for indexing the flux buffer for each vertical level
    /// @param emissions Pointer to EmissionsModel object
    /// @param cell_stride Pointer to the stride across grid cells
    /// @param level_stride Pointer to the stride across vertical levels
    /// @param species_stride Pointer to the stride across species
    /// @param error Error struct to indicate success or failure
    void GetFlux3DStrides(
        EmissionsModel* emissions,
        size_t* cell_stride,
        size_t* level_stride,
        size_t* species_stride,
        Error* error);

    /// @brief Get pointers to the environment buffers. The caller writes the buffers before EmissionsRun().
    /// @param emissions Pointer to EmissionsModel object
    /// @param environment Struct to fill; the caller sets environment->size first
    /// @param error Error struct to indicate success or failure
    void GetEmissionsEnvironment(EmissionsModel* emissions, EmissionsEnvironmentPointers* environment, Error* error);
#ifdef __cplusplus
  }
#endif
}  // namespace musica
