/**
 * Unit tests for the State class
 * Mirrors python/test/unit/micm/test_state.py
 * Uses Node.js built-in test runner
 */

const { describe, it, before } = require('node:test');
const assert = require('node:assert');
const path = require('path');
const musica = require('musica-addon');
const { MICM } = musica.micmSolver;
const { isClose } = require('../../util/testUtils');

/**
 * Helper function to create a test mechanism
 * This creates a simple mechanism for testing state operations
 */
function createTestMechanism() {
    // For JavaScript, we'll use a config path instead of mechanism object
    // as the mechanism configuration API might not be fully exposed
    return path.join(__dirname, '../../../../configs/v0/analytical');
}

describe('State initialization', () => {
    it('should create state with single grid cell', () => {
        const configPath = createTestMechanism();
        const solver = MICM.fromConfigPath(configPath);
        const state = solver.createState(1);
        assert.ok(state, 'State should be created');
    });

    it('should create state with multiple grid cells', () => {
        const configPath = createTestMechanism();
        const solver = MICM.fromConfigPath(configPath);
        const state = solver.createState(3);
        assert.ok(state, 'State with 3 grid cells should be created');
    });

    it('should throw error for invalid grid cell count', () => {
        const configPath = createTestMechanism();
        const solver = MICM.fromConfigPath(configPath);
        assert.throws(
            () => solver.createState(0),
            /number_of_grid_cells must be greater than 0/,
            'Should throw error for 0 grid cells'
        );
    });
});

describe('Concentrations', () => {
    let solver;
    let state;

    before(() => {
        const configPath = createTestMechanism();
        solver = MICM.fromConfigPath(configPath);
    });

    it('should set and get concentrations for single grid cell', () => {
        state = solver.createState(1);
        const concentrations = { A: 1.0, B: 2.0, C: 3.0 };
        state.setConcentrations(concentrations);
        const result = state.getConcentrations();

        assert.ok(isClose(result.A[0], 1.0, 1e-13), 'A concentration should be 1.0');
        assert.ok(isClose(result.B[0], 2.0, 1e-13), 'B concentration should be 2.0');
        assert.ok(isClose(result.C[0], 3.0, 1e-13), 'C concentration should be 3.0');
    });

    it('should set and get concentrations for multiple grid cells', () => {
        state = solver.createState(2);
        const concentrations = { A: [1.0, 2.0], B: [3.0, 4.0], C: [5.0, 6.0] };
        state.setConcentrations(concentrations);
        const result = state.getConcentrations();

        assert.deepStrictEqual(result.A, [1.0, 2.0], 'A concentrations should match');
        assert.deepStrictEqual(result.B, [3.0, 4.0], 'B concentrations should match');
        assert.deepStrictEqual(result.C, [5.0, 6.0], 'C concentrations should match');
    });

    it('should handle empty concentration update', () => {
        state = solver.createState(1);
        const concentrations = { A: 1.0, B: 2.0, C: 3.0 };
        state.setConcentrations(concentrations);

        // Set empty concentrations - should not change values
        state.setConcentrations({});
        const result = state.getConcentrations();

        assert.ok(isClose(result.A[0], 1.0, 1e-13), 'A concentration should remain 1.0');
        assert.ok(isClose(result.B[0], 2.0, 1e-13), 'B concentration should remain 2.0');
        assert.ok(isClose(result.C[0], 3.0, 1e-13), 'C concentration should remain 3.0');
    });
});

