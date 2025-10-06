# Copyright (C) 2023-2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

from __future__ import annotations
# from typing import Union, Any, TYPE_CHECKING
# from os import PathLike

from .. import backend
# from .state import State

_backend = backend.get_backend()

SolverType = _backend._micm._SolverType
# create_solver = _backend._micm._create_solver
# create_solver_from_mechanism = _backend._micm._create_solver_from_mechanism
# micm_solve = _backend._micm._micm_solve
# vector_size = _backend._micm._vector_size
# # mc = _backend._mechanism_configuration


class SolverType(SolverType):
    """
    Enum class for the type of solver to use.
    """
