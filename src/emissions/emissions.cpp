// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the EmissionsModel class.
#include "miem_backend.hpp"

#include <musica/emissions/emissions.hpp>

namespace musica
{
  struct EmissionsModel::Impl
  {
    MiemBackend backend;
  };

  EmissionsModel::EmissionsModel(const mechanism_configuration::Mechanism& mechanism, int n_cells, int n_vert_levels)
      : impl_(std::make_unique<Impl>(Impl{ MiemBackend(mechanism, n_cells, n_vert_levels) }))
  {
  }

  EmissionsModel::~EmissionsModel() = default;
  EmissionsModel::EmissionsModel(EmissionsModel&&) noexcept = default;
  EmissionsModel& EmissionsModel::operator=(EmissionsModel&&) noexcept = default;

  void EmissionsModel::Run(double epoch_seconds, double dt_seconds)
  {
    impl_->backend.Run(epoch_seconds, dt_seconds);
  }

  const std::vector<std::string>& EmissionsModel::SpeciesNames() const
  {
    return impl_->backend.SpeciesNames();
  }

  SurfaceFluxView EmissionsModel::SurfaceFlux() const
  {
    return impl_->backend.SurfaceFlux();
  }

  Flux3DView EmissionsModel::Flux3D() const
  {
    return impl_->backend.Flux3D();
  }

}  // namespace musica