describe('Conditions', () => {
    let solver;
    let state;

    before(() => {
        const configPath = createTestMechanism();
        solver = MICM.fromConfigPath(configPath);
    });

    it('should set and get conditions for single grid cell', () => {
        state = solver.createState(1);
        state.setConditions({ temperatures: 300.0, pressures: 101325.0 });
        const conditions = state.getConditions();

        assert.ok(isClose(conditions.temperature[0], 300.0, 1e-13), 'Temperature should be 300.0');
        assert.ok(isClose(conditions.pressure[0], 101325.0, 1e-13), 'Pressure should be 101325.0');
        // Air density should be calculated: P / (R * T) where R = 8.31446261815324
        const expectedAirDensity = 101325.0 / (8.31446261815324 * 300.0);
        assert.ok(isClose(conditions.air_density[0], expectedAirDensity, 0.1), 'Air density should be calculated');
    });

    it('should set and get conditions for multiple grid cells', () => {
        state = solver.createState(2);
        state.setConditions({
            temperatures: [300.0, 310.0],
            pressures: [101325.0, 101325.0],
            air_densities: [40.9, 39.5]
        });
        const conditions = state.getConditions();

        assert.deepStrictEqual(conditions.temperature, [300.0, 310.0], 'Temperatures should match');
        assert.deepStrictEqual(conditions.pressure, [101325.0, 101325.0], 'Pressures should match');
        assert.deepStrictEqual(conditions.air_density, [40.9, 39.5], 'Air densities should match');
    });

    it('should accept integer values for conditions', () => {
        state = solver.createState(1);
        // Test setting int values (from Python test)
        state.setConditions({ temperatures: 272, pressures: 101325 });
        const conditions = state.getConditions();

        assert.ok(isClose(conditions.temperature[0], 272, 1e-13), 'Temperature should be 272');
        assert.ok(isClose(conditions.pressure[0], 101325, 1e-13), 'Pressure should be 101325');
    });
});

describe('User-defined rate parameters', () => {
    let solver;
    let state;

    before(() => {
        const configPath = createTestMechanism();
        solver = MICM.fromConfigPath(configPath);
    });

    it('should set and get user-defined rate parameters for single grid cell', () => {
        state = solver.createState(1);
        const params = { 'USER.reaction 1': 1.0 };
        state.setUserDefinedRateParameters(params);
        const result = state.getUserDefinedRateParameters();

        assert.ok(isClose(result['USER.reaction 1'][0], 1.0, 1e-13), 'Rate parameter should be 1.0');
    });

    it('should set and get user-defined rate parameters for multiple grid cells', () => {
        state = solver.createState(2);
        const params = { 'USER.reaction 1': [1.0, 2.0] };
        state.setUserDefinedRateParameters(params);
        const result = state.getUserDefinedRateParameters();

        assert.deepStrictEqual(result['USER.reaction 1'], [1.0, 2.0], 'Rate parameters should match');
    });

    it('should handle empty rate parameter update', () => {
        state = solver.createState(1);
        const params = { 'USER.reaction 1': 1.0 };
        state.setUserDefinedRateParameters(params);

        // Set empty parameters - should not change values
        state.setUserDefinedRateParameters({});
        const result = state.getUserDefinedRateParameters();

        assert.ok(isClose(result['USER.reaction 1'][0], 1.0, 1e-13), 'Rate parameter should remain 1.0');
    });
});

describe('State ordering', () => {
    let solver;
    let state;

    before(() => {
        const configPath = createTestMechanism();
        solver = MICM.fromConfigPath(configPath);
        state = solver.createState(1);
    });

    it('should get species ordering', () => {
        const ordering = state.getSpeciesOrdering();

        assert.ok(typeof ordering === 'object', 'Ordering should be an object');
        assert.ok('A' in ordering, 'Should have species A');
        assert.ok('B' in ordering, 'Should have species B');
        assert.ok('C' in ordering, 'Should have species C');
        assert.ok(ordering.A >= 0, 'A ordering should be non-negative');
        assert.ok(ordering.B >= 0, 'B ordering should be non-negative');
        assert.ok(ordering.C >= 0, 'C ordering should be non-negative');
    });

    it('should get user-defined rate parameters ordering', () => {
        const ordering = state.getUserDefinedRateParametersOrdering();

        assert.ok(typeof ordering === 'object', 'Ordering should be an object');
        assert.ok('USER.reaction 1' in ordering, 'Should have USER.reaction 1');
        assert.ok('USER.reaction 2' in ordering, 'Should have USER.reaction 2');
        assert.ok(ordering['USER.reaction 1'] >= 0, 'Ordering should be non-negative');
        assert.ok(ordering['USER.reaction 2'] >= 0, 'Ordering should be non-negative');
    });
});

describe('Grid cell operations', () => {
    it('should return correct number of grid cells', () => {
        const configPath = createTestMechanism();
        const solver = MICM.fromConfigPath(configPath);

        const state1 = solver.createState(1);
        assert.strictEqual(state1.getNumberOfGridCells(), 1, 'Should have 1 grid cell');

        const state5 = solver.createState(5);
        assert.strictEqual(state5.getNumberOfGridCells(), 5, 'Should have 5 grid cells');
    });
});
