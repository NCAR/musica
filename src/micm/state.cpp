// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the implementation of the MICM class, which represents a
// multi-component reactive transport model. It also includes functions for
// creating and deleting MICM instances, creating solvers, and solving the model.

#include <musica/micm.hpp>

#include <musica/state.hpp>

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

    State *CreateMicmState(musica::MICM *micm)
    {
        if (!micm) {
            std::cerr << "MICM pointer is null, cannot create state." << std::endl;
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
}