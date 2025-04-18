// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the State class.It includes state representations for different
// solver configurations, leveraging both vector-ordered and standard-ordered state type.
// It also includes functions for creating and deleting State instances with c bindings.
#include <musica/micm/state_c_interface.hpp>

namespace musica
{
  template<typename Func>
  auto HandleErrors(Func func, Error* error) -> decltype(func())
  {
    DeleteError(error);
    try
    {
      return func();
    }
    catch (const std::system_error& e)
    {
      *error = ToError(e);
    }
    catch (const std::exception& e)
    {
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_UNKNOWN, e.what());
    }
    return decltype(func())();
  }

  State* CreateMicmState(musica::MICM* micm, Error* error)
  {
    return HandleErrors(
        [&]() -> State*
        {
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
        },
        error);
  }

  void DeleteState(const State* state, Error* error)
  {
    HandleErrors(
        [&]() -> void
        {
          if (state == nullptr)
          {
            std::string msg = "State pointer is null, cannot delete state.";
            *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND, msg.c_str());
            return;
          }

          delete state;
        },
        error);
  }
}  // namespace musica
