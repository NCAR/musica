// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/miem/emissions_c_interface.hpp>

#include <stdexcept>
#include <string>
#include <vector>

namespace musica
{
  namespace
  {
    void RequirePointer(const void* pointer, const char* name)
    {
      if (pointer == nullptr)
      {
        throw Exception(MiemErrorCode::NullPointer, std::string(name) + " pointer is null.");
      }
    }

    std::vector<std::string> OrderedNames(const Mappings* mappings)
    {
      if (mappings == nullptr || mappings->size_ == 0)
      {
        return {};
      }
      RequirePointer(mappings->mappings_, "Diagnostic-sector mappings");

      std::vector<std::string> names(mappings->size_);
      std::vector<bool> populated(mappings->size_, false);
      for (std::size_t i = 0; i < mappings->size_; ++i)
      {
        const Mapping& mapping = mappings->mappings_[i];
        RequirePointer(mapping.name_.value_, "Diagnostic-sector name");
        if (mapping.index_ >= mappings->size_ || populated[mapping.index_])
        {
          throw std::invalid_argument("Diagnostic-sector mappings must contain each zero-based index exactly once.");
        }
        names[mapping.index_] = mapping.name_.value_;
        populated[mapping.index_] = true;
      }
      return names;
    }

    void FillMappings(const std::vector<std::string>& names, Mappings* mappings)
    {
      RequirePointer(mappings, "Mappings output");
      mappings->mappings_ = new Mapping[names.size()];
      mappings->size_ = names.size();
      for (std::size_t i = 0; i < names.size(); ++i)
      {
        ToMapping(names[i].c_str(), i, &mappings->mappings_[i]);
      }
    }

    template<typename T>
    T* VectorPointer(const std::vector<T>& values, size_t* array_size)
    {
      RequirePointer(array_size, "Array-size output");
      *array_size = values.size();
      return const_cast<T*>(values.data());
    }

    const miem::InventoryGridMetadata& RequireGridMetadata(EmissionsModel* emissions)
    {
      RequirePointer(emissions, "EmissionsModel");
      if (!emissions->HasInventoryGridMetadata())
      {
        throw std::runtime_error("Inventory grid metadata is unavailable before a successful exact-grid run.");
      }
      return emissions->GridMetadata();
    }

    const char* GridFieldName(int field)
    {
      switch (field)
      {
        case MUSICA_EMISSIONS_GRID_FIELD_AREA_CELL: return "areaCell";
        case MUSICA_EMISSIONS_GRID_FIELD_LAT_CELL: return "latCell";
        case MUSICA_EMISSIONS_GRID_FIELD_LON_CELL: return "lonCell";
        case MUSICA_EMISSIONS_GRID_FIELD_X_CELL: return "xCell";
        case MUSICA_EMISSIONS_GRID_FIELD_Y_CELL: return "yCell";
        case MUSICA_EMISSIONS_GRID_FIELD_Z_CELL: return "zCell";
        default: throw std::invalid_argument("Unknown emissions grid field ID.");
      }
    }

    int GridGeometryValue(miem::InventoryGridGeometry geometry)
    {
      switch (geometry)
      {
        case miem::InventoryGridGeometry::Planar: return MUSICA_EMISSIONS_GRID_GEOMETRY_PLANAR;
        case miem::InventoryGridGeometry::Spherical: return MUSICA_EMISSIONS_GRID_GEOMETRY_SPHERICAL;
        case miem::InventoryGridGeometry::Unknown: return MUSICA_EMISSIONS_GRID_GEOMETRY_UNKNOWN;
      }
      return MUSICA_EMISSIONS_GRID_GEOMETRY_UNKNOWN;
    }
  }  // namespace

