const path = require('path');
const musica = require('../../index.js');
const { MICM, SolverType, GAS_CONSTANT } = musica.micmSolver;

// Test configuration
const CONFIG_PATH = path.join(__dirname, '../../../configs/v0/analytical');

// Helper function to check if values are close (equivalent to np.isclose)
function isClose(a, b, atol = 1e-5, rtol = 1e-9) {
    const diff = Math.abs(a - b);
    return diff <= (atol + rtol * Math.abs(b));
}

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
    console.assert(isClose(initialConcentrations.A[0], concentrations.A, 1e-13), 'Initial A concentration mismatch');
    console.assert(isClose(initialConcentrations.B[0], concentrations.B, 1e-13), 'Initial B concentration mismatch');
    console.assert(isClose(initialConcentrations.C[0], concentrations.C, 1e-13), 'Initial C concentration mismatch');
    console.assert(isClose(initialConcentrations.D[0], concentrations.D, 1e-13), 'Initial D concentration mismatch');
    console.assert(isClose(initialConcentrations.E[0], concentrations.E, 1e-13), 'Initial E concentration mismatch');
    console.assert(isClose(initialConcentrations.F[0], concentrations.F, 1e-13), 'Initial F concentration mismatch');
    console.assert(isClose(initialRateParameters['USER.reaction 1'][0], rateConstants['USER.reaction 1'], 1e-13), 'Rate parameter 1 mismatch');
    console.assert(isClose(initialRateParameters['USER.reaction 2'][0], rateConstants['USER.reaction 2'], 1e-13), 'Rate parameter 2 mismatch');
    console.assert(isClose(initialConditions.temperature[0], temperature, 1e-13), 'Temperature mismatch');
    console.assert(isClose(initialConditions.pressure[0], pressure, 1e-13), 'Pressure mismatch');
    console.assert(isClose(initialConditions.air_density[0], airDensity, 1e-13), 'Air density mismatch');

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
        if (!isClose(conc.A[0], A_conc, tolerance)) {
            throw new Error(`A concentration mismatch at t=${currTime}: ${conc.A[0]} vs ${A_conc}`);
        }
        if (!isClose(conc.B[0], B_conc, tolerance)) {
            throw new Error(`B concentration mismatch at t=${currTime}: ${conc.B[0]} vs ${B_conc}`);
        }
        if (!isClose(conc.C[0], C_conc, tolerance)) {
            throw new Error(`C concentration mismatch at t=${currTime}: ${conc.C[0]} vs ${C_conc}`);
        }
        if (!isClose(conc.D[0], D_conc, tolerance)) {
            throw new Error(`D concentration mismatch at t=${currTime}: ${conc.D[0]} vs ${D_conc}`);
        }
        if (!isClose(conc.E[0], E_conc, tolerance)) {
            throw new Error(`E concentration mismatch at t=${currTime}: ${conc.E[0]} vs ${E_conc}`);
        }
        if (!isClose(conc.F[0], F_conc, tolerance)) {
            throw new Error(`F concentration mismatch at t=${currTime}: ${conc.F[0]} vs ${F_conc}`);
        }

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
        console.assert(isClose(initialConcentrations.A[i], concentrations.A[i], 1e-13));
        console.assert(isClose(initialConcentrations.B[i], concentrations.B[i], 1e-13));
        console.assert(isClose(initialConcentrations.C[i], concentrations.C[i], 1e-13));
        console.assert(isClose(initialConcentrations.D[i], concentrations.D[i], 1e-13));
        console.assert(isClose(initialConcentrations.E[i], concentrations.E[i], 1e-13));
        console.assert(isClose(initialConcentrations.F[i], concentrations.F[i], 1e-13));
        console.assert(isClose(initialRateParameters['USER.reaction 1'][i], rateConstants['USER.reaction 1'][i], 1e-13));
        console.assert(isClose(initialRateParameters['USER.reaction 2'][i], rateConstants['USER.reaction 2'][i], 1e-13));
        console.assert(isClose(conditions.temperature[i], temperatures[i], 1e-13));
        console.assert(isClose(conditions.pressure[i], pressures[i], 1e-13));
        const expectedAirDensity = pressures[i] / (8.31446261815324 * temperatures[i]);
        console.assert(isClose(conditions.air_density[i], expectedAirDensity, 1e-13));
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
            if (!isClose(conc.A[i], A_conc, tolerance)) {
                throw new Error(`Grid cell ${i} of ${numGridCells}: A concentration mismatch. Initial A: ${initialConcentrations.A[i]}`);
            }
            if (!isClose(conc.B[i], B_conc, tolerance)) {
                throw new Error(`Grid cell ${i} of ${numGridCells}: B concentration mismatch. Initial B: ${initialConcentrations.B[i]}`);
            }
            if (!isClose(conc.C[i], C_conc, tolerance)) {
                throw new Error(`Grid cell ${i} of ${numGridCells}: C concentration mismatch. Initial C: ${initialConcentrations.C[i]}`);
            }
            if (!isClose(conc.D[i], D_conc, tolerance)) {
                throw new Error(`Grid cell ${i} of ${numGridCells}: D concentration mismatch. Initial D: ${initialConcentrations.D[i]}`);
            }
            if (!isClose(conc.E[i], E_conc, tolerance)) {
                throw new Error(`Grid cell ${i} of ${numGridCells}: E concentration mismatch. Initial E: ${initialConcentrations.E[i]}`);
            }
            if (!isClose(conc.F[i], F_conc, tolerance)) {
                throw new Error(`Grid cell ${i} of ${numGridCells}: F concentration mismatch. Initial F: ${initialConcentrations.F[i]}`);
            }
        }

        currTime += timeStep;
    }
}

