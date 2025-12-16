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
const musica = require('musica-addon');
const { types, reactionTypes, Mechanism } = musica.mechanismConfiguration;
const { Species, Phase, ReactionComponent } = types;

/**
 * Helper to get the config path for testing
 */
function getConfigPath() {
  return path.join(__dirname, '../../../../configs/v0/analytical');
}

describe('MICM Initialization', () => {
  it('should initialize with fromConfigPath', () => {
    const micm = MICM.fromConfigPath(getConfigPath());
    assert.ok(micm, 'MICM should be created');
    assert.ok(micm.solverType() !== null, 'Solver type should be set');
  });

  it('should initialize with fromConfigPath and solver_type', () => {
    const micm = MICM.fromConfigPath(
      getConfigPath(),
      SolverType.rosenbrock_standard_order
    );
    assert.ok(micm, 'MICM should be created');
    assert.strictEqual(
      micm.solverType(),
      SolverType.rosenbrock_standard_order,
      'Solver type should match'
    );
  });

  it('should throw error with invalid config_path type', () => {
    assert.throws(
      () => MICM.fromConfigPath(123),
      /configPath must be a string/,
      'Should throw error when configPath is not a string'
    );
  });

  it('should use default solver type', () => {
    const micm = MICM.fromConfigPath(getConfigPath());
    assert.strictEqual(
      micm.solverType(),
      SolverType.rosenbrock_standard_order,
      'Default solver type should be rosenbrock_standard_order'
    );
  });

  it('should initialize with backward_euler_standard_order', () => {
    const micm = MICM.fromConfigPath(
      getConfigPath(),
      SolverType.backward_euler_standard_order
    );
    assert.strictEqual(
      micm.solverType(),
      SolverType.backward_euler_standard_order,
      'Solver type should be backward_euler_standard_order'
    );
  });

  it('should initialize with fromMechanism', () => {
    // Create a simple mechanism for testing
    const A = new Species({ name: 'A' });
    const B = new Species({ name: 'B' });
    const C = new Species({ name: 'C' });
    
    const gas = new Phase({
      name: 'gas',
      species: [A, B, C]
    });

    const reaction1 = new reactionTypes.UserDefined({
      name: 'reaction 1',
      gas_phase: 'gas',
      reactants: [new ReactionComponent({ species_name: 'A' })],
      products: [new ReactionComponent({ species_name: 'B' })]
    });

    const reaction2 = new reactionTypes.UserDefined({
      name: 'reaction 2',
      gas_phase: 'gas',
      reactants: [new ReactionComponent({ species_name: 'B' })],
      products: [new ReactionComponent({ species_name: 'C' })]
    });

    const mechanism = new Mechanism({
      name: 'Test Mechanism',
      version: '1.0.0',
      species: [A, B, C],
      phases: [gas],
      reactions: [reaction1, reaction2]
    });

    const micm = MICM.fromMechanism(mechanism);
    assert.ok(micm, 'MICM should be created from mechanism');
    assert.ok(micm.solverType() !== null, 'Solver type should be set');
  });

  it('should throw error with invalid mechanism', () => {
    assert.throws(
      () => MICM.fromMechanism({}),
      /mechanism must be a valid Mechanism object with getJSON\(\) method/,
      'Should throw error when mechanism is invalid'
    );
  });

  it('should throw error with null mechanism', () => {
    assert.throws(
      () => MICM.fromMechanism(null),
      /mechanism must be a valid Mechanism object with getJSON\(\) method/,
      'Should throw error when mechanism is null'
    );
  });
});

describe('MICM solverType method', () => {
  it('should return correct solver type', () => {
    const micm = MICM.fromConfigPath(
      getConfigPath(),
      SolverType.rosenbrock_standard_order
    );
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
      const micm = MICM.fromConfigPath(getConfigPath(), solverType);
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
    micm = MICM.fromConfigPath(getConfigPath());
    const state = micm.createState();
    assert.ok(state instanceof State, 'Should create State instance');
  });

  it('should create state with explicit single grid cell', () => {
    micm = MICM.fromConfigPath(getConfigPath());
    const state = micm.createState(1);
    assert.ok(state instanceof State, 'Should create State instance');
  });

  it('should create state with multiple grid cells', () => {
    micm = MICM.fromConfigPath(getConfigPath());
    const numCells = [2, 5, 10, 100];

    for (const num of numCells) {
      const state = micm.createState(num);
      assert.ok(state instanceof State, `Should create State with ${num} cells`);
    }
  });

  it('should throw error for zero grid cells', () => {
    micm = MICM.fromConfigPath(getConfigPath());
    assert.throws(
      () => micm.createState(0),
      /number_of_grid_cells must be greater than 0/,
      'Should throw error for 0 grid cells'
    );
  });

  it('should throw error for negative grid cells', () => {
    micm = MICM.fromConfigPath(getConfigPath());
    assert.throws(
      () => micm.createState(-1),
      /number_of_grid_cells must be greater than 0/,
      'Should throw error for negative grid cells'
    );
  });

  it('should create multiple independent states', () => {
    micm = MICM.fromConfigPath(getConfigPath());
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
    micm = MICM.fromConfigPath(getConfigPath());
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
    micm = MICM.fromConfigPath(getConfigPath());
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
    micm = MICM.fromConfigPath(getConfigPath());
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
    micm = MICM.fromConfigPath(getConfigPath());

    assert.throws(
      () => micm.solve('not a state', 1.0),
      /state must be an instance of State/,
      'Should throw TypeError for invalid state'
    );
  });

  it('should throw error for invalid timestep type', () => {
    micm = MICM.fromConfigPath(getConfigPath());
    const state = micm.createState(1);

    assert.throws(
      () => micm.solve(state, 'not a number'),
      /timeStep must be a number/,
      'Should throw TypeError for invalid timestep'
    );
  });

  it('should throw error for null state', () => {
    micm = MICM.fromConfigPath(getConfigPath());

    assert.throws(
      () => micm.solve(null, 1.0),
      /state must be an instance of State/,
      'Should throw TypeError for null state'
    );
  });

  it('should throw error for undefined state', () => {
    micm = MICM.fromConfigPath(getConfigPath());

    assert.throws(
      () => micm.solve(undefined, 1.0),
      /state must be an instance of State/,
      'Should throw TypeError for undefined state'
    );
  });

  it('should solve multiple times with same state', () => {
    micm = MICM.fromConfigPath(getConfigPath());
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
    const micm = MICM.fromConfigPath(
      getConfigPath(),
      SolverType.rosenbrock_standard_order
    );

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
    const micm = MICM.fromConfigPath(getConfigPath());
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
      const micm = MICM.fromConfigPath(getConfigPath(), solverType);

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
    const micm = MICM.fromConfigPath(getConfigPath());
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
