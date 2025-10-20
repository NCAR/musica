# Copyright (C) 2023-2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

from typing import Optional, Dict, List, Union, Any
import math

from ..constants import GAS_CONSTANT
from .. import backend
from .utils import is_scalar_number, species_ordering, user_defined_rate_parameters_ordering

_backend = backend.get_backend()

create_state = _backend._micm._create_state


class State():
    """
    State class for the MICM solver. It contains the initial conditions and species concentrations.
    """

    def __init__(self, solver: Any, number_of_grid_cells: int, vector_size: int = 0):
        if number_of_grid_cells < 1:
            raise ValueError("number_of_grid_cells must be greater than 0.")
        super().__init__()
        self.__states = [
            create_state(solver, min(
                vector_size, number_of_grid_cells - i * vector_size))
            for i in range(math.ceil(number_of_grid_cells / vector_size))
        ] if vector_size > 0 else [create_state(solver, number_of_grid_cells)]
        self.__species_ordering = species_ordering(self.__states[0])
        self.__user_defined_rate_parameters_ordering = user_defined_rate_parameters_ordering(
            self.__states[0])
        self.__number_of_grid_cells = number_of_grid_cells
        self.__vector_size = vector_size

    def get_internal_states(self) -> List[Any]:
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
            if is_scalar_number(value):
                value = [value]
            if len(value) != self.__number_of_grid_cells:
                raise ValueError(
                    f"Concentration list for {name} must have length {self.__number_of_grid_cells}.")
            # Counter 'k' is used to map grid cell indices across multiple state segments.
            k = 0
            for state in self.__states:
                cell_stride, species_stride = state.concentration_strides()
                for i_cell in range(state.number_of_grid_cells()):
                    state.concentrations[i_species *
                                         species_stride + i_cell * cell_stride] = value[k]
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
                raise ValueError(
                    f"User-defined rate parameter {name} not found in the mechanism.")
            i_param = self.__user_defined_rate_parameters_ordering[name]
            if is_scalar_number(value):
                value = [value]
            if len(value) != self.__number_of_grid_cells:
                raise ValueError(
                    f"User-defined rate parameter list for {name} must have length {self.__number_of_grid_cells}.")
            # Initialize `k` to index the grid cells when assigning user-defined rate parameters.
            k = 0
            for state in self.__states:
                cell_stride, param_stride = state.user_defined_rate_parameter_strides()
                for i_cell in range(state.number_of_grid_cells()):
                    state.user_defined_rate_parameters[i_param *
                                                       param_stride + i_cell * cell_stride] = value[k]
                    k += 1

    def set_conditions(self,
                       temperatures: Optional[Union[Union[float, int], List[Union[float, int]]]] = None,
                       pressures: Optional[Union[Union[float, int], List[Union[float, int]]]] = None,
                       air_densities: Optional[Union[Union[float, int], List[Union[float, int]]]] = None):
        """
        Set the conditions for the state. The individual conditions can be a single value
        when solving for a single grid cell, or a list of values when solving for multiple grid cells.
        If air density is not provided, it will be calculated from the Ideal Gas Law using the provided
        temperature and pressure. If temperature or pressure are not provided, their values will remain
        unchanged.

        Parameters
        ----------
        temperatures : Optional[Union[float, List[float]]]
            Temperature in Kelvin.
        pressures : Optional[Union[float, List[float]]]
            Pressure in Pascals.
        air_densities : Optional[Union[float, List[float]]]
            Air density in mol m-3. If not provided, it will be calculated from the Ideal Gas Law.
        """
        if temperatures is not None and is_scalar_number(temperatures):
            if self.__number_of_grid_cells > 1:
                raise ValueError(
                    f"temperatures must be a list of length {self.__number_of_grid_cells}.")
            temperatures = [temperatures]
        if pressures is not None and is_scalar_number(pressures):
            if self.__number_of_grid_cells > 1:
                raise ValueError(
                    f"pressures must be a list of length {self.__number_of_grid_cells}.")
            pressures = [pressures]
        if air_densities is not None and is_scalar_number(air_densities):
            if self.__number_of_grid_cells > 1:
                raise ValueError(
                    f"air_densities must be a list of length {self.__number_of_grid_cells}.")
            air_densities = [air_densities]
        if temperatures is not None and len(temperatures) != self.__number_of_grid_cells:
            raise ValueError(
                f"temperatures must be a list of length {self.__number_of_grid_cells}.")
        if pressures is not None and len(pressures) != self.__number_of_grid_cells:
            raise ValueError(
                f"pressures must be a list of length {self.__number_of_grid_cells}.")
        if air_densities is not None and len(air_densities) != self.__number_of_grid_cells:
            raise ValueError(
                f"air_densities must be a list of length {self.__number_of_grid_cells}.")
        k = 0
        for state in self.__states:
            for condition in state.conditions:
                condition.temperature = temperatures[k] if temperatures is not None else condition.temperature
                condition.pressure = pressures[k] if pressures is not None else condition.pressure
                condition.air_density = air_densities[k] if air_densities is not None else condition.pressure / (
                    GAS_CONSTANT * condition.temperature)
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
                conditions["temperature"].append(
                    state.conditions[i_cell].temperature)
                conditions["pressure"].append(
                    state.conditions[i_cell].pressure)
                conditions["air_density"].append(
                    state.conditions[i_cell].air_density)
        return conditions

    def get_species_ordering(self) -> Dict[str, int]:
        """
        Get the species ordering for the state.

        Returns
        -------
        Dict[str, int]
            Dictionary of species names and their indices.
        """
        return self.__species_ordering

    def get_user_defined_rate_parameters_ordering(self) -> Dict[str, int]:
        """
        Get the user-defined rate parameters ordering for the state.

        Returns
        -------
        Dict[str, int]
            Dictionary of user-defined rate parameter names and their indices.
        """
        return self.__user_defined_rate_parameters_ordering
