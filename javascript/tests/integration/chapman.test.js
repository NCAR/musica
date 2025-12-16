/**
 * Integration tests for Chapman mechanism
 * Mirrors python/test/integration/test_chapman.py
 * Uses Node.js built-in test runner
 */

const { describe, it } = require('node:test');
const assert = require('node:assert');
const path = require('path');
const musica = require('musica-addon');
const { MICM, SolverType } = musica.micmSolver;
const { isClose } = require('../util/testUtils');

/**
 * Test solver with Chapman mechanism
 * Equivalent to TestSolve in Python
 */
function testSolve(solver) {
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
        O2: 0.75,
        O: 0.0,
        O1D: 0.0,
        O3: 0.0000081
    };

    // Test setting int values (from Python test)
    state.setConditions({ temperatures: 272, pressures: 101325 });

    // Set actual test conditions
    state.setConditions({ temperatures: temperature, pressures: pressure });
    state.setConcentrations(initialConcentrations);
    state.setUserDefinedRateParameters(rateConstants);

    // Solve for one time step
    solver.solve(state, timeStep);
    const concentrations = state.getConcentrations();

    // Verify results match Python test expectations
    assert.ok(
        isClose(concentrations.O2[0], 0.75, 1e-5),
        `O2 concentration should be approximately 0.75, got ${concentrations.O2[0]}`
    );
    assert.ok(
        concentrations.O[0] > 0.0,
        `O concentration should be greater than 0, got ${concentrations.O[0]}`
    );
    assert.ok(
        concentrations.O1D[0] > 0.0,
        `O1D concentration should be greater than 0, got ${concentrations.O1D[0]}`
    );
    assert.ok(
        concentrations.O3[0] !== 0.0000081,
        `O3 concentration should have changed from initial value, got ${concentrations.O3[0]}`
    );
}

describe('Chapman mechanism with v0 config path', () => {
    it('should solve with v0 config directory', () => {
        const configPath = path.join(__dirname, '../../../configs/v0/chapman');
        const solver = MICM.fromConfigPath(
            configPath,
            SolverType.rosenbrock_standard_order
        );
        testSolve(solver);
    });
});

describe('Chapman mechanism with v1 config files', () => {
    it('should solve with v1 JSON config file', () => {
        const configPath = path.join(__dirname, '../../../configs/v1/chapman/config.json');
        const solver = MICM.fromConfigPath(
            configPath,
            SolverType.rosenbrock_standard_order
        );
        testSolve(solver);
    });

    it('should solve with v1 YAML config file', () => {
        const configPath = path.join(__dirname, '../../../configs/v1/chapman/config.yaml');
        const solver = MICM.fromConfigPath(
            configPath,
            SolverType.rosenbrock_standard_order
        );
        testSolve(solver);
    });
});
