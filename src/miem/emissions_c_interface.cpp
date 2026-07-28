// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/miem/emissions_c_interface.hpp>

#include <string>

namespace musica
{
  EmissionsModel* CreateEmissions(const Mechanism* mechanism, int n_cells, int n_vert_levels, Error* error)
  {
    return HandleErrors(
        [&]() -> EmissionsModel*
        {
          if (!mechanism)
          {
            std::string const msg = "Mechanism pointer is null, cannot create EmissionsModel.";
            ToError(MUSICA_MIEM_ERROR_CATEGORY, MUSICA_MIEM_ERROR_CODE_NULL_POINTER, msg.c_str(), MUSICA_SEVERITY_CRITICAL, error);
            return nullptr;
          }
          EmissionsModel* emissions = new EmissionsModel(EmissionsModel::FromMechanism(*mechanism, n_cells, n_vert_levels));
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
          int n = emissions->NumSpecies();
          NoError(error);
          return n;
        },
        error);
  }

  void GetEmissionsSpeciesOrdering(EmissionsModel* emissions, Mappings* species_ordering, Error* error)
  {
    HandleErrors(
        [&]()
        {
          const auto& names = emissions->SpeciesNames();
          species_ordering->mappings_ = new Mapping[names.size()];
          species_ordering->size_ = names.size();
          for (std::size_t i = 0; i < names.size(); ++i)
          {
            ToMapping(names[i].c_str(), i, &species_ordering->mappings_[i]);
          }
          NoError(error);
        },
        error);
  }

  double* GetSurfaceFluxPointer(EmissionsModel* emissions, size_t* array_size, Error* error)
  {
    return HandleErrors(
        [&]() -> double*
        {
          const std::vector<double>& flux = emissions->SurfaceFluxData();
          *array_size = flux.size();
          NoError(error);
          return const_cast<double*>(flux.data());
        },
        error);
  }

  void GetSurfaceFluxStrides(EmissionsModel* emissions, Error* error, size_t* cell_stride, size_t* species_stride)
  {
    HandleErrors(
        [&]()
        {
          *cell_stride = 1;
          *species_stride = static_cast<size_t>(emissions->NumCells());
          NoError(error);
        },
        error);
  }
}  // namespace musica
