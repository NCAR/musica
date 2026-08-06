// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the extern "C" interface to musica::EmissionsModel, so that
// C/Fortran callers can create, run, and query an emissions module.
#pragma once

#include <musica/configuration/read_mechanism_c_interface.hpp>
#include <musica/miem/emissions.hpp>
#include <musica/utils/util.hpp>

#include <cstdint>

namespace musica
{
  /// @brief C-interoperable inventory geometry values.
  enum EmissionsGridGeometry
  {
    MUSICA_EMISSIONS_GRID_GEOMETRY_UNKNOWN = 0,
    MUSICA_EMISSIONS_GRID_GEOMETRY_PLANAR = 1,
    MUSICA_EMISSIONS_GRID_GEOMETRY_SPHERICAL = 2,
  };

  /// @brief IDs accepted by GetEmissionsGridFieldPointer/Units.
  enum EmissionsGridField
  {
    MUSICA_EMISSIONS_GRID_FIELD_AREA_CELL = 0,
    MUSICA_EMISSIONS_GRID_FIELD_LAT_CELL = 1,
    MUSICA_EMISSIONS_GRID_FIELD_LON_CELL = 2,
    MUSICA_EMISSIONS_GRID_FIELD_X_CELL = 3,
    MUSICA_EMISSIONS_GRID_FIELD_Y_CELL = 4,
    MUSICA_EMISSIONS_GRID_FIELD_Z_CELL = 5,
  };

  /// @brief C-interoperable immutable exact-grid metadata descriptor.
  ///
  /// Initialize this struct to zero before GetEmissionsGridMetadata and call
  /// DeleteEmissionsGridMetadata when finished with its owned String fields.
  struct EmissionsGridMetadata
  {
    int available_ = 0;
    int exact_grid_ = 0;
    int global_n_cells_ = 0;
    int geometry_ = MUSICA_EMISSIONS_GRID_GEOMETRY_UNKNOWN;
    int has_sphere_radius_ = 0;
    double sphere_radius_ = 0.0;
    std::uint32_t field_mask_ = 0;
    String on_a_sphere_;
    String is_periodic_;
    String fingerprint_algorithm_;
    String fingerprint_;
    String field_manifest_;
    String index_to_cell_id_units_;
  };

#ifdef __cplusplus
  extern "C"
  {
#endif
    /// @brief Create an EmissionsModel from a parsed Mechanism's emissions section
    /// @param mechanism Pointer to a Mechanism (e.g. from ReadMechanismC / ReadMechanismFromStringC)
    /// @param n_cells Number of horizontal grid cells
    /// @param n_vert_levels Number of vertical levels
    /// @param error Error struct to indicate success or failure
    /// @return Pointer to EmissionsModel object
    EmissionsModel* CreateEmissions(const Mechanism* mechanism, int n_cells, int n_vert_levels, Error* error);

    /// @brief Create a rank-local EmissionsModel with bounded diagnostics.
    /// @param global_n_cells Full inventory grid cell count
    /// @param selected_global_cell_ids One-based inventory slots in caller output order
    /// @param n_selected_global_cell_ids Number of selected IDs (must be nonzero)
    /// @param diagnostic_sectors Optional name/index mapping in requested output order
    /// @param diagnostic_category_ids Optional category IDs in requested output order
    /// @param n_diagnostic_categories Number of category IDs
    /// @param layered_diagnostics Nonzero to allocate selected layered diagnostic buffers
    /// @param max_diagnostic_fields Hard cap on species * groups * (layered ? levels : 1)
    EmissionsModel* CreateEmissionsSelected(
        const Mechanism* mechanism,
        int global_n_cells,
        int n_vert_levels,
        const int* selected_global_cell_ids,
        size_t n_selected_global_cell_ids,
        const Mappings* diagnostic_sectors,
        const int* diagnostic_category_ids,
        size_t n_diagnostic_categories,
        int layered_diagnostics,
        size_t max_diagnostic_fields,
        Error* error);

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

    int GetEmissionsNumGlobalCells(EmissionsModel* emissions, Error* error);
    int GetEmissionsNumCells(EmissionsModel* emissions, Error* error);
    int GetEmissionsNumVertLevels(EmissionsModel* emissions, Error* error);

    /// @brief Get the ordered selected one-based global IDs.
    /// @return Pointer owned by emissions and valid for its lifetime
    int* GetEmissionsSelectedGlobalCellIdsPointer(EmissionsModel* emissions, size_t* array_size, Error* error);

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

    /// @brief Get the species/level/cell layer flux buffer from the last run.
    double* GetLayerFluxPointer(EmissionsModel* emissions, size_t* array_size, Error* error);

    /// @brief Get the strides for indexing the surface flux buffer
    /// @param emissions Pointer to EmissionsModel object
    /// @param cell_stride Pointer to the stride across grid cells
    /// @param species_stride Pointer to the stride across species
    /// @param error Error struct to indicate success or failure
    void GetSurfaceFluxStrides(EmissionsModel* emissions, size_t* cell_stride, size_t* species_stride, Error* error);

    void GetLayerFluxStrides(
        EmissionsModel* emissions,
        size_t* cell_stride,
        size_t* level_stride,
        size_t* species_stride,
        Error* error);

    /// @brief Get bounded diagnostic ordering selected at construction.
    void GetEmissionsSectorOrdering(EmissionsModel* emissions, Mappings* sector_ordering, Error* error);
    int* GetEmissionsCategoryIdsPointer(EmissionsModel* emissions, size_t* array_size, Error* error);

    /// @brief Get one selected diagnostic buffer by zero-based group index.
    double* GetSectorFluxPointer(EmissionsModel* emissions, size_t sector_index, size_t* array_size, Error* error);
    double* GetCategoryFluxPointer(EmissionsModel* emissions, size_t category_index, size_t* array_size, Error* error);
    double* GetSectorLayerFluxPointer(EmissionsModel* emissions, size_t sector_index, size_t* array_size, Error* error);
    double* GetCategoryLayerFluxPointer(
        EmissionsModel* emissions,
        size_t category_index,
        size_t* array_size,
        Error* error);

    /// @brief Copy exact-grid scalar/string metadata after a successful run.
    void GetEmissionsGridMetadata(EmissionsModel* emissions, EmissionsGridMetadata* metadata, Error* error);
    void DeleteEmissionsGridMetadata(EmissionsGridMetadata* metadata);

    /// @brief Get selected exact-grid indexToCellID values in output order.
    std::int64_t* GetEmissionsGridIndexToCellIdPointer(
        EmissionsModel* emissions,
        size_t* array_size,
        Error* error);

    /// @brief Get a selected exact-grid field and its units by EmissionsGridField ID.
    double* GetEmissionsGridFieldPointer(
        EmissionsModel* emissions,
        int field,
        size_t* array_size,
        Error* error);
    void GetEmissionsGridFieldUnits(EmissionsModel* emissions, int field, String* units, Error* error);
#ifdef __cplusplus
  }
#endif
}  // namespace musica
