"""Tests for the State class."""
from __future__ import annotations
import pytest  # type: ignore # pylint: disable=import-error
import numpy as np  # type: ignore # pylint: disable=import-error
from musica.types import State, MICM
import musica.mechanism_configuration as mc
from musica.mechanism_configuration import Mechanism


def create_test_mechanism() -> Mechanism:
    """Helper function to create a test mechanism."""
    # Chemical species
    A = mc.Species(
        name="A",
        molecular_weight_kg_mol=32.1,
        other_properties={
            "__absolute tolerance": "1.0e-30"})
    B = mc.Species(name="B")
    C = mc.Species(name="C")
    M = mc.Species(name="M", is_third_body=True)

    # Chemical phases
    gas = mc.Phase(name="gas", species=[mc.PhaseSpecies(name=A.name, diffusion_coefficient_m2_s=1.0), B, C, M])

    # Reactions
    my_arrhenius = mc.Arrhenius(
        name="my arrhenius",
        A=32.1, B=-2.3, C=102.3, D=63.4, E=-1.3,
        gas_phase=gas,
        reactants=[B],
        products=[C],
        other_properties={"__irrelevant": "2"},
    )

    my_other_arrhenius = mc.Arrhenius(
        name="my other arrhenius",
        A=29.3, B=-1.5, Ea=101.2, D=82.6, E=-0.98,
        gas_phase=gas,
        reactants=[A],
        products=[(1.2, B)]
    )

    my_troe = mc.Troe(
        name="my troe",
        gas_phase=gas,
        k0_A=1.2e-12,
        k0_B=167,
        k0_C=3,
        kinf_A=136,
        kinf_B=5,
        kinf_C=24,
        Fc=0.9,
        N=0.8,
        reactants=[B, A],
        products=[C],
        other_properties={"__irrelevant": "2"},
    )

    my_ternary = mc.TernaryChemicalActivation(
        name="my ternary chemical activation",
        gas_phase=gas,
        k0_A=32.1,
        k0_B=-2.3,
        k0_C=102.3,
        kinf_A=63.4,
        kinf_B=-1.3,
        kinf_C=908.5,
        Fc=1.3,
        N=32.1,
        reactants=[B, A],
        products=[C],
        other_properties={"__irrelevant": "2"},
    )

    my_branched = mc.Branched(
        name="my branched",
        gas_phase=gas,
        reactants=[A],
        alkoxy_products=[B],
        nitrate_products=[C],
        X=1.2e-4,
        Y=167,
        a0=0.15,
        n=9,
        other_properties={"__irrelevant": "2"},
    )

    my_tunneling = mc.Tunneling(
        name="my tunneling",
        gas_phase=gas,
        reactants=[B],
        products=[C],
        A=123.45,
        B=1200.0,
        C=1.0e8,
        other_properties={"__irrelevant": "2"},
    )

    my_surface = mc.Surface(
        name="my surface",
        gas_phase=gas,
        gas_phase_species=A,
        reaction_probability=2.0e-2,
        gas_phase_products=[B, C],
        other_properties={"__irrelevant": "2"},
    )

    photo_b = mc.Photolysis(
        name="photo B",
        gas_phase=gas,
        reactants=[B],
        products=[C],
        scaling_factor=12.3,
        other_properties={"__irrelevant": "2"},
    )

    my_emission = mc.Emission(
        name="my emission",
        gas_phase=gas,
        products=[B],
        scaling_factor=12.3,
        other_properties={"__irrelevant": "2"},
    )

    my_first_order_loss = mc.FirstOrderLoss(
        name="my first order loss",
        gas_phase=gas,
        reactants=[C],
        scaling_factor=12.3,
        other_properties={"__irrelevant": "2"},
    )

    user_defined = mc.UserDefined(
        name="my user defined",
        gas_phase=gas,
        reactants=[A, B],
        products=[(1.3, C)],
        scaling_factor=12.3,
        other_properties={"__irrelevant": "2"}
    )

    # Mechanism
    return mc.Mechanism(
        name="Full Configuration",
        species=[A, B, C, M],
        phases=[gas],
        reactions=[my_arrhenius, my_other_arrhenius, my_troe, my_ternary,
                   my_branched, my_tunneling,
                   my_surface, photo_b,
                   my_emission, my_first_order_loss, user_defined],
        version=mc.Version(1, 0, 0),
    )


