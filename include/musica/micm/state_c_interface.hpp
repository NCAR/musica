// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the State class.It includes state representations for different
// solver configurations, leveraging both vector-ordered and standard-ordered state type.
// It also includes functions for creating and deleting State instances with c bindings.
#pragma once

#include <musica/util.hpp>
#include <musica/micm/micm.hpp>
#include <musica/micm/state.hpp>

#ifndef MICM_VECTOR_MATRIX_SIZE
  #define MICM_VECTOR_MATRIX_SIZE 4
#endif

namespace musica
{
  /// @brief Create a state object by specifying micm solver object using the solver variant
  /// @param micm Pointer to MICM object
  /// @param error Error struct to indicate success or failure
  State *CreateMicmState(musica::MICM *micm, Error *error);

  /// @brief Deletes a state object
  /// @param state Pointer to state object
  /// @param error Error struct to indicate success or failure
  void DeleteState(const State *state, Error *error);
}  // namespace musica