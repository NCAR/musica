/**
 * Integration tests for Chapman mechanism
 * Mirrors python/test/integration/test_chapman.py
 * Uses Node.js built-in test runner
 */

import { describe, it, before } from 'node:test';
import assert from 'node:assert';
import path from 'path';
import * as musica from '../../index.js';

import { fileURLToPath } from 'url';

// Convert import.meta.url to a file path
const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const { MICM, SolverType } = musica;

before(async () => {
  await musica.initModule();
});

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
    'PHOTO.jo2_b': 2.42e-17,
    'PHOTO.jo3_a': 1.15e-5,
    'PHOTO.jo3_b': 6.61e-9,
  };

  const initialConcentrations = {
    N2: 0.75,
    O: 0.0,
    O1D: 0.0,
    O3: 0.0000081,
    O2: 0.21,
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
  console.log('Concentrations after solve:', concentrations);

  // Verify concentrations change
  assert.ok(
    concentrations['O3'][0] != initialConcentrations['O3'],
    'O3 concentration should change after solve'
  );
  assert.ok(
    concentrations['O'][0] != initialConcentrations['O'],
    'O concentration should change after solve'
  );
  assert.ok(
    concentrations['O1D'][0] != initialConcentrations['O1D'],
    'O1D concentration should change after solve'
  );
  assert.ok(
    concentrations['O2'][0] != initialConcentrations['O2'],
    'O2 concentration should change after solve'
  );
}

describe('Chapman mechanism with v1 config files', () => {
  it('should solve with v1 JSON config file', async (t) => {
    const configPath = path.join(__dirname, '../../../configs/v1/chapman/config.json');
    const solver = MICM.fromConfigPath(configPath, SolverType.rosenbrock_standard_order);
    testSolve(solver);
  });

  it('should solve with v1 YAML config file', async (t) => {
    const configPath = path.join(__dirname, '../../../configs/v1/chapman/config.yaml');
    const solver = MICM.fromConfigPath(configPath, SolverType.rosenbrock_standard_order);
    testSolve(solver);
  });
});
