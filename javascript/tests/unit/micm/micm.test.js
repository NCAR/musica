/**
 * Unit tests for the MICM class
 * Mirrors python/test/unit/micm/test_micm.py
 * Uses Node.js built-in test runner
 */

const { describe, it } = require('node:test');
const assert = require('node:assert');
const path = require('path');
const { MICM } = require('../../../micm/micm');
const { State } = require('../../../micm/state');
const { SolverType } = require('../../../micm/solver');
const { SolverResult, SolverStats } = require('../../../micm/solver_result');

/**
 * Helper to get the config path for testing
 */
function getConfigPath() {
  return path.join(__dirname, '../../../../configs/v0/analytical');
}

describe('MICM Initialization', () => {
  it('should initialize with config_path', () => {
    const micm = new MICM({ config_path: getConfigPath() });
    assert.ok(micm, 'MICM should be created');
    assert.ok(micm.solverType() !== null, 'Solver type should be set');
  });

  it('should initialize with config_path and solver_type', () => {
    const micm = new MICM({
      config_path: getConfigPath(),
      solver_type: SolverType.rosenbrock_standard_order
    });
    assert.ok(micm, 'MICM should be created');
    assert.strictEqual(
      micm.solverType(),
      SolverType.rosenbrock_standard_order,
      'Solver type should match'
    );
  });

  it('should throw error without config_path', () => {
    assert.throws(
      () => new MICM(),
      /config_path must be provided/,
      'Should throw error when config_path is missing'
    );
  });

  it('should use default solver type', () => {
    const micm = new MICM({ config_path: getConfigPath() });
    assert.strictEqual(
      micm.solverType(),
      SolverType.rosenbrock_standard_order,
      'Default solver type should be rosenbrock_standard_order'
    );
  });

  it('should initialize with backward_euler_standard_order', () => {
    const micm = new MICM({
      config_path: getConfigPath(),
      solver_type: SolverType.backward_euler_standard_order
    });
    assert.strictEqual(
      micm.solverType(),
      SolverType.backward_euler_standard_order,
      'Solver type should be backward_euler_standard_order'
    );
  });
});

describe('MICM solverType method', () => {
  it('should return correct solver type', () => {
    const micm = new MICM({
      config_path: getConfigPath(),
      solver_type: SolverType.rosenbrock_standard_order
    });
    assert.strictEqual(
      micm.solverType(),
      SolverType.rosenbrock_standard_order,
      'Should return rosenbrock_standard_order'
    );
  });

  it('should work with different solver types', () => {
    const solverTypes = [
      SolverType.rosenbrock_standard_order,
      SolverType.backward_euler_standard_order,
    ];

    for (const solverType of solverTypes) {
      const micm = new MICM({
        config_path: getConfigPath(),
        solver_type: solverType
      });
      assert.strictEqual(
        micm.solverType(),
        solverType,
        `Solver type should match ${solverType}`
      );
    }
  });
});

describe('MICM createState method', () => {
  let micm;

  it('should create state with default single grid cell', () => {
    micm = new MICM({ config_path: getConfigPath() });
    const state = micm.createState();
    assert.ok(state instanceof State, 'Should create State instance');
  });

  it('should create state with explicit single grid cell', () => {
    micm = new MICM({ config_path: getConfigPath() });
    const state = micm.createState(1);
    assert.ok(state instanceof State, 'Should create State instance');
  });

  it('should create state with multiple grid cells', () => {
    micm = new MICM({ config_path: getConfigPath() });
    const numCells = [2, 5, 10, 100];

    for (const num of numCells) {
      const state = micm.createState(num);
      assert.ok(state instanceof State, `Should create State with ${num} cells`);
    }
  });

  it('should throw error for zero grid cells', () => {
    micm = new MICM({ config_path: getConfigPath() });
    assert.throws(
      () => micm.createState(0),
      /number_of_grid_cells must be greater than 0/,
      'Should throw error for 0 grid cells'
    );
  });

  it('should throw error for negative grid cells', () => {
    micm = new MICM({ config_path: getConfigPath() });
    assert.throws(
      () => micm.createState(-1),
      /number_of_grid_cells must be greater than 0/,
      'Should throw error for negative grid cells'
    );
  });

  it('should create multiple independent states', () => {
    micm = new MICM({ config_path: getConfigPath() });
    const state1 = micm.createState(1);
    const state2 = micm.createState(5);

    assert.ok(state1 instanceof State, 'State1 should be State instance');
    assert.ok(state2 instanceof State, 'State2 should be State instance');
    assert.notStrictEqual(state1, state2, 'States should be independent');
  });
});

