/**
 * Unit tests for the MICM class
 * Mirrors python/test/unit/micm/test_micm.py
 * Uses Node.js built-in test runner
 */

import { describe, it, before } from 'node:test';
import assert from 'node:assert';
import path from 'path';
import * as musica from '../../index.js';
import { fileURLToPath } from 'url';

const { MICM, SolverType, State, SolverResult, SolverStats } = musica;
const { types, reactionTypes, Mechanism } = musica.mechanismConfiguration;
const { Species, Phase, ReactionComponent } = types;

// Convert import.meta.url to a file path
const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

before(async () => {
  await musica.initModule();
});

/**
 * Helper to get the config path for testing
 */
function getConfigPath() {
  return path.join(__dirname, '../../../configs/v0/analytical');
}

function createTestMechanism() {
  // Create a simple mechanism for testing
  const A = new Species({ name: 'A' });
  const B = new Species({ name: 'B' });
  const C = new Species({ name: 'C' });

  const gas = new Phase({
    name: 'gas',
    species: [A, B, C],
  });

  const reaction1 = new reactionTypes.UserDefined({
    name: 'reaction 1',
    gas_phase: 'gas',
    reactants: [new ReactionComponent({ species_name: 'A' })],
    products: [new ReactionComponent({ species_name: 'B' })],
  });

  const reaction2 = new reactionTypes.UserDefined({
    name: 'reaction 2',
    gas_phase: 'gas',
    reactants: [new ReactionComponent({ species_name: 'B' })],
    products: [new ReactionComponent({ species_name: 'C' })],
  });

  const mechanism = new Mechanism({
    name: 'Test Mechanism',
    version: '1.0.0',
    species: [A, B, C],
    phases: [gas],
    reactions: [reaction1, reaction2],
  });
  return mechanism;
}

describe('MICM Initialization', () => {
  it('should initialize with fromConfigPath and solver_type', async (t) => {
    const types = Object.values(SolverType);
    for (const solverType of types) {
      const micm = MICM.fromConfigPath(getConfigPath(), solverType);
      assert.ok(micm, 'MICM should be created');
      assert.strictEqual(micm.solverType(), solverType, `Solver type should match ${solverType}`);
      micm.delete();
    }
  });

  it('should throw error with invalid config_path type', async (t) => {
    assert.throws(
      () => MICM.fromConfigPath(123),
      /configPath must be a string/,
      'Should throw error when configPath is not a string'
    );
  });

  it('should initialize and solve with fromMechanism', async (t) => {
    const types = Object.values(SolverType);
    const mechanism = createTestMechanism();
    for (const solverType of types) {
      const micm = MICM.fromMechanism(mechanism, solverType);
      assert.ok(micm, 'MICM should be created from mechanism');
      assert.strictEqual(micm.solverType(), solverType, `Solver type should match ${solverType}`);

      const state = micm.createState(1);
      state.setConcentrations({ A: [1.0], B: [2.0], C: [3.0] });
      state.setConditions({ temperatures: [298.15], pressures: [101325.0], air_densities: [1.0] });
      state.setUserDefinedRateParameters({
        'USER.reaction 1': 0.001,
        'USER.reaction 2': 0.002,
      });

      const result = micm.solve(state, 60.0);
      assert.ok(result, 'MICM.solve should return a result');
      let concentrations = state.getConcentrations();

      assert.ok(concentrations.A !== 1.0, 'Concentration of A should have changed after solve');
      assert.ok(concentrations.B !== 2.0, 'Concentration of B should have changed after solve');
      assert.ok(concentrations.C !== 3.0, 'Concentration of C should have changed after solve');

      micm.delete();
      state.delete();
    }
  });

  it('should throw error with invalid mechanism', async (t) => {
    assert.throws(
      () => MICM.fromMechanism({}),
      /mechanism must be a valid Mechanism object with getJSON\(\) method/,
      'Should throw error when mechanism is invalid'
    );
  });

  it('should throw error with null mechanism', async (t) => {
    assert.throws(
      () => MICM.fromMechanism(null),
      /mechanism must be a valid Mechanism object with getJSON\(\) method/,
      'Should throw error when mechanism is null'
    );
  });
});

