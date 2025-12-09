const { describe, it } = require('node:test');
const assert = require('node:assert');
const path = require('path');
const musica = require('musica-addon');
const { MICM, SolverType, GAS_CONSTANT } = musica.micmSolver;
const { isClose } = require('../util/testUtils');

// Test configuration
const CONFIG_PATH = path.join(__dirname, '../../../configs/v0/analytical');

// NOTE: Vector-ordered Rosenbrock currently only supports up to 4 grid cells
// This is because the C++ implementation requires splitting into multiple
// internal states for >4 cells (the vector size), which is not yet implemented
// in the JavaScript bindings. Python handles this by creating multiple states.
const maxCells = 4; // Vector size limitation


/**
 * Test single grid cell analytical solution
 * Equivalent to TestSingleGridCell in Python
 */
function testSingleGridCell(solver, state, timeStep, places = 5) {
    const temperature = 272.5;
    const pressure = 101253.3;
    const airDensity = pressure / (GAS_CONSTANT * temperature);

    const rateConstants = {
        'USER.reaction 1': 0.001,
        'USER.reaction 2': 0.002
    };

    const concentrations = {
        A: 0.75,
        B: 0,
        C: 0.4,
        D: 0.8,
        E: 0,
        F: 0.1
    };

    state.setConditions({ temperatures: temperature, pressures: pressure, air_densities: airDensity });
    state.setConcentrations(concentrations);
    state.setUserDefinedRateParameters(rateConstants);

    // Test to make sure a second call with empty dictionary does not change values
    state.setConcentrations({});
    state.setUserDefinedRateParameters({});

    const initialConcentrations = state.getConcentrations();
    const initialRateParameters = state.getUserDefinedRateParameters();
    const initialConditions = state.getConditions();

    // Verify initial conditions
    assert.ok(isClose(initialConcentrations.A[0], concentrations.A, 1e-13), 'Initial A concentration mismatch');
    assert.ok(isClose(initialConcentrations.B[0], concentrations.B, 1e-13), 'Initial B concentration mismatch');
    assert.ok(isClose(initialConcentrations.C[0], concentrations.C, 1e-13), 'Initial C concentration mismatch');
    assert.ok(isClose(initialConcentrations.D[0], concentrations.D, 1e-13), 'Initial D concentration mismatch');
    assert.ok(isClose(initialConcentrations.E[0], concentrations.E, 1e-13), 'Initial E concentration mismatch');
    assert.ok(isClose(initialConcentrations.F[0], concentrations.F, 1e-13), 'Initial F concentration mismatch');
    assert.ok(isClose(initialRateParameters['USER.reaction 1'][0], rateConstants['USER.reaction 1'], 1e-13), 'Rate parameter 1 mismatch');
    assert.ok(isClose(initialRateParameters['USER.reaction 2'][0], rateConstants['USER.reaction 2'], 1e-13), 'Rate parameter 2 mismatch');
    assert.ok(isClose(initialConditions.temperature[0], temperature, 1e-13), 'Temperature mismatch');
    assert.ok(isClose(initialConditions.pressure[0], pressure, 1e-13), 'Pressure mismatch');
    assert.ok(isClose(initialConditions.air_density[0], airDensity, 1e-13), 'Air density mismatch');

    timeStep = 1;
    const simLength = 100;

    let currTime = timeStep;
    const initialA = initialConcentrations.A[0];
    const initialC = initialConcentrations.C[0];
    const initialD = initialConcentrations.D[0];
    const initialF = initialConcentrations.F[0];

    const tolerance = Math.pow(10, -places);

    // Integrate and compare with analytical solution
    while (currTime <= simLength) {
        solver.solve(state, timeStep);
        const conc = state.getConcentrations();

        const k1 = rateConstants['USER.reaction 1'];
        const k2 = rateConstants['USER.reaction 2'];
        const k3 = 0.004 * Math.exp(50.0 / temperature);
        const k4 = 0.012 * Math.exp(75.0 / temperature) *
            Math.pow(temperature / 50.0, -2) * (1.0 + 1.0e-6 * pressure);

        // Analytical solutions
        const A_conc = initialA * Math.exp(-k3 * currTime);
        const B_conc = initialA * (k3 / (k4 - k3)) *
            (Math.exp(-k3 * currTime) - Math.exp(-k4 * currTime));
        const C_conc = initialC + initialA *
            (1.0 + (k3 * Math.exp(-k4 * currTime) - k4 * Math.exp(-k3 * currTime)) / (k4 - k3));
        const D_conc = initialD * Math.exp(-k1 * currTime);
        const E_conc = initialD * (k1 / (k2 - k1)) *
            (Math.exp(-k1 * currTime) - Math.exp(-k2 * currTime));
        const F_conc = initialF + initialD *
            (1.0 + (k1 * Math.exp(-k2 * currTime) - k2 * Math.exp(-k1 * currTime)) / (k2 - k1));

        // Check concentrations
        assert.ok(isClose(conc.A[0], A_conc, tolerance),
            `A concentration mismatch at t=${currTime}: ${conc.A[0]} vs ${A_conc}`);
        assert.ok(isClose(conc.B[0], B_conc, tolerance),
            `B concentration mismatch at t=${currTime}: ${conc.B[0]} vs ${B_conc}`);
        assert.ok(isClose(conc.C[0], C_conc, tolerance),
            `C concentration mismatch at t=${currTime}: ${conc.C[0]} vs ${C_conc}`);
        assert.ok(isClose(conc.D[0], D_conc, tolerance),
            `D concentration mismatch at t=${currTime}: ${conc.D[0]} vs ${D_conc}`);
        assert.ok(isClose(conc.E[0], E_conc, tolerance),
            `E concentration mismatch at t=${currTime}: ${conc.E[0]} vs ${E_conc}`);
        assert.ok(isClose(conc.F[0], F_conc, tolerance),
            `F concentration mismatch at t=${currTime}: ${conc.F[0]} vs ${F_conc}`);

        currTime += timeStep;
    }
}

