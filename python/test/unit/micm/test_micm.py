"""Unit tests for the MICM class."""
from __future__ import annotations
import pytest
from musica.micm import MICM, State, SolverType, SolverResult, SolverState
import musica.mechanism_configuration as mc


def create_simple_mechanism() -> mc.Mechanism:
    """Helper function to create a simple test mechanism."""
    # Create simple species
    A = mc.Species(name="A")
    B = mc.Species(name="B")
    C = mc.Species(name="C")

    # Create gas phase
    gas = mc.Phase(name="gas", species=[A, B, C])

    # Create a simple arrhenius reaction: A -> B
    reaction = mc.Arrhenius(
        name="A to B",
        A=1.0e-6,
        C=100.0,
        gas_phase=gas,
        reactants=[A],
        products=[B]
    )

    # Create mechanism
    mechanism = mc.Mechanism(
        name="simple test mechanism",
        species=[A, B, C],
        phases=[gas],
        reactions=[reaction]
    )

    return mechanism


class TestMICMInitialization:
    """Test MICM class initialization."""

    def test_init_with_config_path(self):
        """Test initialization with a configuration path."""
        micm = MICM(config_path="configs/v0/analytical")
        assert micm is not None
        assert isinstance(micm.solver_type(), SolverType)

    def test_init_with_config_path_and_solver_type(self):
        """Test initialization with config path and explicit solver type."""
        micm = MICM(
            config_path="configs/v0/analytical",
            solver_type=SolverType.rosenbrock_standard_order
        )
        assert micm is not None
        assert micm.solver_type() == SolverType.rosenbrock_standard_order

    def test_init_with_mechanism(self):
        """Test initialization with a mechanism object."""
        mechanism = create_simple_mechanism()
        micm = MICM(mechanism=mechanism)
        assert micm is not None
        assert isinstance(micm.solver_type(), SolverType)

    def test_init_with_mechanism_and_solver_type(self):
        """Test initialization with mechanism and explicit solver type."""
        mechanism = create_simple_mechanism()
        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_standard_order
        )
        assert micm is not None
        assert micm.solver_type() == SolverType.rosenbrock_standard_order

    def test_init_with_mechanism_ignore_non_gas_phases(self):
        """Test initialization with mechanism and ignore_non_gas_phases flag."""
        mechanism = create_simple_mechanism()
        micm = MICM(
            mechanism=mechanism,
            ignore_non_gas_phases=True
        )
        assert micm is not None

    def test_init_without_config_or_mechanism_raises_error(self):
        """Test that initialization without config_path or mechanism raises ValueError."""
        with pytest.raises(ValueError, match="Either config_path or mechanism must be provided"):
            MICM()

    def test_init_with_both_config_and_mechanism_raises_error(self):
        """Test that initialization with both config_path and mechanism raises ValueError."""
        mechanism = create_simple_mechanism()
        with pytest.raises(ValueError, match="Only one of config_path or mechanism must be provided"):
            MICM(config_path="configs/v0/analytical", mechanism=mechanism)

    def test_default_solver_type(self):
        """Test that default solver type is rosenbrock_standard_order."""
        micm = MICM(config_path="configs/v0/analytical")
        assert micm.solver_type() == SolverType.rosenbrock_standard_order


class TestMICMSolverType:
    """Test MICM solver_type method."""

    def test_solver_type_returns_correct_type(self):
        """Test that solver_type() returns the correct SolverType."""
        micm = MICM(
            config_path="configs/v0/analytical",
            solver_type=SolverType.rosenbrock_standard_order
        )
        assert micm.solver_type() == SolverType.rosenbrock_standard_order

    def test_solver_type_with_different_types(self):
        """Test solver_type() with different solver types."""
        solver_types = [
            SolverType.rosenbrock_standard_order,
            SolverType.backward_euler_standard_order,
        ]

        for solver_type in solver_types:
            micm = MICM(
                config_path="configs/v0/analytical",
                solver_type=solver_type
            )
            assert micm.solver_type() == solver_type