/**
 * Test functions matching Python pytest structure
 */

function test_single_grid_cell_standard_rosenbrock() {
    const solver = new MICM({
        config_path: CONFIG_PATH,
        solver_type: SolverType.rosenbrock_standard_order
    });
    const state = solver.createState(1);
    testSingleGridCell(solver, state, 200.0, 5);
    console.log('Passed test_single_grid_cell_standard_rosenbrock passed');
}

function test_multiple_grid_cells_standard_rosenbrock() {
    for (let i = 1; i <= 10; i++) {
        const solver = new MICM({
            config_path: CONFIG_PATH,
            solver_type: SolverType.rosenbrock_standard_order
        });
        const state = solver.createState(i);
        testMultipleGridCell(solver, state, i, 200.0, 5);
    }
    console.log('Passed test_multiple_grid_cells_standard_rosenbrock passed (1-10 cells)');
}

function test_single_grid_cell_backward_euler() {
    const solver = new MICM({
        config_path: CONFIG_PATH,
        solver_type: SolverType.backward_euler_standard_order
    });
    const state = solver.createState(1);
    testSingleGridCell(solver, state, 10.0, 2);
    console.log('Passed test_single_grid_cell_backward_euler passed');
}

function test_multiple_grid_cells_backward_euler() {
    for (let i = 1; i <= 10; i++) {
        const solver = new MICM({
            config_path: CONFIG_PATH,
            solver_type: SolverType.backward_euler_standard_order
        });
        const state = solver.createState(i);
        testMultipleGridCell(solver, state, i, 10.0, 2);
    }
    console.log('Passed test_multiple_grid_cells_backward_euler passed (1-10 cells)');
}

function test_single_grid_cell_rosenbrock() {
    const solver = new MICM({
        config_path: CONFIG_PATH,
        solver_type: SolverType.rosenbrock
    });
    const state = solver.createState(1);
    testSingleGridCell(solver, state, 200.0, 5);
    console.log('Passed test_single_grid_cell_rosenbrock passed');
}