def get_test_solver(mech: Mechanism) -> MICM:
    """Helper function to create a test solver."""
    return MICM(mechanism=mech)


def test_state_initialization():
    """Test State initialization with various grid cell configurations."""
    # Use the test mechanism
    mech = create_test_mechanism()

    # Create MICM instance
    solver = get_test_solver(mech)

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
    mech = create_test_mechanism()
    solver = get_test_solver(mech)

    # Test single grid cell
    state = solver.create_state(number_of_grid_cells=1)
    concentrations = {"A": 1.0, "B": 2.0, "C": 3.0}
    state.set_concentrations(concentrations)
    result = state.get_concentrations()
    assert result["A"][0] == 1.0
    assert result["B"][0] == 2.0
    assert result["C"][0] == 3.0

    # Test multiple grid cells
    state_multi = solver.create_state(number_of_grid_cells=2)
    concentrations_multi = {"A": [1.0, 2.0], "B": [3.0, 4.0], "C": [5.0, 6.0]}
    state_multi.set_concentrations(concentrations_multi)
    result_multi = state_multi.get_concentrations()
    assert result_multi["A"] == [1.0, 2.0]
    assert result_multi["B"] == [3.0, 4.0]
    assert result_multi["C"] == [5.0, 6.0]

    # Test invalid species
    with pytest.raises(ValueError, match="Species D not found in the mechanism"):
        state.set_concentrations({"D": 1.0})

    # Test invalid length
    with pytest.raises(ValueError, match="must have length"):
        state_multi.set_concentrations({"A": [1.0]})

    # Test cannot set third-body species
    with pytest.raises(ValueError, match="Species M not found in the mechanism"):
        state.set_concentrations({"M": 1.0})


def test_set_get_conditions():
    """Test setting and getting environmental conditions."""
    # Use the test mechanism
    mech = create_test_mechanism()
    solver = get_test_solver(mech)

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
    mech = create_test_mechanism()
    solver = get_test_solver(mech)

    # Test single grid cell
    state = solver.create_state(number_of_grid_cells=1)
    params = {"EMIS.my emission": 1.0}
    state.set_user_defined_rate_parameters(params)
    result = state.get_user_defined_rate_parameters()
    assert result["EMIS.my emission"][0] == 1.0

    params = {
        "SURF.my surface.effective radius [m]": 0.5,
        "SURF.my surface.particle number concentration [# m-3]": 1000.0,
    }
    state.set_user_defined_rate_parameters(params)
    result = state.get_user_defined_rate_parameters()
    assert result["SURF.my surface.effective radius [m]"][0] == 0.5
    assert result["SURF.my surface.particle number concentration [# m-3]"][0] == 1000.0

    # Test multiple grid cells
    state_multi = solver.create_state(number_of_grid_cells=2)
    params_multi = {"PHOTO.photo B": [1.0, 2.0]}
    state_multi.set_user_defined_rate_parameters(params_multi)
    result_multi = state_multi.get_user_defined_rate_parameters()
    assert result_multi["PHOTO.photo B"] == [1.0, 2.0]

    # Test invalid parameter
    with pytest.raises(ValueError, match="User-defined rate parameter invalid_param not found"):
        state.set_user_defined_rate_parameters({"invalid_param": 1.0})

    solver = get_test_solver(mech)

    state = solver.create_state(number_of_grid_cells=1)

    # Test species ordering
    species_ordering = state.get_species_ordering()
    assert isinstance(species_ordering, dict)
    assert len(species_ordering) == 3  # A, B, C (M is third-body and not included)

    # Dictionary style access
    assert species_ordering["A"] >= 0
    assert species_ordering["B"] >= 0
    assert species_ordering["C"] >= 0

    # Using get() method with default value
    assert species_ordering.get("A", -1) >= 0  # returns value if key exists
    assert species_ordering.get("Z", -1) == -1  # returns -1 since Z doesn't exist

    # Test key membership
    assert "A" in species_ordering
    assert "B" in species_ordering
    assert "C" in species_ordering
    assert "M" not in species_ordering  # third-body not included

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
