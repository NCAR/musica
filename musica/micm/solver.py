# Copyright (C) 2023-2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

from .. import backend

_backend = backend.get_backend()

_BackendSolverType = _backend._micm._SolverType
create_solver = _backend._micm._create_solver
create_solver_from_mechanism = _backend._micm._create_solver_from_mechanism
micm_solve = _backend._micm._micm_solve
vector_size = _backend._micm._vector_size

class SolverType(_BackendSolverType):
    """
    Enum class for the type of solver to use.
    """
