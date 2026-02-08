/**
 * Unit tests for the State class
 * Mirrors python/test/unit/micm/test_state.py
 * Uses Node.js built-in test runner
 */

import { describe, it, before } from 'node:test';
import assert from 'node:assert';
import path from 'path';
import * as musica from '../../index.js';
import { isClose } from '../util/testUtils.js';
import { fileURLToPath } from 'url';

const { MICM } = musica;

// Convert import.meta.url to a file path
const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

before(async () => {
  await musica.initModule();
});

/**
 * Helper function to create a test mechanism
 * This creates a simple mechanism for testing state operations
 */
function getConfigPath() {
  // For JavaScript, we'll use a config path instead of mechanism object
  // as the mechanism configuration API might not be fully exposed
  return path.join(__dirname, '../../../configs/v0/analytical');
}

describe('State initialization', () => {
  it('should create state with single grid cell', async (t) => {
    const configPath = getConfigPath();
    const solver = MICM.fromConfigPath(configPath);
    const state = solver.createState(1);
    assert.ok(state, 'State should be created');
  });

  it('should create state with multiple grid cells', async (t) => {
    const configPath = getConfigPath();
    const solver = MICM.fromConfigPath(configPath);
    const state = solver.createState(3);
    assert.ok(state, 'State with 3 grid cells should be created');
  });

  it('should throw error for invalid grid cell count', async (t) => {
    const configPath = getConfigPath();
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
  const gridCellsList = [1, 5, 10, 20];

  before(() => {
    const configPath = getConfigPath();
    solver = MICM.fromConfigPath(configPath);
  });

  it('should set and get concentrations for various numbers of grid cells', async () => {
    for (const nCells of gridCellsList) {
      state = solver.createState(nCells);

      // Set concentrations
      const concentrations = {
        A: Array.from({ length: nCells }, (_, i) => i + 1),
        B: Array.from({ length: nCells }, (_, i) => i + 10),
        C: Array.from({ length: nCells }, (_, i) => i + 100),
      };
      state.setConcentrations(concentrations);

      const result = state.getConcentrations();

      for (let i = 0; i < nCells; i++) {
        assert.ok(isClose(result.A[i], concentrations.A[i], 1e-13), `A[${i}] should match`);
        assert.ok(isClose(result.B[i], concentrations.B[i], 1e-13), `B[${i}] should match`);
        assert.ok(isClose(result.C[i], concentrations.C[i], 1e-13), `C[${i}] should match`);
      }
    }
  });

  it('should handle empty concentration update without changing values', async () => {
    for (const nCells of gridCellsList) {
      state = solver.createState(nCells);

      const concentrations = {
        A: Array(nCells).fill(1.0),
        B: Array(nCells).fill(2.0),
        C: Array(nCells).fill(3.0),
      };
      state.setConcentrations(concentrations);

      // Empty update
      state.setConcentrations({});
      const result = state.getConcentrations();

      for (let i = 0; i < nCells; i++) {
        assert.ok(isClose(result.A[i], concentrations.A[i], 1e-13), `A[${i}] should remain`);
        assert.ok(isClose(result.B[i], concentrations.B[i], 1e-13), `B[${i}] should remain`);
        assert.ok(isClose(result.C[i], concentrations.C[i], 1e-13), `C[${i}] should remain`);
      }
    }
  });
});

describe('Conditions', () => {
  let solver;
  let state;
  const gridCellsList = [1, 5, 10, 20];

  before(() => {
    const configPath = getConfigPath();
    solver = MICM.fromConfigPath(configPath);
  });

  it('should set and get conditions for various numbers of grid cells', async () => {
    for (const nCells of gridCellsList) {
      state = solver.createState(nCells);

      // Create test values
      const temperatures = Array.from({ length: nCells }, (_, i) => 300.0 + i);
      const pressures = Array.from({ length: nCells }, () => 101325.0);
      const airDensities = temperatures.map((T, i) => 101325.0 / (8.31446261815324 * T));

      state.setConditions({ temperatures, pressures });
      const conditions = state.getConditions();

      assert.strictEqual(conditions.length, nCells, `Should have ${nCells} grid cells`);

      for (let i = 0; i < nCells; i++) {
        assert.ok(
          isClose(conditions[i].temperature, temperatures[i], 1e-13),
          `Temperature[${i}] should match`
        );
        assert.ok(
          isClose(conditions[i].pressure, pressures[i], 1e-13),
          `Pressure[${i}] should match`
        );
        assert.ok(
          isClose(conditions[i].air_density, airDensities[i], 0.1),
          `Air density[${i}] should match`
        );
      }
    }
  });

  it('should handle integer values for conditions', async () => {
    for (const nCells of gridCellsList) {
      state = solver.createState(nCells);

      const temperatures = Array.from({ length: nCells }, () => 272);
      const pressures = Array.from({ length: nCells }, () => 101325);
      const airDensities = temperatures.map((T) => 101325 / (8.31446261815324 * T));

      state.setConditions({ temperatures, pressures });
      const conditions = state.getConditions();

      assert.strictEqual(conditions.length, nCells, `Should have ${nCells} grid cells`);

      for (let i = 0; i < nCells; i++) {
        assert.ok(
          isClose(conditions[i].temperature, 272, 1e-13),
          `Temperature[${i}] should be 272`
        );
        assert.ok(
          isClose(conditions[i].pressure, 101325, 1e-13),
          `Pressure[${i}] should be 101325`
        );
        assert.ok(
          isClose(conditions[i].air_density, airDensities[i], 1e-13),
          `Air density[${i}] should be calculated`
        );
      }
    }
  });
});

describe('User-defined rate parameters', () => {
  let solver;
  let state;
  const gridCellsList = [1, 5, 10, 20];

  before(() => {
    const configPath = getConfigPath();
    solver = MICM.fromConfigPath(configPath);
  });

  it('should set and get user-defined rate parameters for multiple grid cells', async () => {
    for (const nCells of gridCellsList) {
      state = solver.createState(nCells);

      const params = { 'USER.reaction 1': Array.from({ length: nCells }, (_, i) => 1.0 + i) };
      state.setUserDefinedRateParameters(params);
      const result = state.getUserDefinedRateParameters();

      assert.strictEqual(result['USER.reaction 1'].length, nCells, `Should have ${nCells} values`);
      for (let i = 0; i < nCells; i++) {
        assert.ok(
          isClose(result['USER.reaction 1'][i], 1.0 + i, 1e-13),
          `Rate parameter[${i}] should match`
        );
      }
    }
  });

  it('should handle empty rate parameter update', async () => {
    for (const nCells of gridCellsList) {
      state = solver.createState(nCells);

      const params = { 'USER.reaction 1': Array.from({ length: nCells }, () => 1.0) };
      state.setUserDefinedRateParameters(params);

      // Set empty parameters - should not change values
      state.setUserDefinedRateParameters({});
      const result = state.getUserDefinedRateParameters();

      for (let i = 0; i < nCells; i++) {
        assert.ok(
          isClose(result['USER.reaction 1'][i], 1.0, 1e-13),
          `Rate parameter[${i}] should remain 1.0`
        );
      }
    }
  });
});