describe('MICM solve method', () => {
  let micm;

  it('should solve with valid state and timestep', () => {
    micm = new MICM({ config_path: getConfigPath() });
    const state = micm.createState(1);

    // Set initial conditions
    state.setConditions({
      temperatures: 298.15,
      pressures: 101325.0,
      air_densities: 1.2
    });
    state.setConcentrations({ A: 1.0, B: 0.0, C: 0.5 });
    state.setUserDefinedRateParameters({
      'USER.reaction 1': 0.001,
      'USER.reaction 2': 0.002
    });

    // Solve
    const result = micm.solve(state, 1.0);

    assert.ok(result instanceof SolverResult, 'Should return SolverResult');
    assert.ok(result.stats instanceof SolverStats, 'Should have SolverStats');
  });

  it('should solve with float timestep', () => {
    micm = new MICM({ config_path: getConfigPath() });
    const state = micm.createState(1);

    state.setConditions({
      temperatures: 298.15,
      pressures: 101325.0,
      air_densities: 1.2
    });
    state.setConcentrations({ A: 1.0 });
    state.setUserDefinedRateParameters({
      'USER.reaction 1': 0.001,
      'USER.reaction 2': 0.002
    });

    const result = micm.solve(state, 1.5);
    assert.ok(result instanceof SolverResult, 'Should solve with float timestep');
  });

  it('should solve with integer timestep', () => {
    micm = new MICM({ config_path: getConfigPath() });
    const state = micm.createState(1);

    state.setConditions({
      temperatures: 298.15,
      pressures: 101325.0,
      air_densities: 1.2
    });
    state.setConcentrations({ A: 1.0 });
    state.setUserDefinedRateParameters({
      'USER.reaction 1': 0.001,
      'USER.reaction 2': 0.002
    });

    const result = micm.solve(state, 1);
    assert.ok(result instanceof SolverResult, 'Should solve with integer timestep');
  });

  it('should throw error for invalid state type', () => {
    micm = new MICM({ config_path: getConfigPath() });

    assert.throws(
      () => micm.solve('not a state', 1.0),
      /state must be an instance of State/,
      'Should throw TypeError for invalid state'
    );
  });

  it('should throw error for invalid timestep type', () => {
    micm = new MICM({ config_path: getConfigPath() });
    const state = micm.createState(1);

    assert.throws(
      () => micm.solve(state, 'not a number'),
      /timeStep must be a number/,
      'Should throw TypeError for invalid timestep'
    );
  });

  it('should throw error for null state', () => {
    micm = new MICM({ config_path: getConfigPath() });

    assert.throws(
      () => micm.solve(null, 1.0),
      /state must be an instance of State/,
      'Should throw TypeError for null state'
    );
  });

  it('should throw error for undefined state', () => {
    micm = new MICM({ config_path: getConfigPath() });

    assert.throws(
      () => micm.solve(undefined, 1.0),
      /state must be an instance of State/,
      'Should throw TypeError for undefined state'
    );
  });

  it('should solve multiple times with same state', () => {
    micm = new MICM({ config_path: getConfigPath() });
    const state = micm.createState(1);

    state.setConditions({
      temperatures: 298.15,
      pressures: 101325.0,
      air_densities: 1.2
    });
    state.setConcentrations({ A: 1.0, B: 0.0 });
    state.setUserDefinedRateParameters({
      'USER.reaction 1': 0.001,
      'USER.reaction 2': 0.002
    });

    // Solve multiple times
    for (let i = 0; i < 5; i++) {
      const result = micm.solve(state, 1.0);
      assert.ok(
        result instanceof SolverResult,
        `Should solve on iteration ${i + 1}`
      );
    }
  });
});

