// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the State class.It includes state representations for different
// solver configurations, leveraging both vector-ordered and standard-ordered state type.
// It also includes functions for creating and deleting State instances with c bindings.
#include <musica/micm/state_c_interface.hpp>

namespace musica
{
  State* CreateMicmState(musica::MICM* micm, Error* error)
  {
    DeleteError(error);
    if (!micm)
    {
      std::string msg = "MICM pointer is null, cannot create state.";
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND, msg.c_str());
      delete micm;
      return nullptr;
    }

    State* state = new State();

    std::visit([&](auto& solver_ptr) { state->state_variant_ = solver_ptr->GetState(); }, micm->solver_variant_);

    return state;
  }

  void DeleteState(const State* state, Error* error)
  {
    DeleteError(error);
    if (state == nullptr)
    {
      *error = NoError();
      return;
    }
    try
    {
      delete state;
      *error = NoError();
    }
    catch (const std::system_error& e)
    {
      *error = ToError(e);
    }
  }
}  // namespace musica