/**
 * Test multiple grid cells analytical solution
 * Equivalent to TestMultipleGridCell in Python
 */
function testMultipleGridCell(solver, state, numGridCells, timeStep, places = 5) {
    const concentrations = {
        A: [],
        B: [],
        C: [],
        D: [],
        E: [],
        F: []
    };
    const rateConstants = {
        'USER.reaction 1': [],
        'USER.reaction 2': []
    };
    const temperatures = [];
    const pressures = [];

    // Generate random initial conditions for each grid cell
    for (let i = 0; i < numGridCells; i++) {
        temperatures.push(275.0 + (Math.random() - 0.5) * 100.0);
        pressures.push(101253.3 + (Math.random() - 0.5) * 1000.0);
        concentrations.A.push(0.75 + (Math.random() - 0.5) * 0.1);
        concentrations.B.push(0);
        concentrations.C.push(0.4 + (Math.random() - 0.5) * 0.1);
        concentrations.D.push(0.8 + (Math.random() - 0.5) * 0.1);
        concentrations.E.push(0);
        concentrations.F.push(0.1 + (Math.random() - 0.5) * 0.1);
        rateConstants['USER.reaction 1'].push(0.001 + (Math.random() - 0.5) * 0.0002);
        rateConstants['USER.reaction 2'].push(0.002 + (Math.random() - 0.5) * 0.0002);
    }

    state.setConditions({ temperatures, pressures }); // Air density calculated automatically
    state.setConcentrations(concentrations);
    state.setUserDefinedRateParameters(rateConstants);

    const initialConcentrations = state.getConcentrations();
    const initialRateParameters = state.getUserDefinedRateParameters();
    const conditions = state.getConditions();

    // Verify initial conditions
    for (let i = 0; i < numGridCells; i++) {
        assert.ok(isClose(initialConcentrations.A[i], concentrations.A[i], 1e-13));
        assert.ok(isClose(initialConcentrations.B[i], concentrations.B[i], 1e-13));
        assert.ok(isClose(initialConcentrations.C[i], concentrations.C[i], 1e-13));
        assert.ok(isClose(initialConcentrations.D[i], concentrations.D[i], 1e-13));
        assert.ok(isClose(initialConcentrations.E[i], concentrations.E[i], 1e-13));
        assert.ok(isClose(initialConcentrations.F[i], concentrations.F[i], 1e-13));
        assert.ok(isClose(initialRateParameters['USER.reaction 1'][i], rateConstants['USER.reaction 1'][i], 1e-13));
        assert.ok(isClose(initialRateParameters['USER.reaction 2'][i], rateConstants['USER.reaction 2'][i], 1e-13));
        assert.ok(isClose(conditions.temperature[i], temperatures[i], 1e-13));
        assert.ok(isClose(conditions.pressure[i], pressures[i], 1e-13));
        const expectedAirDensity = pressures[i] / (8.31446261815324 * temperatures[i]);
        assert.ok(isClose(conditions.air_density[i], expectedAirDensity, 1e-13));
    }

    timeStep = 1;
    const simLength = 100;

    let currTime = timeStep;
    const initialA = initialConcentrations.A.slice();
    const initialC = initialConcentrations.C.slice();
    const initialD = initialConcentrations.D.slice();
    const initialF = initialConcentrations.F.slice();

    const k1 = [];
    const k2 = [];
    const k3 = [];
    const k4 = [];
    for (let i = 0; i < numGridCells; i++) {
        k1.push(rateConstants['USER.reaction 1'][i]);
        k2.push(rateConstants['USER.reaction 2'][i]);
        k3.push(0.004 * Math.exp(50.0 / temperatures[i]));
        k4.push(0.012 * Math.exp(75.0 / temperatures[i]) *
            Math.pow(temperatures[i] / 50.0, -2) * (1.0 + 1.0e-6 * pressures[i]));
    }

    const tolerance = Math.pow(10, -places);

    // Integrate and compare with analytical solution
    while (currTime <= simLength) {
        solver.solve(state, timeStep);
        const conc = state.getConcentrations();

        for (let i = 0; i < numGridCells; i++) {
            // Analytical solutions
            const A_conc = initialA[i] * Math.exp(-k3[i] * currTime);
            const B_conc = initialA[i] * (k3[i] / (k4[i] - k3[i])) *
                (Math.exp(-k3[i] * currTime) - Math.exp(-k4[i] * currTime));
            const C_conc = initialC[i] + initialA[i] * (1.0 + (
                k3[i] * Math.exp(-k4[i] * currTime) - k4[i] * Math.exp(-k3[i] * currTime)) / (k4[i] - k3[i]));
            const D_conc = initialD[i] * Math.exp(-k1[i] * currTime);
            const E_conc = initialD[i] * (k1[i] / (k2[i] - k1[i])) *
                (Math.exp(-k1[i] * currTime) - Math.exp(-k2[i] * currTime));
            const F_conc = initialF[i] + initialD[i] * (1.0 + (
                k1[i] * Math.exp(-k2[i] * currTime) - k2[i] * Math.exp(-k1[i] * currTime)) / (k2[i] - k1[i]));

            // Check concentrations
            assert.ok(isClose(conc.A[i], A_conc, tolerance),
                `Grid cell ${i} of ${numGridCells}: A concentration mismatch. Initial A: ${initialConcentrations.A[i]}`);
            assert.ok(isClose(conc.B[i], B_conc, tolerance),
                `Grid cell ${i} of ${numGridCells}: B concentration mismatch. Initial B: ${initialConcentrations.B[i]}`);
            assert.ok(isClose(conc.C[i], C_conc, tolerance),
                `Grid cell ${i} of ${numGridCells}: C concentration mismatch. Initial C: ${initialConcentrations.C[i]}`);
            assert.ok(isClose(conc.D[i], D_conc, tolerance),
                `Grid cell ${i} of ${numGridCells}: D concentration mismatch. Initial D: ${initialConcentrations.D[i]}`);
            assert.ok(isClose(conc.E[i], E_conc, tolerance),
                `Grid cell ${i} of ${numGridCells}: E concentration mismatch. Initial E: ${initialConcentrations.E[i]}`);
            assert.ok(isClose(conc.F[i], F_conc, tolerance),
                `Grid cell ${i} of ${numGridCells}: F concentration mismatch. Initial F: ${initialConcentrations.F[i]}`);
        }

        currTime += timeStep;
    }
}

