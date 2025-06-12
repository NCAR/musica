import pytest
import musica
import musica.mechanism_configuration as mc


def test_solve_with_config_path():
    solver = musica.MICM(
        config_path="configs/chapman",
        solver_type=musica.SolverType.rosenbrock_standard_order,
    )
    TestSolve(solver)


def test_solve_with_mechanism():
    solver = musica.MICM(
        mechanism=GetMechanism(),
        solver_type=musica.SolverType.rosenbrock_standard_order,
    )
    TestSolve(solver)


def TestSolve(solver):
    state = solver.create_state()

    time_step = 200.0
    temperature = 272.5
    pressure = 101253.3

    rate_constants = {
        "PHOTO.jO2": 2.42e-17,
        "PHOTO.jO3->O": 1.15e-5,
        "PHOTO.jO3->O1D": 6.61e-9
    }

    initial_concentrations = {
        "O2": 0.75,
        "O": 0.0,
        "O1D": 0.0,
        "O3": 0.0000081
    }

    # Test setting int values
    state.set_conditions(temperatures=272, pressures=101325)

    # Set actual test conditions
    state.set_conditions(temperatures=temperature, pressures=pressure)
    state.set_concentrations(initial_concentrations)
    state.set_user_defined_rate_parameters(rate_constants)

    solver.solve(state, time_step)
    concentrations = state.get_concentrations()

    assert pytest.approx(concentrations["O2"][0], rel=1e-5) == 0.75
    assert concentrations["O"][0] > 0.0
    assert concentrations["O1D"][0] > 0.0
    assert concentrations["O3"][0] != 0.0000081


def GetMechanism():
    M = mc.Species(tracer_type="THIRD_BODY")
    O2 = mc.Species(name="O2", tracer_type="CONSTANT")
    O = mc.Species(name="O", other_properties={
                   "__absolute_tolerance": "1e-12"})
    O1D = mc.Species(name="O1D", other_properties={
                     "__absolute_tolerance": "1e-12"})
    O3 = mc.Species(
        name="O3",
        molecular_weight_kg_mol=0.048,
        other_properties={
            "__absolute_tolerance": "1e-12",
            "__long_name": "ozone",
            "__atmos": "3",
            "__do_advect": "True",
        })
    gas = mc.Phase(
        name="gas",
        species=[O2, O, O1D, O3, M],
    )
    jO2 = mc.Photolysis(
        name="jO2",
        reactants=[O2],
        products=[(2, O)],
    )
    R2 = mc.Arrhenius(
        name="R2",
        A=8.018e-17,
        reactants=[O, O2],
        products=[O3],
    )
    jO31 = mc.Photolysis(
        name="jO3->O",
        reactants=[O3],
        products=[O, O2],
    )
    R4 = mc.Arrhenius(
        name="R4",
        A=1.576e-15,
        reactants=[O, O3],
        products=[(2, O2)],
    )
    jO32 = mc.Photolysis(
        name="jO3->O1D",
        reactants=[O3],
        products=[O1D, O2],
    )
    R6 = mc.Arrhenius(
        name="R6",
        A=7.11e-11,
        reactants=[O1D, M],
        products=[O, M],
    )
    R7 = mc.Arrhenius(
        name="R7",
        A=1.2e-10,
        reactants=[O1D, O3],
        products=[(2, O2)],
    )
    return mc.Mechanism(
        name="Chapman",
        species=[O2, O, O1D, O3, M],
        phases=[gas],
        reactions=[jO2, R2, jO31, R4, jO32, R6, R7],
    )
