# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

from typing import List, Optional


class RosenbrockSolverParameters:
    """Parameters for configuring Rosenbrock solvers.

    Attributes
    ----------
    relative_tolerance : float
        Relative tolerance for the solver (default: 1e-6).
    absolute_tolerances : list of float or None
        Absolute tolerances per species. None uses MICM defaults.
    h_min : float
        Minimum step size in seconds (default: 0.0, meaning solver chooses).
    h_max : float
        Maximum step size in seconds (default: 0.0, meaning solver chooses).
    h_start : float
        Initial step size in seconds (default: 0.0, meaning solver chooses).
    max_number_of_steps : int
        Maximum number of internal steps (default: 1000).
    """

    def __init__(
        self,
        relative_tolerance: float = 1e-6,
        absolute_tolerances: Optional[List[float]] = None,
        h_min: float = 0.0,
        h_max: float = 0.0,
        h_start: float = 0.0,
        max_number_of_steps: int = 1000,
    ):
        self.relative_tolerance = relative_tolerance
        self.absolute_tolerances = absolute_tolerances
        self.h_min = h_min
        self.h_max = h_max
        self.h_start = h_start
        self.max_number_of_steps = max_number_of_steps

    def __repr__(self):
        return (
            f"RosenbrockSolverParameters("
            f"relative_tolerance={self.relative_tolerance}, "
            f"absolute_tolerances={self.absolute_tolerances}, "
            f"h_min={self.h_min}, h_max={self.h_max}, h_start={self.h_start}, "
            f"max_number_of_steps={self.max_number_of_steps})"
        )


class BackwardEulerSolverParameters:
    """Parameters for configuring Backward Euler solvers.

    Attributes
    ----------
    relative_tolerance : float
        Relative tolerance for the solver (default: 1e-6).
    absolute_tolerances : list of float or None
        Absolute tolerances per species. None uses MICM defaults.
    max_number_of_steps : int
        Maximum number of internal steps (default: 11).
    time_step_reductions : list of float
        Factors by which to reduce the time step after failed solves.
        Must have exactly 5 elements (default: [0.5, 0.5, 0.5, 0.5, 0.1]).
    """

    def __init__(
        self,
        relative_tolerance: float = 1e-6,
        absolute_tolerances: Optional[List[float]] = None,
        max_number_of_steps: int = 11,
        time_step_reductions: Optional[List[float]] = None,
    ):
        self.relative_tolerance = relative_tolerance
        self.absolute_tolerances = absolute_tolerances
        self.max_number_of_steps = max_number_of_steps
        self.time_step_reductions = (
            time_step_reductions
            if time_step_reductions is not None
            else [0.5, 0.5, 0.5, 0.5, 0.1]
        )
        if len(self.time_step_reductions) != 5:
            raise ValueError(
                f"time_step_reductions must have exactly 5 elements, "
                f"got {len(self.time_step_reductions)}"
            )

    def __repr__(self):
        return (
            f"BackwardEulerSolverParameters("
            f"relative_tolerance={self.relative_tolerance}, "
            f"absolute_tolerances={self.absolute_tolerances}, "
            f"max_number_of_steps={self.max_number_of_steps}, "
            f"time_step_reductions={self.time_step_reductions})"
        )