class TestMICMCreateState:
    """Test MICM create_state method."""

    def test_create_state_default_single_grid_cell(self):
        """Test creating a state with default single grid cell."""
        micm = MICM(config_path="configs/v0/analytical")
        state = micm.create_state()
        assert isinstance(state, State)

    def test_create_state_single_grid_cell_explicit(self):
        """Test creating a state with explicitly specified single grid cell."""
        micm = MICM(config_path="configs/v0/analytical")
        state = micm.create_state(number_of_grid_cells=1)
        assert isinstance(state, State)

    def test_create_state_multiple_grid_cells(self):
        """Test creating a state with multiple grid cells."""
        micm = MICM(config_path="configs/v0/analytical")
        for num_cells in [2, 5, 10, 100]:
            state = micm.create_state(number_of_grid_cells=num_cells)
            assert isinstance(state, State)

    def test_create_multiple_states(self):
        """Test creating multiple state objects from the same solver."""
        micm = MICM(config_path="configs/v0/analytical")
        state1 = micm.create_state()
        state2 = micm.create_state(number_of_grid_cells=5)
        assert isinstance(state1, State)
        assert isinstance(state2, State)
        assert state1 is not state2


class TestMICMSolve:
    """Test MICM solve method."""

    def test_solve_with_valid_state_and_timestep(self):
        """Test solve with valid state and time step."""
        micm = MICM(config_path="configs/v0/analytical")
        state = micm.create_state()

        # Set initial conditions
        state.set_conditions(temperatures=298.15, pressures=101325.0, air_densities=1.2)
        state.set_concentrations({"A": 1.0, "B": 0.0, "C": 0.5})
        state.set_user_defined_rate_parameters({"USER.reaction 1": 0.001, "USER.reaction 2": 0.002})

        # Solve
        results = micm.solve(state, time_step=1.0)

        assert isinstance(results, SolverResult)
        assert results.state == SolverState.Converged

    def test_solve_with_multiple_grid_cells(self):
        """Test solve with multiple grid cells."""
        micm = MICM(config_path="configs/v0/analytical")
        num_cells = 5
        state = micm.create_state(number_of_grid_cells=num_cells)

        # Set initial conditions for all cells
        temperatures = [298.15] * num_cells
        pressures = [101325.0] * num_cells
        state.set_conditions(temperatures=temperatures, pressures=pressures)
        state.set_concentrations({"A": [1.0] * num_cells, "B": [0.0] * num_cells})
        state.set_user_defined_rate_parameters({
            "USER.reaction 1": [0.001] * num_cells,
            "USER.reaction 2": [0.002] * num_cells
        })

        # Solve
        results = micm.solve(state, time_step=1.0)

        assert isinstance(results, SolverResult)

    def test_solve_with_float_timestep(self):
        """Test solve with float time step."""
        micm = MICM(config_path="configs/v0/analytical")
        state = micm.create_state()
        state.set_conditions(temperatures=298.15, pressures=101325.0, air_densities=1.2)
        state.set_concentrations({"A": 1.0})
        state.set_user_defined_rate_parameters({"USER.reaction 1": 0.001, "USER.reaction 2": 0.002})

        results = micm.solve(state, time_step=1.5)
        assert isinstance(results, SolverResult)

    def test_solve_with_int_timestep(self):
        """Test solve with integer time step."""
        micm = MICM(config_path="configs/v0/analytical")
        state = micm.create_state()
        state.set_conditions(temperatures=298.15, pressures=101325.0, air_densities=1.2)
        state.set_concentrations({"A": 1.0})
        state.set_user_defined_rate_parameters({"USER.reaction 1": 0.001, "USER.reaction 2": 0.002})

        results = micm.solve(state, time_step=1)
        assert isinstance(results, SolverResult)

    def test_solve_with_invalid_state_type_raises_error(self):
        """Test that solve with invalid state type raises TypeError."""
        micm = MICM(config_path="configs/v0/analytical")

        with pytest.raises(TypeError, match="state must be an instance of State"):
            micm.solve("not a state", time_step=1.0)

    def test_solve_with_invalid_timestep_type_raises_error(self):
        """Test that solve with invalid time step type raises TypeError."""
        micm = MICM(config_path="configs/v0/analytical")
        state = micm.create_state()

        with pytest.raises(TypeError, match="time_step must be an int or float"):
            micm.solve(state, time_step="not a number")

    def test_solve_with_none_state_raises_error(self):
        """Test that solve with None state raises TypeError."""
        micm = MICM(config_path="configs/v0/analytical")

        with pytest.raises(TypeError, match="state must be an instance of State"):
            micm.solve(None, time_step=1.0)

    def test_solve_multiple_times(self):
        """Test solving multiple times with the same state."""
        micm = MICM(config_path="configs/v0/analytical")
        state = micm.create_state()
        state.set_conditions(temperatures=298.15, pressures=101325.0, air_densities=1.2)
        state.set_concentrations({"A": 1.0, "B": 0.0})
        state.set_user_defined_rate_parameters({"USER.reaction 1": 0.001, "USER.reaction 2": 0.002})

        # Solve multiple times
        for _ in range(5):
            results = micm.solve(state, time_step=1.0)
            assert isinstance(results, SolverResult)


