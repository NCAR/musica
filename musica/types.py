# Copyright (C) 2023-2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# This file is part of the musica Python package.
# For more information, see the LICENSE file in the top-level directory of this distribution.
from typing import Optional, Any, Dict, List, Union, Tuple
from os import PathLike
import math
from _musica._core import (
    _Conditions,
    _SolverType,
    _Solver,
    _State,
    _create_solver,
    _create_solver_from_mechanism,
    _create_state,
    _micm_solve,
    _vector_size,
    _species_ordering,
    _user_defined_rate_parameters_ordering,
)
import musica.mechanism_configuration as mc

AVOGADRO = 6.02214076e23  # mol^-1
BOLTZMANN = 1.380649e-23  # J K^-1
GAS_CONSTANT = AVOGADRO * BOLTZMANN  # J K^-1 mol^-1

FilePath = Union[str, "PathLike[str]"]


def _get_vector_matrix_indices(row_index: int, column_index: int, vector_size: int) -> Tuple[int, int]:
    """
    Get the row and column indices for a matrix given the row and column indices for a vector.

    Parameters
    ----------
    row_index : int
        Row index of the vector.
    column_index : int
        Column index of the vector.
    vector_size : int
        Size of the vector.

    Returns
    -------
    tuple[int, int]
        Index for which state matrix to use and the index in that matrix'x underlying data vector.
    """
    return (row_index // vector_size, column_index * (vector_size - 1) + row_index % vector_size)


class Conditions(_Conditions):
    """
    Conditions class for the MICM solver. If air density is not provided,
    it will be calculated from the Ideal Gas Law using the provided temperature and pressure.

    Parameters
    ----------
    temperature : float
        Temperature in Kelvin.
    pressure : float
        Pressure in Pascals.
    air_density : float
        Air density in mol m-3
    """

    def __init__(
        self,
        temperature: Optional[Union[float, int]] = None,
        pressure: Optional[Union[float, int]] = None,
        air_density: Optional[Union[float, int]] = None,
    ):
        super().__init__()
        if temperature is not None:
            self.temperature = temperature
        if pressure is not None:
            self.pressure = pressure
        if air_density is not None:
            self.air_density = air_density
        elif temperature is not None and pressure is not None:
            self.air_density = 1.0 / (GAS_CONSTANT * temperature / pressure)


class SolverType(_SolverType):
    """
    Enum class for the type of solver to use.
    """


class State():
    """
    State class for the MICM solver. It contains the initial conditions and species concentrations.
    """

    def __init__(self, solver: _Solver, number_of_grid_cells: int, vector_size: int = 0):
        if number_of_grid_cells < 1:
            raise ValueError("number_of_grid_cells must be greater than 0.")
        super().__init__()
        self.__states = [
            _create_state(solver, min(vector_size, number_of_grid_cells - i * vector_size))
            for i in range(math.ceil(number_of_grid_cells / vector_size))
        ] if vector_size > 0 else [_create_state(solver, number_of_grid_cells)]
        self.__species_ordering = _species_ordering(self.__states[0])
        self.__user_defined_rate_parameters_ordering = _user_defined_rate_parameters_ordering(self.__states[0])
        self.__number_of_grid_cells = number_of_grid_cells
        self.__vector_size = vector_size

    def get_internal_states(self) -> List[_State]:
        """
        Get the internal states of the MICM solver.

        Returns
        -------
        List[_State]
            List of internal states.
        """
        return self.__states

    def set_concentrations(self, concentrations: Dict[str, Union[Union[float, int], List[Union[float, int]]]]):
        """
        Set the concentrations of the species in the state. Any species not in the
        dictionary will be set to zero. The concentrations can be a single value when solving
        for a single grid cell, or a list of values when solving for multiple grid cells.

        Parameters
        ----------
        concentrations : Dict[str, Union[Union[float, int], List[Union[float, int]]]]
            Dictionary of species names and their concentrations.
        """
        for name, value in concentrations.items():
            if name not in self.__species_ordering:
                raise ValueError(f"Species {name} not found in the mechanism.")
            i_species = self.__species_ordering[name]
            if isinstance(value, float) or isinstance(value, int):
                value = [value]
            if len(value) != self.__number_of_grid_cells:
                raise ValueError(f"Concentration list for {name} must have length {self.__number_of_grid_cells}.")
            # Counter 'k' is used to map grid cell indices across multiple state segments.
            k = 0
            for state in self.__states:
                cell_stride, species_stride = state.concentration_strides()
                for i_cell in range(state.number_of_grid_cells()):
                    state.concentrations[i_species * species_stride + i_cell * cell_stride] = value[k]
                    k += 1

    def set_user_defined_rate_parameters(
            self, user_defined_rate_parameters: Dict[str, Union[Union[float, int], List[Union[float, int]]]]):
        """
        Set the user-defined rate parameters in the state. Any parameter not in the
        dictionary will be set to zero. The parameters can be a single value when solving
        for a single grid cell, or a list of values when solving for multiple grid cells.

        Parameters
        ----------
        user_defined_rate_parameters : Dict[str, Union[Union[float, int], List[Union[float, int]]]]
            Dictionary of user-defined rate parameter names and their values.
        """
        for name, value in user_defined_rate_parameters.items():
            if name not in self.__user_defined_rate_parameters_ordering:
                raise ValueError(f"User-defined rate parameter {name} not found in the mechanism.")
            i_param = self.__user_defined_rate_parameters_ordering[name]
            if isinstance(value, float) or isinstance(value, int):
                value = [value]
            if len(value) != self.__number_of_grid_cells:
                raise ValueError(
                    f"User-defined rate parameter list for {name} must have length {self.__number_of_grid_cells}.")
            # Initialize `k` to index the grid cells when assigning user-defined rate parameters.
            k = 0
            for state in self.__states:
                cell_stride, param_stride = state.user_defined_rate_parameter_strides()
                for i_cell in range(state.number_of_grid_cells()):
                    state.user_defined_rate_parameters[i_param * param_stride + i_cell * cell_stride] = value[k]
                    k += 1

    def set_conditions(self,
                       temperatures: Union[Union[float, int], List[Union[float, int]]],
                       pressures: Union[Union[float, int], List[Union[float, int]]],
                       air_densities: Optional[Union[Union[float, int], List[Union[float, int]]]] = None):
        """
        Set the conditions for the state. The individual conditions can be a single value
        when solving for a single grid cell, or a list of values when solving for multiple grid cells.
        If air density is not provided, it will be calculated from the Ideal Gas Law using the provided
        temperature and pressure.

        Parameters
        ----------
        temperatures : Union[float, List[float]]
            Temperature in Kelvin.
        pressures : Union[float, List[float]]
            Pressure in Pascals.
        air_densities : Optional[Union[float, List[float]]]
            Air density in mol m-3. If not provided, it will be calculated from the Ideal Gas Law.
        """
        if isinstance(temperatures, float) or isinstance(temperatures, int):
            if self.__number_of_grid_cells > 1:
                raise ValueError(f"temperatures must be a list of length {self.__number_of_grid_cells}.")
            temperatures = [temperatures]
        if isinstance(pressures, float) or isinstance(pressures, int):
            if self.__number_of_grid_cells > 1:
                raise ValueError(f"pressures must be a list of length {self.__number_of_grid_cells}.")
            pressures = [pressures]
        if air_densities is not None and (isinstance(air_densities, float) or isinstance(air_densities, int)):
            if self.__number_of_grid_cells > 1:
                raise ValueError(f"air_densities must be a list of length {self.__number_of_grid_cells}.")
            air_densities = [air_densities]
        if len(temperatures) != self.__number_of_grid_cells:
            raise ValueError(f"temperatures must be a list of length {self.__number_of_grid_cells}.")
        if len(pressures) != self.__number_of_grid_cells:
            raise ValueError(f"pressures must be a list of length {self.__number_of_grid_cells}.")
        if air_densities is not None and len(air_densities) != self.__number_of_grid_cells:
            raise ValueError(f"air_densities must be a list of length {self.__number_of_grid_cells}.")
        k = 0
        for state in self.__states:
            for condition in state.conditions:
                condition.temperature = temperatures[k]
                condition.pressure = pressures[k]
                condition.air_density = air_densities[k] if air_densities is not None else pressures[k] / (
                    GAS_CONSTANT * temperatures[k])
                k += 1

    def get_concentrations(self) -> Dict[str, List[float]]:
        """
        Get the concentrations of the species in the state.

        Returns
        -------
        Dict[str, List[float]]
            Dictionary of species names and their concentrations.
        """
        concentrations = {}
        for species, i_species in self.__species_ordering.items():
            concentrations[species] = []
            for state in self.__states:
                cell_stride, species_stride = state.concentration_strides()
                for i_cell in range(state.number_of_grid_cells()):
                    concentrations[species].append(
                        state.concentrations[i_species * species_stride + i_cell * cell_stride])
        return concentrations

    def get_user_defined_rate_parameters(self) -> Dict[str, List[float]]:
        """
        Get the user-defined rate parameters in the state.

        Returns
        -------
        Dict[str, List[float]]
            Dictionary of user-defined rate parameter names and their values.
        """
        user_defined_rate_parameters = {}
        for param, i_param in self.__user_defined_rate_parameters_ordering.items():
            user_defined_rate_parameters[param] = []
            for state in self.__states:
                cell_stride, param_stride = state.user_defined_rate_parameter_strides()
                for i_cell in range(state.number_of_grid_cells()):
                    user_defined_rate_parameters[param].append(
                        state.user_defined_rate_parameters[i_param * param_stride + i_cell * cell_stride])
        return user_defined_rate_parameters

    def get_conditions(self) -> Dict[str, List[float]]:
        """
        Get the conditions for the state.

        Returns
        -------
        Dict[str, List[float]]
            Dictionary of conditions names and their values.
        """
        conditions = {}
        conditions["temperature"] = []
        conditions["pressure"] = []
        conditions["air_density"] = []
        for state in self.__states:
            for i_cell in range(state.number_of_grid_cells()):
                conditions["temperature"].append(state.conditions[i_cell].temperature)
                conditions["pressure"].append(state.conditions[i_cell].pressure)
                conditions["air_density"].append(state.conditions[i_cell].air_density)
        return conditions


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
        mechanism: mc.Mechanism = None,
        solver_type: _SolverType = None,
    ):
        self.__solver_type = solver_type
        self.__vector_size = _vector_size(solver_type)
        if config_path is None and mechanism is None:
            raise ValueError("Either config_path or mechanism must be provided.")
        if config_path is not None and mechanism is not None:
            raise ValueError("Only one of config_path or mechanism must be provided.")
        if config_path is not None:
            self.__solver = _create_solver(config_path, solver_type)
        elif mechanism is not None:
            self.__solver = _create_solver_from_mechanism(mechanism, solver_type)

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
    ):
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
        State
            Updated state object after solving the system of equations.
        """
        if not isinstance(state, State):
            raise TypeError("state must be an instance of State.")
        if not isinstance(time_step, (int, float)):
            raise TypeError("time_step must be an int or float.")
        states = state.get_internal_states()
        for _, _state in enumerate(states):
            _micm_solve(self.__solver, _state, time_step)
