"""Tests for the State class."""
import pytest
import numpy as np
from musica.types import State, MICM
from test_util_full_mechanism import get_fully_defined_mechanism


def test_state_initialization():
    """Test State initialization with various grid cell configurations."""
    # Create MICM instance
    solver = MICM(mechanism=get_fully_defined_mechanism())

    # Test with valid input
    state = solver.create_state(number_of_grid_cells=1)
    assert isinstance(state, State)

    # Test with multiple grid cells
    state_multi = solver.create_state(number_of_grid_cells=3)
    assert isinstance(state_multi, State)

    # Test with invalid input
    with pytest.raises(ValueError, match="number_of_grid_cells must be greater than 0"):
        solver.create_state(number_of_grid_cells=0)


def test_set_get_concentrations():
    """Test setting and getting concentrations."""
    # Use the test mechanism
    print()
    solver = MICM(mechanism=get_fully_defined_mechanism())

    # Test single grid cell
    state = solver.create_state(number_of_grid_cells=1)
    concentrations = {"A": 1.0, "H2O2": 2.0, "ethanol": 3.0}
    state.set_concentrations(concentrations)
    result = state.get_concentrations()
    assert result["A"][0] == 1.0
    assert result["H2O2"][0] == 2.0
    assert result["ethanol"][0] == 3.0

    # Test multiple grid cells
    state_multi = solver.create_state(number_of_grid_cells=2)
    concentrations_multi = {"A": [1.0, 2.0], "H2O2": [3.0, 4.0], "ethanol": [5.0, 6.0]}
    state_multi.set_concentrations(concentrations_multi)
    result_multi = state_multi.get_concentrations()
    assert result_multi["A"] == [1.0, 2.0]
    assert result_multi["H2O2"] == [3.0, 4.0]
    assert result_multi["ethanol"] == [5.0, 6.0]

    # Test invalid species
    with pytest.raises(ValueError, match="Species D not found in the mechanism"):
        state.set_concentrations({"D": 1.0})

    # Test invalid length
    with pytest.raises(ValueError, match="must have length"):
        state_multi.set_concentrations({"A": [1.0]})

    # Test cannot set third-body species
    with pytest.raises(ValueError, match="Species M not found in the mechanism"):
        state.set_concentrations({"M": 1.0})

    # Test cannot set constant concentration species
    with pytest.raises(ValueError, match="Species B not found in the mechanism"):
        state.set_concentrations({"B": 1.0})
    
    # Test cannot set constant mixing ratio species
    with pytest.raises(ValueError, match="Species C not found in the mechanism"):
        state.set_concentrations({"C": 1.0})


def test_set_get_conditions():
    """Test setting and getting environmental conditions."""
    print()
    solver = MICM(mechanism=get_fully_defined_mechanism())

    # Test single grid cell
    state = solver.create_state(number_of_grid_cells=1)
    state.set_conditions(temperatures=300.0, pressures=101325.0)
    conditions = state.get_conditions()
    assert conditions["temperature"][0] == 300.0
    assert conditions["pressure"][0] == 101325.0
    assert np.isclose(conditions["air_density"][0], 40.9, rtol=0.1)  # Approximate value from ideal gas law

    # Test multiple grid cells
    state_multi = solver.create_state(number_of_grid_cells=2)
    state_multi.set_conditions(
        temperatures=[300.0, 310.0],
        pressures=[101325.0, 101325.0],
        air_densities=[40.9, 39.5]
    )
    conditions_multi = state_multi.get_conditions()
    assert conditions_multi["temperature"] == [300.0, 310.0]
    assert conditions_multi["pressure"] == [101325.0, 101325.0]
    assert conditions_multi["air_density"] == [40.9, 39.5]

    # Test invalid input length
    with pytest.raises(ValueError, match="must be a list of length"):
        state_multi.set_conditions(temperatures=[300.0])


def test_set_get_user_defined_rate_parameters():
    """Test setting and getting user-defined rate parameters."""
    # Use the test mechanism which includes a user-defined reaction
    print()
    solver = MICM(mechanism=get_fully_defined_mechanism())

    # Test single grid cell
    state = solver.create_state(number_of_grid_cells=1)
    params = {"EMIS.my emission": 1.0}
    state.set_user_defined_rate_parameters(params)
    result = state.get_user_defined_rate_parameters()
    assert result["EMIS.my emission"][0] == 1.0

    # Test multiple grid cells
    state_multi = solver.create_state(number_of_grid_cells=2)
    params_multi = {"PHOTO.photo B": [1.0, 2.0]}
    state_multi.set_user_defined_rate_parameters(params_multi)
    result_multi = state_multi.get_user_defined_rate_parameters()
    assert result_multi["PHOTO.photo B"] == [1.0, 2.0]

    # Test invalid parameter
    with pytest.raises(ValueError, match="User-defined rate parameter invalid_param not found"):
        state.set_user_defined_rate_parameters({"invalid_param": 1.0})

    state = solver.create_state(number_of_grid_cells=1)

    # Test species ordering
    species_ordering = state.get_species_ordering()
    assert isinstance(species_ordering, dict)
    assert len(species_ordering) == 3  # A, B, C (M is third-body and not included)

    # Dictionary style access
    assert species_ordering["A"] >= 0
    assert species_ordering["ethanol"] >= 0
    assert species_ordering["H2O2"] >= 0

    # Using get() method with default value
    assert species_ordering.get("A", -1) >= 0  # returns value if key exists
    assert species_ordering.get("Z", -1) == -1  # returns -1 since Z doesn't exist

    # Test key membership
    assert "A" in species_ordering
    assert "H2O2" in species_ordering
    assert "ethanol" in species_ordering
    assert "M" not in species_ordering  # third-body not included
    assert "B" not in species_ordering  # constant concentration not included
    assert "C" not in species_ordering  # constant mixing ratio not included

    # Test parameter ordering
    param_ordering = state.get_user_defined_rate_parameters_ordering()
    assert isinstance(param_ordering, dict)
    assert len(param_ordering) == 6

    # Convert dict keys to list using list() function
    param_names = list(param_ordering.keys())
    assert len(param_names) == 6
    assert isinstance(param_names[0], str)

    # Alternative way using list comprehension
    param_names_alt = [key for key in param_ordering]
    assert param_names_alt == param_names

    # Sort the keys if needed - useful for consistent ordering in tests
    sorted_param_names = sorted(param_ordering.keys())
    assert len(sorted_param_names) == 6
    assert all(isinstance(name, str) for name in sorted_param_names)

    # Verify all expected keys are present
    expected_params = [
        "PHOTO.photo B",
        "EMIS.my emission",
        "LOSS.my first order loss",
        "SURF.my surface.effective radius [m]",
        "SURF.my surface.particle number concentration [# m-3]",
        "USER.my user defined"
    ]
    assert sorted(expected_params) == sorted(param_names)

if __name__ == "__main__":
    pytest.main([__file__])