  EmissionsModel* CreateEmissions(const Mechanism* mechanism, int n_cells, int n_vert_levels, Error* error)
  {
    return HandleErrors(
        [&]() -> EmissionsModel*
        {
          RequirePointer(mechanism, "Mechanism");
          EmissionsModel* emissions = new EmissionsModel(EmissionsModel::FromMechanism(*mechanism, n_cells, n_vert_levels));
          NoError(error);
          return emissions;
        },
        error);
  }

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
      Error* error)
  {
    return HandleErrors(
        [&]() -> EmissionsModel*
        {
          RequirePointer(mechanism, "Mechanism");
          if (n_selected_global_cell_ids == 0)
          {
            throw std::invalid_argument("CreateEmissionsSelected requires at least one selected global cell ID.");
          }
          RequirePointer(selected_global_cell_ids, "Selected global cell IDs");
          if (n_diagnostic_categories > 0)
          {
            RequirePointer(diagnostic_category_ids, "Diagnostic category IDs");
          }

          EmissionsOptions options;
          options.global_n_cells = global_n_cells;
          options.n_vert_levels = n_vert_levels;
          options.selected_global_cell_ids.assign(
              selected_global_cell_ids,
              selected_global_cell_ids + n_selected_global_cell_ids);
          options.diagnostics.sectors_ = OrderedNames(diagnostic_sectors);
          if (n_diagnostic_categories > 0)
          {
            options.diagnostics.categories_.assign(
                diagnostic_category_ids,
                diagnostic_category_ids + n_diagnostic_categories);
          }
          options.diagnostics.layered_output_ = layered_diagnostics != 0;
          options.diagnostics.max_fields_ = max_diagnostic_fields;

          EmissionsModel* emissions = new EmissionsModel(EmissionsModel::FromMechanism(*mechanism, options));
          NoError(error);
          return emissions;
        },
        error);
  }

  void DeleteEmissions(EmissionsModel* emissions, Error* error)
  {
    HandleErrors(
        [&]()
        {
          delete emissions;
          NoError(error);
        },
        error);
  }

  void EmissionsRun(EmissionsModel* emissions, double epoch_seconds, double dt_seconds, Error* error)
  {
    HandleErrors(
        [&]()
        {
          RequirePointer(emissions, "EmissionsModel");
          emissions->Run(epoch_seconds, dt_seconds);
          NoError(error);
        },
        error);
  }

  int GetNumSpecies(EmissionsModel* emissions, Error* error)
  {
    return HandleErrors(
        [&]() -> int
        {
          RequirePointer(emissions, "EmissionsModel");
          int n = emissions->NumSpecies();
          NoError(error);
          return n;
        },
        error);
  }

  int GetEmissionsNumGlobalCells(EmissionsModel* emissions, Error* error)
  {
    return HandleErrors(
        [&]() -> int
        {
          RequirePointer(emissions, "EmissionsModel");
          const int value = emissions->NumGlobalCells();
          NoError(error);
          return value;
        },
        error);
  }

  int GetEmissionsNumCells(EmissionsModel* emissions, Error* error)
  {
    return HandleErrors(
        [&]() -> int
        {
          RequirePointer(emissions, "EmissionsModel");
          const int value = emissions->NumCells();
          NoError(error);
          return value;
        },
        error);
  }

  int GetEmissionsNumVertLevels(EmissionsModel* emissions, Error* error)
  {
    return HandleErrors(
        [&]() -> int
        {
          RequirePointer(emissions, "EmissionsModel");
          const int value = emissions->NumVertLevels();
          NoError(error);
          return value;
        },
        error);
  }

  int* GetEmissionsSelectedGlobalCellIdsPointer(EmissionsModel* emissions, size_t* array_size, Error* error)
  {
    return HandleErrors(
        [&]() -> int*
        {
          RequirePointer(emissions, "EmissionsModel");
          int* result = VectorPointer(emissions->SelectedGlobalCellIds(), array_size);
          NoError(error);
          return result;
        },
        error);
  }

  void GetEmissionsSpeciesOrdering(EmissionsModel* emissions, Mappings* species_ordering, Error* error)
  {
    HandleErrors(
        [&]()
        {
          RequirePointer(emissions, "EmissionsModel");
          FillMappings(emissions->SpeciesNames(), species_ordering);
          NoError(error);
        },
        error);
  }

  double* GetSurfaceFluxPointer(EmissionsModel* emissions, size_t* array_size, Error* error)
  {
    return HandleErrors(
        [&]() -> double*
        {
          RequirePointer(emissions, "EmissionsModel");
          double* result = VectorPointer(emissions->SurfaceFluxData(), array_size);
          NoError(error);
          return result;
        },
        error);
  }

  double* GetLayerFluxPointer(EmissionsModel* emissions, size_t* array_size, Error* error)
  {
    return HandleErrors(
        [&]() -> double*
        {
          RequirePointer(emissions, "EmissionsModel");
          double* result = VectorPointer(emissions->LayerFluxData(), array_size);
          NoError(error);
          return result;
        },
        error);
  }

  void GetSurfaceFluxStrides(EmissionsModel* emissions, size_t* cell_stride, size_t* species_stride, Error* error)
  {
    HandleErrors(
        [&]()
        {
          RequirePointer(emissions, "EmissionsModel");
          RequirePointer(cell_stride, "Cell-stride output");
          RequirePointer(species_stride, "Species-stride output");
          *cell_stride = 1;
          *species_stride = static_cast<size_t>(emissions->NumCells());
          NoError(error);
        },
        error);
  }

  void GetLayerFluxStrides(
      EmissionsModel* emissions,
      size_t* cell_stride,
      size_t* level_stride,
      size_t* species_stride,
      Error* error)
  {
    HandleErrors(
        [&]()
        {
          RequirePointer(emissions, "EmissionsModel");
          RequirePointer(cell_stride, "Cell-stride output");
          RequirePointer(level_stride, "Level-stride output");
          RequirePointer(species_stride, "Species-stride output");
          *cell_stride = 1;
          *level_stride = static_cast<size_t>(emissions->NumCells());
          *species_stride = static_cast<size_t>(emissions->NumCells()) * emissions->NumVertLevels();
          NoError(error);
        },
        error);
  }

  void GetEmissionsSectorOrdering(EmissionsModel* emissions, Mappings* sector_ordering, Error* error)
  {
    HandleErrors(
        [&]()
        {
          RequirePointer(emissions, "EmissionsModel");
          FillMappings(emissions->DiagnosticSectorNames(), sector_ordering);
          NoError(error);
        },
        error);
  }

  int* GetEmissionsCategoryIdsPointer(EmissionsModel* emissions, size_t* array_size, Error* error)
  {
    return HandleErrors(
        [&]() -> int*
        {
          RequirePointer(emissions, "EmissionsModel");
          int* result = VectorPointer(emissions->DiagnosticCategoryIds(), array_size);
          NoError(error);
          return result;
        },
        error);
  }

  double* GetSectorFluxPointer(EmissionsModel* emissions, size_t sector_index, size_t* array_size, Error* error)
  {
    return HandleErrors(
        [&]() -> double*
        {
          RequirePointer(emissions, "EmissionsModel");
          const auto& sector = emissions->DiagnosticSectorNames().at(sector_index);
          double* result = VectorPointer(emissions->SectorFluxData(sector), array_size);
          NoError(error);
          return result;
        },
        error);
  }

  double* GetCategoryFluxPointer(EmissionsModel* emissions, size_t category_index, size_t* array_size, Error* error)
  {
    return HandleErrors(
        [&]() -> double*
        {
          RequirePointer(emissions, "EmissionsModel");
          const int category = emissions->DiagnosticCategoryIds().at(category_index);
          double* result = VectorPointer(emissions->CategoryFluxData(category), array_size);
          NoError(error);
          return result;
        },
        error);
  }

  double* GetSectorLayerFluxPointer(EmissionsModel* emissions, size_t sector_index, size_t* array_size, Error* error)
  {
    return HandleErrors(
        [&]() -> double*
        {
          RequirePointer(emissions, "EmissionsModel");
          const auto& sector = emissions->DiagnosticSectorNames().at(sector_index);
          double* result = VectorPointer(emissions->SectorLayerFluxData(sector), array_size);
          NoError(error);
          return result;
        },
        error);
  }

  double* GetCategoryLayerFluxPointer(
      EmissionsModel* emissions,
      size_t category_index,
      size_t* array_size,
      Error* error)
  {
    return HandleErrors(
        [&]() -> double*
        {
          RequirePointer(emissions, "EmissionsModel");
          const int category = emissions->DiagnosticCategoryIds().at(category_index);
          double* result = VectorPointer(emissions->CategoryLayerFluxData(category), array_size);
          NoError(error);
          return result;
        },
        error);
  }

  void GetEmissionsGridMetadata(EmissionsModel* emissions, EmissionsGridMetadata* metadata, Error* error)
  {
    HandleErrors(
        [&]()
        {
          RequirePointer(metadata, "Grid-metadata output");
          const auto& source = RequireGridMetadata(emissions);
          DeleteEmissionsGridMetadata(metadata);
          metadata->available_ = source.available_ ? 1 : 0;
          metadata->exact_grid_ = source.IsExactGrid() ? 1 : 0;
          metadata->global_n_cells_ = source.global_n_cells_;
          metadata->geometry_ = GridGeometryValue(source.geometry_);
          metadata->has_sphere_radius_ = source.has_sphere_radius_ ? 1 : 0;
          metadata->sphere_radius_ = source.sphere_radius_;
          for (int field = MUSICA_EMISSIONS_GRID_FIELD_AREA_CELL;
               field <= MUSICA_EMISSIONS_GRID_FIELD_Z_CELL;
               ++field)
          {
            if (source.FindField(GridFieldName(field)) != nullptr)
            {
              metadata->field_mask_ |= std::uint32_t{ 1 } << field;
            }
          }
          CreateString(source.on_a_sphere_.c_str(), &metadata->on_a_sphere_);
          CreateString(source.is_periodic_.c_str(), &metadata->is_periodic_);
          CreateString(source.fingerprint_algorithm_.c_str(), &metadata->fingerprint_algorithm_);
          CreateString(source.fingerprint_.c_str(), &metadata->fingerprint_);
          CreateString(source.field_manifest_.c_str(), &metadata->field_manifest_);
          CreateString(source.index_to_cell_id_units_.c_str(), &metadata->index_to_cell_id_units_);
          NoError(error);
        },
        error);
  }

  void DeleteEmissionsGridMetadata(EmissionsGridMetadata* metadata)
  {
    if (metadata == nullptr)
    {
      return;
    }
    DeleteString(&metadata->on_a_sphere_);
    DeleteString(&metadata->is_periodic_);
    DeleteString(&metadata->fingerprint_algorithm_);
    DeleteString(&metadata->fingerprint_);
    DeleteString(&metadata->field_manifest_);
    DeleteString(&metadata->index_to_cell_id_units_);
    metadata->available_ = 0;
    metadata->exact_grid_ = 0;
    metadata->global_n_cells_ = 0;
    metadata->geometry_ = MUSICA_EMISSIONS_GRID_GEOMETRY_UNKNOWN;
    metadata->has_sphere_radius_ = 0;
    metadata->sphere_radius_ = 0.0;
    metadata->field_mask_ = 0;
  }

  std::int64_t* GetEmissionsGridIndexToCellIdPointer(
      EmissionsModel* emissions,
      size_t* array_size,
      Error* error)
  {
    return HandleErrors(
        [&]() -> std::int64_t*
        {
          const auto& metadata = RequireGridMetadata(emissions);
          std::int64_t* result = VectorPointer(metadata.index_to_cell_id_, array_size);
          NoError(error);
          return result;
        },
        error);
  }

  double* GetEmissionsGridFieldPointer(EmissionsModel* emissions, int field, size_t* array_size, Error* error)
  {
    return HandleErrors(
        [&]() -> double*
        {
          const auto& metadata = RequireGridMetadata(emissions);
          const auto* grid_field = metadata.FindField(GridFieldName(field));
          if (grid_field == nullptr)
          {
            throw std::invalid_argument(std::string("Inventory grid field is unavailable: ") + GridFieldName(field));
          }
          double* result = VectorPointer(grid_field->values_, array_size);
          NoError(error);
          return result;
        },
        error);
  }

  void GetEmissionsGridFieldUnits(EmissionsModel* emissions, int field, String* units, Error* error)
  {
    HandleErrors(
        [&]()
        {
          RequirePointer(units, "Grid-field units output");
          const auto& metadata = RequireGridMetadata(emissions);
          const auto* grid_field = metadata.FindField(GridFieldName(field));
          if (grid_field == nullptr)
          {
            throw std::invalid_argument(std::string("Inventory grid field is unavailable: ") + GridFieldName(field));
          }
          DeleteString(units);
          CreateString(grid_field->units_.c_str(), units);
          NoError(error);
        },
        error);
  }
}  // namespace musica
