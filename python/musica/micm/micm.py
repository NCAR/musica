# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

from __future__ import annotations
from typing import Union, Any, TYPE_CHECKING, Optional
from os import PathLike

from .state import State
from .solver import SolverType
from .solver_result import SolverResult
from .solver_parameters import RosenbrockSolverParameters, BackwardEulerSolverParameters
from .. import backend

_backend = backend.get_backend()

create_solver = _backend._micm._create_solver
create_solver_from_mechanism = _backend._micm._create_solver_from_mechanism
micm_solve = _backend._micm._micm_solve
vector_size = _backend._micm._vector_size
_set_rosenbrock_params = _backend._micm._set_rosenbrock_solver_parameters
_set_backward_euler_params = _backend._micm._set_backward_euler_solver_parameters
_get_rosenbrock_params = _backend._micm._get_rosenbrock_solver_parameters
_get_backward_euler_params = _backend._micm._get_backward_euler_solver_parameters
_CppRosenbrockParams = _backend._micm._RosenbrockSolverParameters
_CppBackwardEulerParams = _backend._micm._BackwardEulerSolverParameters
_VectorDouble = _backend.VectorDouble

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
        solver_parameters: Optional[Union[RosenbrockSolverParameters, BackwardEulerSolverParameters]] = None,
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
            solver_parameters : RosenbrockSolverParameters or BackwardEulerSolverParameters, optional
                Solver-specific parameters. Must match the solver type.
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
            self.__solver = create_solver_from_mechanism(mechanism, solver_type)
        if solver_parameters is not None:
            self.set_solver_parameters(solver_parameters)

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

    def set_solver_parameters(
        self,
        params: Union[RosenbrockSolverParameters, BackwardEulerSolverParameters],
    ):
        """Set solver-specific parameters.

        Parameters
        ----------
        params : RosenbrockSolverParameters or BackwardEulerSolverParameters
            The parameters to set. Must match the solver type.

        Raises
        ------
        TypeError
            If the parameter type does not match the solver type.
        """
        if isinstance(params, RosenbrockSolverParameters):
            cpp_params = _CppRosenbrockParams()
            cpp_params.relative_tolerance = params.relative_tolerance
            if params.absolute_tolerances is not None:
                cpp_params.absolute_tolerances = _VectorDouble(params.absolute_tolerances)
            cpp_params.h_min = params.h_min
            cpp_params.h_max = params.h_max
            cpp_params.h_start = params.h_start
            cpp_params.max_number_of_steps = params.max_number_of_steps
            _set_rosenbrock_params(self.__solver, cpp_params)
        elif isinstance(params, BackwardEulerSolverParameters):
            cpp_params = _CppBackwardEulerParams()
            cpp_params.relative_tolerance = params.relative_tolerance
            if params.absolute_tolerances is not None:
                cpp_params.absolute_tolerances = _VectorDouble(params.absolute_tolerances)
            cpp_params.max_number_of_steps = params.max_number_of_steps
            cpp_params.time_step_reductions = _VectorDouble(params.time_step_reductions)
            _set_backward_euler_params(self.__solver, cpp_params)
        else:
            raise TypeError(
                "params must be RosenbrockSolverParameters or BackwardEulerSolverParameters"
            )

    def get_solver_parameters(
        self,
    ) -> Union[RosenbrockSolverParameters, BackwardEulerSolverParameters]:
        """Get the current solver parameters.

        Returns
        -------
        RosenbrockSolverParameters or BackwardEulerSolverParameters
            The current solver parameters, depending on the solver type.
        """
        solver_type = self.__solver_type
        if solver_type in (SolverType.rosenbrock, SolverType.rosenbrock_standard_order):
            cpp_params = _get_rosenbrock_params(self.__solver)
            return RosenbrockSolverParameters(
                relative_tolerance=cpp_params.relative_tolerance,
                absolute_tolerances=list(cpp_params.absolute_tolerances) if cpp_params.absolute_tolerances else None,
                h_min=cpp_params.h_min,
                h_max=cpp_params.h_max,
                h_start=cpp_params.h_start,
                max_number_of_steps=cpp_params.max_number_of_steps,
            )
        elif solver_type in (SolverType.backward_euler, SolverType.backward_euler_standard_order):
            cpp_params = _get_backward_euler_params(self.__solver)
            return BackwardEulerSolverParameters(
                relative_tolerance=cpp_params.relative_tolerance,
                absolute_tolerances=list(cpp_params.absolute_tolerances) if cpp_params.absolute_tolerances else None,
                max_number_of_steps=cpp_params.max_number_of_steps,
                time_step_reductions=list(cpp_params.time_step_reductions),
            )
        else:
            raise RuntimeError(f"Unknown solver type: {solver_type}")
