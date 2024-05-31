/**
 * This file contains the defintion of the TUVX class, which represents a photolysis calculator
 * It also includes functions for creating and deleting TUVX instances with c binding
 * Copyright (C) 2023-2024 National Center for Atmospheric Research,
 *
 * SPDX-License-Identifier: Apache-2.0* creating solvers, and solving the model.
 */
#pragma once

#include <memory>

namespace musica {
    struct GridMap
    {
        GridMap(void* grid_map) : grid_map_(grid_map) {}

        private:
            void* grid_map_;
    };
}