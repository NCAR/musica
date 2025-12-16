# Copyright (C) 2023-2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

from __future__ import annotations
from typing import Union, Any, TYPE_CHECKING, List
from os import PathLike

from .state import State
from .solver import SolverType
from .solver_result import SolverResult
from .. import backend

_backend = backend.get_backend()

create_solver = _backend._micm._create_solver
create_solver_from_mechanism = _backend._micm._create_solver_from_mechanism
micm_solve = _backend._micm._micm_solve
vector_size = _backend._micm._vector_size

# For type hints
if TYPE_CHECKING:
    from ..mechanism_configuration import Mechanism
    from .solver_result import SolverResult as SolverResultType
else:
    Mechanism = _backend._mechanism_configuration._Mechanism
    SolverResultType = SolverResult

FilePath = Union[str, "PathLike[str]"]


class MICM():
    """
    The MICM class is a wrapper around the C++ MICM solver. It provides methods to create a solver,
    create a state, and solve the system of equations.

    Parameters
    ----------
    config_path : FilePath
        Path to the configuration file.
    mechanism : mechanism_configuration.Mechanism
        Mechanism object which specifies the chemical mechanism to use.
    solver_type : SolverType
        Type of solver to use.
    number_of_grid_cells : int
        Number of grid cells to use. The default is 1.
    """

    def __init__(
        self,
        config_path: FilePath = None,
        mechanism: Mechanism = None,
        solver_type: Any = None,
        ignore_non_gas_phases: bool = True,
    ):
        """    Initialize the MICM solver.

        Parameters
        ----------
            config_path : FilePath, optional
                Path to the configuration file. If provided, this will be used to create the solver.
            mechanism : Mechanism, optional
                Mechanism object which specifies the chemical mechanism to use. If provided, this will be used
                to create the solver.
            solver_type : SolverType, optional
                Type of solver to use. If not provided, the default Rosenbrock (with standard-ordered matrices) solver type will be used.
            ignore_non_gas_phases : bool, optional
                If True, non-gas phases will be ignored when configuring micm with the mechanism. This is only needed
                until micm properly supports non-gas phases. This option is only supported when passing in a mechanism.
        """
        if solver_type is None:
            solver_type = SolverType.rosenbrock_standard_order
        self.__solver_type = solver_type
        self.__vector_size = vector_size(solver_type)
        if self.__vector_size <= 0:
            raise ValueError(f"Invalid vector size: {self.__vector_size}")
        if config_path is None and mechanism is None:
            raise ValueError(
                "Either config_path or mechanism must be provided.")
        if config_path is not None and mechanism is not None:
            raise ValueError(
                "Only one of config_path or mechanism must be provided.")
        if config_path is not None:
            self.__solver = create_solver(config_path, solver_type)
        elif mechanism is not None:
            self.__solver = create_solver_from_mechanism(
                mechanism, solver_type, ignore_non_gas_phases)

    def solver_type(self) -> SolverType:
        """
        Get the type of solver used.

        Returns
        -------
        SolverType
            The type of solver used.
        """
        return self.__solver_type

    def create_state(self, number_of_grid_cells: int = 1) -> State:
        """
        Create a new state object.

        Returns
        -------
        State
            A new state object.
        """
        return State(self.__solver, number_of_grid_cells, self.__vector_size)

    def solve(
            self,
            state: State,
            time_step: float,
    ) -> SolverResult:
        """
        Solve the system of equations for the given state and time step.

        Parameters
        ----------
        state : State
            State object containing the initial conditions.
        time_step : float
            Time step in seconds.

        Returns
        -------
        SolverResult
            A SolverResult object containing the solver state and statistics.
        """
        if not isinstance(state, State):
            raise TypeError("state must be an instance of State.")
        if not isinstance(time_step, (int, float)):
            raise TypeError("time_step must be an int or float.")

        return micm_solve(self.__solver, state.get_internal_state(), time_step)