describe('MICM createState method', () => {
  let micm;

  it('should create state with default single grid cell', async (t) => {
    micm = MICM.fromConfigPath(getConfigPath());
    const state = micm.createState();
    assert.ok(state instanceof State, 'Should create State instance');
    state.delete();
    micm.delete();
  });

  it('should create state with explicit single grid cell', async (t) => {
    micm = MICM.fromConfigPath(getConfigPath());
    const state = micm.createState(1);
    assert.ok(state instanceof State, 'Should create State instance');
    state.delete();
    micm.delete();
  });

  it('should create state with multiple grid cells', async (t) => {
    micm = MICM.fromConfigPath(getConfigPath());
    const numCells = [2, 5, 10, 100];

    for (const num of numCells) {
      const state = micm.createState(num);
      assert.ok(state instanceof State, `Should create State with ${num} cells`);
      state.delete();
    }
    micm.delete();
  });

  it('should throw error for zero grid cells', async (t) => {
    micm = MICM.fromConfigPath(getConfigPath());
    assert.throws(
      () => micm.createState(0),
      /number_of_grid_cells must be greater than 0/,
      'Should throw error for 0 grid cells'
    );
    micm.delete();
  });

  it('should throw error for negative grid cells', async (t) => {
    micm = MICM.fromConfigPath(getConfigPath());
    assert.throws(
      () => micm.createState(-1),
      /number_of_grid_cells must be greater than 0/,
      'Should throw error for negative grid cells'
    );
    micm.delete();
  });

  it('should create multiple independent states', async (t) => {
    micm = MICM.fromConfigPath(getConfigPath());
    const state1 = micm.createState(1);
    const state2 = micm.createState(5);

    assert.ok(state1 instanceof State, 'State1 should be State instance');
    assert.ok(state2 instanceof State, 'State2 should be State instance');
    assert.notStrictEqual(state1, state2, 'States should be independent');
    state1.delete();
    state2.delete();
    micm.delete();
  });
});