class TestMICMWithMechanism:
    """Test MICM class with mechanism-based initialization."""

    def test_mechanism_initialization_creates_solver(self):
        """Test that mechanism initialization creates a working solver."""
        mechanism = create_simple_mechanism()
        micm = MICM(mechanism=mechanism)
        state = micm.create_state()

        assert isinstance(state, State)

    def test_mechanism_based_solve(self):
        """Test solving with mechanism-based initialization."""
        mechanism = create_simple_mechanism()
        micm = MICM(mechanism=mechanism)
        state = micm.create_state()

        # Set initial conditions
        state.set_conditions(temperatures=298.15, pressures=101325.0, air_densities=1.2)
        state.set_concentrations({"A": 1.0, "B": 0.0, "C": 0.0})

        # Solve
        results = micm.solve(state, time_step=1.0)

        assert isinstance(results, SolverResult)


class TestMICMIntegration:
    """Integration tests for MICM class."""

    def test_end_to_end_workflow(self):
        """Test complete workflow: initialize, create state, set conditions, solve."""
        # Initialize solver
        micm = MICM(
            config_path="configs/v0/analytical",
            solver_type=SolverType.rosenbrock_standard_order
        )

        # Create state
        state = micm.create_state()

        # Set conditions
        state.set_conditions(temperatures=298.15, pressures=101325.0, air_densities=1.2)
        state.set_concentrations({
            "A": 0.75,
            "B": 0.0,
            "C": 0.4,
            "D": 0.8,
            "E": 0.0,
            "F": 0.1
        })
        state.set_user_defined_rate_parameters({
            "USER.reaction 1": 0.001,
            "USER.reaction 2": 0.002
        })

        # Solve
        results = micm.solve(state, time_step=1.0)

        # Verify results
        assert isinstance(results, SolverResult)

        # Get updated concentrations
        concentrations = state.get_concentrations()
        assert "A" in concentrations
        assert "B" in concentrations

    def test_end_to_end_with_mechanism(self):
        """Test complete workflow using mechanism initialization."""
        # Create mechanism
        mechanism = create_simple_mechanism()

        # Initialize solver
        micm = MICM(
            mechanism=mechanism,
            solver_type=SolverType.rosenbrock_standard_order
        )

        # Create state
        state = micm.create_state()

        # Set conditions
        state.set_conditions(temperatures=298.15, pressures=101325.0, air_densities=1.2)
        state.set_concentrations({"A": 1.0, "B": 0.0, "C": 0.0})

        # Solve
        results = micm.solve(state, time_step=1.0)

        # Verify results
        assert isinstance(results, SolverResult)


if __name__ == '__main__':
    pytest.main([__file__, '-v'])
