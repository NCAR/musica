// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the MICM class, which represents a
// multi-component reactive transport model. It also includes functions for
// creating and deleting MICM instances, creating solvers, and solving the model.

#include <musica/micm/micm.hpp>

#include <musica/micm/state.hpp>

#include <musica/util.hpp>

#include <micm/solver/rosenbrock_solver_parameters.hpp>
#include <micm/solver/solver_builder.hpp>
#include <micm/system/species.hpp>
#include <micm/version.hpp>

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <string>
#include <system_error>

namespace musica
{

    State *CreateMicmState(musica::MICM *micm, Error *error)
    {
        DeleteError(error);
        if (!micm) {
            std::string msg = "MICM pointer is null, cannot create state.";
            *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND, msg.c_str());
            delete micm;
            return nullptr;
        }

        State *state = new State();

        // Visit the solver_variant_ and assign the correct state manually
        std::visit([&](auto &solver_ptr) 
        {
        state->state_variant_ = solver_ptr->GetState();
        }, micm->solver_variant_);
                
        return state;
    }

    void DeleteState(const State *state_wrapper, Error *error)
    {
        DeleteError(error);
        if (state_wrapper == nullptr)
        {
            *error = NoError();
            return;
        }
        try
        {
            delete state_wrapper;
            *error = NoError();
        }
        catch (const std::system_error &e)
        {
            *error = ToError(e);
        }
    }
}