describe('MICM Solve - comprehensive', () => {
  const gridCellsList = [1, 5, 10, 20];
  const solverTypes = Object.values(SolverType);

  let mechanism;
  before(() => {
    mechanism = createTestMechanism();
  });

  it('should solve from a mechanism in code', async (t) => {
    for (const solverType of solverTypes) {
      const micm = MICM.fromMechanism(mechanism, solverType);

      for (const nCells of gridCellsList) {
        const state = micm.createState(nCells);

        // Set initial concentrations and conditions
        const conc = {
          A: Array(nCells).fill(1.0),
          B: Array(nCells).fill(0.0),
          C: Array(nCells).fill(0.5),
        };
        state.setConcentrations(conc);
        state.setConditions({
          temperatures: Array(nCells).fill(298.15),
          pressures: Array(nCells).fill(101325.0),
          airDensities: Array(nCells).fill(1.0),
        });
        state.setUserDefinedRateParameters({
          'USER.reaction 1': Array(nCells).fill(0.001),
          'USER.reaction 2': Array(nCells).fill(0.002),
        });

        const result = micm.solve(state, 1.0);
        assert.ok(result instanceof SolverResult, 'Should return SolverResult');
        assert.ok(result.stats instanceof SolverStats, 'Should return SolverStats');

        const updated = state.getConcentrations();
        for (let i = 0; i < nCells; i++) {
          assert.ok(updated.A[i] !== 1.0, `A[${i}] should have changed`);
          assert.ok(updated.B[i] !== 0.0, `B[${i}] should have changed`);
          assert.ok(updated.C[i] !== 0.5, `C[${i}] should have changed`);
        }
        state.delete();
      }
      micm.delete();
    }
  });

  it('should solve from a config file', async (t) => {
    for (const solverType of solverTypes) {
      const micm = MICM.fromConfigPath(getConfigPath(), solverType);

      for (const nCells of gridCellsList) {
        const state = micm.createState(nCells);

        const conc = {
          A: Array(nCells).fill(0.5),
          B: Array(nCells).fill(1.0),
          C: Array(nCells).fill(0.0),
        };
        state.setConcentrations(conc);
        state.setConditions({
          temperatures: Array(nCells).fill(300),
          pressures: Array(nCells).fill(101325),
          airDensities: Array(nCells).fill(1.2),
        });
        state.setUserDefinedRateParameters({
          'USER.reaction 1': Array(nCells).fill(0.002),
          'USER.reaction 2': Array(nCells).fill(0.004),
        });

        const result = micm.solve(state, 2.0);
        assert.ok(result instanceof SolverResult, 'Should return SolverResult');
        assert.ok(result.stats instanceof SolverStats, 'Should have stats');

        const updated = state.getConcentrations();
        for (let i = 0; i < nCells; i++) {
          assert.ok(updated.A[i] !== 0.5, `A[${i}] should have changed`);
          assert.ok(updated.B[i] !== 1.0, `B[${i}] should have changed`);
          assert.ok(updated.C[i] !== 0.0, `C[${i}] should have changed`);
        }
        state.delete();
      }
      micm.delete();
    }
  });

  it('should handle multiple solve iterations', async (t) => {
    const micm = MICM.fromMechanism(mechanism, solverTypes[0]);
    const state = micm.createState(5);

    state.setConcentrations({
      A: Array(5).fill(1.0),
      B: Array(5).fill(0.0),
      C: Array(5).fill(0.5),
    });
    state.setConditions({
      temperatures: Array(5).fill(298.15),
      pressures: Array(5).fill(101325.0),
      airDensities: Array(5).fill(1.0),
    });
    state.setUserDefinedRateParameters({
      'USER.reaction 1': Array(5).fill(0.001),
      'USER.reaction 2': Array(5).fill(0.002),
    });

    for (let i = 0; i < 10; i++) {
      const result = micm.solve(state, 0.5);
      assert.ok(result instanceof SolverResult, `Iteration ${i + 1} should succeed`);
      assert.ok(result.stats instanceof SolverStats, 'Should have stats');
    }

    const final = state.getConcentrations();
    for (let i = 0; i < 5; i++) {
      assert.ok(final.A[i] !== 1.0, `A[${i}] should have changed`);
    }

    state.delete();
    micm.delete();
  });

  it('should work with all solver types', async (t) => {
    for (const solverType of solverTypes) {
      const micm = MICM.fromConfigPath(getConfigPath(), solverType);
      const state = micm.createState(2);

      state.setConcentrations({ A: [1.0, 0.5], B: [0.0, 0.5], C: [0.5, 1.0] });
      state.setConditions({
        temperatures: [298.15, 300],
        pressures: [101325.0, 101325.0],
        airDensities: [1.0, 1.2],
      });
      state.setUserDefinedRateParameters({
        'USER.reaction 1': [0.001, 0.002],
        'USER.reaction 2': [0.002, 0.003],
      });

      const result = micm.solve(state, 1.0);
      assert.ok(result instanceof SolverResult, `Should solve with solver ${solverType}`);

      const updated = state.getConcentrations();
      for (let i = 0; i < 2; i++) {
        assert.ok(updated.A[i] !== (i === 0 ? 1.0 : 0.5), `A[${i}] should have changed`);
        assert.ok(updated.B[i] !== (i === 0 ? 0.0 : 0.5), `B[${i}] should have changed`);
        assert.ok(updated.C[i] !== (i === 0 ? 0.5 : 1.0), `C[${i}] should have changed`);
      }

      state.delete();
      micm.delete();
    }
  });
});

describe('MICM SolverResult validation', () => {
  it('should return valid SolverResult structure', async (t) => {
    const micm = MICM.fromConfigPath(getConfigPath());
    const state = micm.createState(1);

    state.setConditions({
      temperatures: 298.15,
      pressures: 101325.0,
      air_densities: 1.2,
    });

    state.setConcentrations({ A: 1.0 });
    state.setUserDefinedRateParameters({
      'USER.reaction 1': 0.001,
      'USER.reaction 2': 0.002,
    });

    const result = micm.solve(state, 1.0);

    // Validate result structure
    assert.ok(result instanceof SolverResult, 'Should be SolverResult');
    assert.ok(typeof result.state === 'number', 'Should have state property');
    assert.ok(result.stats instanceof SolverStats, 'Should have stats property');

    // Validate stats structure
    const stats = result.stats;
    assert.ok(stats.function_calls > 0n);
    assert.ok(stats.jacobian_updates > 0n);
    assert.ok(stats.number_of_steps > 0n);
    assert.ok(stats.accepted > 0n);
    assert.ok(stats.rejected >= 0n);
    assert.ok(stats.decompositions > 0n);
    assert.ok(stats.solves > 0n);
    assert.ok(stats.final_time >= 0.0);

    state.delete();
    micm.delete();
  });
});
