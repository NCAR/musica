// Chapman mechanism integration test
// Matches python/musica/test/integration/test_chapman.py

const path = require('path');
const musica = require('musica-addon');
const { MICM, SolverType } = musica.micmSolver;


const CONFIG_PATH = path.join(__dirname, '../../../configs/v1/chapman/config.json');

function testChapmanV1() {
    console.log('\n============================================================');
    console.log('Chapman Mechanism Test (v1 config)');
    console.log('============================================================\n');

    const solver = new MICM({
        config_path: CONFIG_PATH,
        solver_type: SolverType.rosenbrock_standard_order
    });

    const state = solver.createState(1);

    const timeStep = 200.0;
    const temperature = 272.5;
    const pressure = 101253.3;

    const rateConstants = {
        'PHOTO.jO2': 2.42e-17,
        'PHOTO.jO3->O': 1.15e-5,
        'PHOTO.jO3->O1D': 6.61e-9
    };

    const initialConcentrations = {
        'O2': 0.75,
        'O': 0.0,
        'O1D': 0.0,
        'O3': 0.0000081
    };

    // Set conditions
    state.setConditions({
        temperatures: temperature,
        pressures: pressure
    });

    // Set initial concentrations
    state.setConcentrations(initialConcentrations);

    // Set user-defined rate parameters (photolysis rates)
    state.setUserDefinedRateParameters(rateConstants);

    // Solve
    solver.solve(state, timeStep);

    // Get results
    const concentrations = state.getConcentrations();

    // Verify results
    const tolerance = 1e-5;

    function isClose(a, b, tol = tolerance) {
        return Math.abs(a - b) / Math.max(Math.abs(a), Math.abs(b)) < tol;
    }

    console.log('Final concentrations:');
    console.log('  O2:', concentrations.O2[0]);
    console.log('  O:', concentrations.O[0]);
    console.log('  O1D:', concentrations.O1D[0]);
    console.log('  O3:', concentrations.O3[0]);

    // Assertions
    if (!isClose(concentrations.O2[0], 0.75)) {
        throw new Error(`O2 concentration mismatch: ${concentrations.O2[0]} (expected ~0.75)`);
    }

    if (concentrations.O[0] <= 0.0) {
        throw new Error(`O concentration should be > 0, got ${concentrations.O[0]}`);
    }

    if (concentrations.O1D[0] <= 0.0) {
        throw new Error(`O1D concentration should be > 0, got ${concentrations.O1D[0]}`);
    }

    if (concentrations.O3[0] === 0.0000081) {
        throw new Error(`O3 concentration should have changed from initial value`);
    }

    console.log('\n✓ Chapman mechanism test passed!\n');
}

// Run tests
try {
    testChapmanV1();
    console.log('=> ALL CHAPMAN TESTS PASSED! <=\n');
    process.exit(0);
} catch (error) {
    console.error('\n=> TEST FAILED! <=');
    console.error(error.message);
    console.error(error.stack);
    process.exit(1);
}