// Test suite for single grid cell - Standard Rosenbrock
describe('Analytical - Single grid cell - Standard Rosenbrock', () => {
    it('should match analytical solution', () => {
        const solver = MICM.fromConfigPath(
            CONFIG_PATH,
            SolverType.rosenbrock_standard_order
        );
        const state = solver.createState(1);
        testSingleGridCell(solver, state, 200.0, 5);
    });
});

// Test suite for multiple grid cells - Standard Rosenbrock
describe('Analytical - Multiple grid cells - Standard Rosenbrock', () => {
    for (let i = 1; i <= maxCells; i++) {
        it(`should match analytical solution for ${i} grid cells`, () => {
            const solver = MICM.fromConfigPath(
                CONFIG_PATH,
                SolverType.rosenbrock_standard_order
            );
            const state = solver.createState(i);
            testMultipleGridCell(solver, state, i, 200.0, 5);
        });
    }
});

// Test suite for single grid cell - Rosenbrock (vector-ordered)
describe('Analytical - Single grid cell - Rosenbrock', () => {
    it('should match analytical solution', () => {
        const solver = MICM.fromConfigPath(
            CONFIG_PATH,
            SolverType.rosenbrock
        );
        const state = solver.createState(1);
        testSingleGridCell(solver, state, 200.0, 5);
    });
});