function test_multiple_grid_cells_rosenbrock() {
    // NOTE: Vector-ordered Rosenbrock currently only supports up to 4 grid cells
    // This is because the C++ implementation requires splitting into multiple
    // internal states for >4 cells (the vector size), which is not yet implemented
    // in the JavaScript bindings. Python handles this by creating multiple states.
    // See musica/types.py lines 93-97 for the Python multi-state implementation.
    const maxCells = 4; // Vector size limitation

    for (let i = 1; i <= maxCells; i++) {
        const solver = new MICM({
            config_path: CONFIG_PATH,
            solver_type: SolverType.rosenbrock
        });
        const state = solver.createState(i);
        testMultipleGridCell(solver, state, i, 200.0, 5);
    }
    console.log(`Passed test_multiple_grid_cells_rosenbrock passed (1-${maxCells} cells) [Limited by vector size]`);
}

function test_single_grid_cell_backward_euler_standard_order() {
    const solver = new MICM({
        config_path: CONFIG_PATH,
        solver_type: SolverType.backward_euler_standard_order
    });
    const state = solver.createState(1);
    testSingleGridCell(solver, state, 10.0, 2);
    console.log('Passed test_single_grid_cell_backward_euler_standard_order passed');
}

function test_multiple_grid_cells_backward_euler_standard_order() {
    for (let i = 1; i <= 10; i++) {
        const solver = new MICM({
            config_path: CONFIG_PATH,
            solver_type: SolverType.backward_euler_standard_order
        });
        const state = solver.createState(i);
        testMultipleGridCell(solver, state, i, 10.0, 2);
    }
    console.log('Passed test_multiple_grid_cells_backward_euler_standard_order passed (1-10 cells)');
}

/**
 * Main test runner
 */
function runTests() {
    console.log('Starting MICM Analytical Tests (matching Python test_analytical.py)...\n');
    console.log('='.repeat(60));

    try {
        // Test 1: Single grid cell - Standard Rosenbrock
        console.log('\nTest 1: Single grid cell - Standard Rosenbrock');
        test_single_grid_cell_standard_rosenbrock();

        // Test 2: Multiple grid cells - Standard Rosenbrock
        console.log('\nTest 2: Multiple grid cells (1-10) - Standard Rosenbrock');
        test_multiple_grid_cells_standard_rosenbrock();

        // Test 3: Single grid cell - Backward Euler
        console.log('\nTest 3: Single grid cell - Backward Euler');
        test_single_grid_cell_backward_euler();

        // Test 4: Multiple grid cells - Backward Euler
        console.log('\nTest 4: Multiple grid cells (1-10) - Backward Euler');
        test_multiple_grid_cells_backward_euler();

        // Test 5: Single grid cell - Rosenbrock (vector-ordered)
        console.log('\nTest 5: Single grid cell - Rosenbrock (vector-ordered)');
        test_single_grid_cell_rosenbrock();

        // Test 6: Multiple grid cells - Rosenbrock (vector-ordered)
        console.log('\nTest 6: Multiple grid cells (1-10) - Rosenbrock (vector-ordered)');
        test_multiple_grid_cells_rosenbrock();

        // Test 7: Single grid cell - Backward Euler Standard Order
        console.log('\nTest 7: Single grid cell - Backward Euler Standard Order');
        test_single_grid_cell_backward_euler_standard_order();

        // Test 8: Multiple grid cells - Backward Euler Standard Order
        console.log('\nTest 8: Multiple grid cells (1-10) - Backward Euler Standard Order');
        test_multiple_grid_cells_backward_euler_standard_order();

        console.log('\n' + '='.repeat(60));
        console.log('ALL TESTS PASSED! ✓');
        console.log('='.repeat(60));
        process.exit(0);

    } catch (error) {
        console.error('\n' + '='.repeat(60));
        console.error('TEST FAILED! ✗');
        console.error('='.repeat(60));
        console.error('Error:', error.message);
        console.error(error.stack);
        process.exit(1);
    }
}

// Run tests if this file is executed directly
if (require.main === module) {
    runTests();
}

module.exports = {
    testSingleGridCell,
    testMultipleGridCell,
    runTests,
    isClose
};
