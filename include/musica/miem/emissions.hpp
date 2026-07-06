// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <miem/source_types.hpp>

#include <vector>

namespace musica
{
  struct Emissions
  {
    std::vector<miem::Source> sources;
    miem::Regridding regridding;
  };
}  // namespace musica
