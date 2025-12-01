# Copyright (C) 2023-2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

"""
Module for exposing solver result types from MICM.

This module provides access to:
- SolverState: Enum representing the final state of the solver
- SolverStats: Statistics from a solver run
- SolverResult: Combined result containing both state and statistics
"""

from .. import backend

_backend = backend.get_backend()

# Expose the SolverState enum
SolverState = _backend._micm._SolverState

# Expose the SolverStats class
SolverStats = _backend._micm._SolverResultsStats

# Expose the SolverResult class
SolverResult = _backend._micm._SolverResult