describe('MICM Integration Tests', () => {
  it('should complete end-to-end workflow', () => {
    // Initialize solver
    const micm = new MICM({
      config_path: getConfigPath(),
      solver_type: SolverType.rosenbrock_standard_order
    });

    // Create state
    const state = micm.createState(1);

    // Set conditions
    state.setConditions({
      temperatures: 298.15,
      pressures: 101325.0,
      air_densities: 1.2
    });

    state.setConcentrations({
      A: 0.75,
      B: 0.0,
      C: 0.4,
      D: 0.8,
      E: 0.0,
      F: 0.1
    });

    state.setUserDefinedRateParameters({
      'USER.reaction 1': 0.001,
      'USER.reaction 2': 0.002
    });

    // Solve
    const result = micm.solve(state, 1.0);

    // Verify results
    assert.ok(result instanceof SolverResult, 'Should return SolverResult');
    assert.ok(result.stats instanceof SolverStats, 'Should have SolverStats');

    // Get updated concentrations
    const concentrations = state.getConcentrations();
    assert.ok('A' in concentrations, 'Should have species A');
    assert.ok('B' in concentrations, 'Should have species B');
    assert.ok('C' in concentrations, 'Should have species C');
  });

  it('should handle multiple solve iterations', () => {
    const micm = new MICM({ config_path: getConfigPath() });
    const state = micm.createState(1);

    state.setConditions({
      temperatures: 298.15,
      pressures: 101325.0,
      air_densities: 1.2
    });

    state.setConcentrations({
      A: 1.0,
      B: 0.0,
      C: 0.5
    });

    state.setUserDefinedRateParameters({
      'USER.reaction 1': 0.001,
      'USER.reaction 2': 0.002
    });

    const numIterations = 10;
    for (let i = 0; i < numIterations; i++) {
      const result = micm.solve(state, 0.5);
      assert.ok(
        result instanceof SolverResult,
        `Iteration ${i + 1} should succeed`
      );
      assert.ok(
        typeof result.stats.final_time === 'number',
        'Should have final_time'
      );
    }

    // Verify concentrations changed
    const finalConcentrations = state.getConcentrations();
    assert.ok(finalConcentrations.A[0] !== 1.0, 'Concentration A should have changed');
  });

  it('should work with different solver types', () => {
    const solverTypes = [
      SolverType.rosenbrock_standard_order,
      SolverType.backward_euler_standard_order,
    ];

    for (const solverType of solverTypes) {
      const micm = new MICM({
        config_path: getConfigPath(),
        solver_type: solverType
      });

      const state = micm.createState(1);

      state.setConditions({
        temperatures: 298.15,
        pressures: 101325.0,
        air_densities: 1.2
      });

      state.setConcentrations({ A: 1.0, B: 0.0 });
      state.setUserDefinedRateParameters({
        'USER.reaction 1': 0.001,
        'USER.reaction 2': 0.002
      });

      const result = micm.solve(state, 1.0);
      assert.ok(
        result instanceof SolverResult,
        `Should solve with solver type ${solverType}`
      );
    }
  });
});

describe('MICM SolverResult validation', () => {
  it('should return valid SolverResult structure', () => {
    const micm = new MICM({ config_path: getConfigPath() });
    const state = micm.createState(1);

    state.setConditions({
      temperatures: 298.15,
      pressures: 101325.0,
      air_densities: 1.2
    });

    state.setConcentrations({ A: 1.0 });
    state.setUserDefinedRateParameters({
      'USER.reaction 1': 0.001,
      'USER.reaction 2': 0.002
    });

    const result = micm.solve(state, 1.0);

    // Validate result structure
    assert.ok(result instanceof SolverResult, 'Should be SolverResult');
    assert.ok(typeof result.state === 'number', 'Should have state property');
    assert.ok(result.stats instanceof SolverStats, 'Should have stats property');

    // Validate stats structure
    const stats = result.stats;
    assert.ok(typeof stats.function_calls === 'number', 'Should have function_calls');
    assert.ok(typeof stats.jacobian_updates === 'number', 'Should have jacobian_updates');
    assert.ok(typeof stats.number_of_steps === 'number', 'Should have number_of_steps');
    assert.ok(typeof stats.accepted === 'number', 'Should have accepted');
    assert.ok(typeof stats.rejected === 'number', 'Should have rejected');
    assert.ok(typeof stats.decompositions === 'number', 'Should have decompositions');
    assert.ok(typeof stats.solves === 'number', 'Should have solves');
    assert.ok(typeof stats.final_time === 'number', 'Should have final_time');
  });
});