// Test suite for multiple grid cells - Rosenbrock (vector-ordered)
describe('Analytical - Multiple grid cells - Rosenbrock', () => {
    for (let i = 1; i <= maxCells; i++) {
        it(`should match analytical solution for ${i} grid cells`, () => {
            const solver = MICM.fromConfigPath(
                CONFIG_PATH,
                SolverType.rosenbrock
            );
            const state = solver.createState(i);
            testMultipleGridCell(solver, state, i, 200.0, 5);
        });
    }
});

// Test suite for single grid cell - Backward Euler
describe('Analytical - Single grid cell - Backward Euler', () => {
    it('should match analytical solution', () => {
        const solver = MICM.fromConfigPath(
            CONFIG_PATH,
            SolverType.backward_euler
        );
        const state = solver.createState(1);
        testSingleGridCell(solver, state, 10.0, 2);
    });
});

// Test suite for multiple grid cells - Backward Euler
describe('Analytical - Multiple grid cells - Backward Euler', () => {
    for (let i = 1; i <= maxCells; i++) {
        it(`should match analytical solution for ${i} grid cells`, () => {
            const solver = MICM.fromConfigPath(
                CONFIG_PATH,
                SolverType.backward_euler
            );
            const state = solver.createState(i);
            testMultipleGridCell(solver, state, i, 10.0, 2);
        });
    }
});

// Test suite for single grid cell - Backward Euler Standard Order
describe('Analytical - Single grid cell - Backward Euler Standard Order', () => {
    it('should match analytical solution', () => {
        const solver = MICM.fromConfigPath(
            CONFIG_PATH,
            SolverType.backward_euler_standard_order
        );
        const state = solver.createState(1);
        testSingleGridCell(solver, state, 10.0, 2);
    });
});

// Test suite for multiple grid cells - Backward Euler Standard Order
describe('Analytical - Multiple grid cells - Backward Euler Standard Order', () => {
    for (let i = 1; i <= maxCells; i++) {
        it(`should match analytical solution for ${i} grid cells`, () => {
            const solver = MICM.fromConfigPath(
                CONFIG_PATH,
                SolverType.backward_euler_standard_order
            );
            const state = solver.createState(i);
            testMultipleGridCell(solver, state, i, 10.0, 2);
        });
    }
});
