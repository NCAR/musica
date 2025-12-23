/**
 * Example tests for WASM MICM functionality
 * These tests will run once the WASM module is built
 * Run with: node --test javascript/tests/unit/wasm/micm.test.js
 */

const { describe, it } = require('node:test');
const assert = require('node:assert');
const path = require('path');
const { musica_wasm } = require('../../util/testUtils.js');

/**
 * Helper to get the config path for testing
 */
function getConfigPath() {
  return path.join(__dirname, '../../../../configs/v0/analytical');
}

describe('WASM MICM Tests', () => {
  it('should create MICM from config path', async (t) => {
    if (!musica_wasm.hasWasm) {
      t.skip();
      return;
    }

    const module = await musica_wasm.initModule();
    assert.ok(module.MICM, 'MICM class should be available');
    
    const micm = module.MICM.fromConfigPath(getConfigPath(), 1);
    assert.ok(micm, 'MICM instance should be created');
    
    const solverType = micm.solverType();
    assert.strictEqual(solverType, 1, 'Solver type should be 1 (rosenbrock_standard_order)');
  });

  it('should create state from MICM', async (t) => {
    if (!musica_wasm.hasWasm) {
      t.skip();
      return;
    }

    const module = await musica_wasm.initModule();
    const micm = module.MICM.fromConfigPath(getConfigPath(), 1);
    
    const state = micm.createState(1);
    assert.ok(state, 'State should be created');
    
    const numGridCells = state.getNumberOfGridCells();
    assert.strictEqual(numGridCells, 1, 'Number of grid cells should be 1');
  });

  it('should set and get concentrations', async (t) => {
    if (!musica_wasm.hasWasm) {
      t.skip();
      return;
    }

    const module = await musica_wasm.initModule();
    const micm = module.MICM.fromConfigPath(getConfigPath(), 1);
    const state = micm.createState(1);
    
    // Set concentrations
    state.setConcentrations({
      'A': [1.0],
      'B': [0.5],
      'C': [0.25]
    });
    
    // Get concentrations back
    const concentrations = state.getConcentrations();
    assert.ok(concentrations, 'Concentrations should be returned');
    assert.ok(concentrations.A, 'Species A should exist');
    assert.ok(Array.isArray(concentrations.A), 'Concentration should be an array');
  });

  it('should set and get conditions', async (t) => {
    if (!musica_wasm.hasWasm) {
      t.skip();
      return;
    }

    const module = await musica_wasm.initModule();
    const micm = module.MICM.fromConfigPath(getConfigPath(), 1);
    const state = micm.createState(1);
    
    // Set conditions
    state.setConditions({
      temperatures: [298.15],
      pressures: [101325.0],
      air_densities: [1.0]
    });
    
    // Get conditions back
    const conditions = state.getConditions();
    assert.ok(conditions, 'Conditions should be returned');
    assert.ok(conditions.temperatures, 'Temperature should exist');
    assert.ok(Array.isArray(conditions.temperatures), 'Temperature should be an array');
  });

  it('should solve chemical system', async (t) => {
    if (!musica_wasm.hasWasm) {
      t.skip();
      return;
    }

    const module = await musica_wasm.initModule();
    const micm = module.MICM.fromConfigPath(getConfigPath(), 1);
    const state = micm.createState(1);
    
    // Set initial conditions
    state.setConcentrations({
      'A': [1.0],
      'B': [0.0],
      'C': [0.0]
    });
    
    state.setConditions({
      temperatures: [298.15],
      pressures: [101325.0],
      air_densities: [1.0]
    });
    
    // Solve
    const result = micm.solve(state, 60.0);
    assert.ok(result, 'Result should be returned');
    assert.ok(result.stats, 'Result should have stats');
    
    // Check that stats has expected properties
    assert.ok(typeof result.stats.function_calls !== 'undefined', 'Stats should have function_calls');
    assert.ok(typeof result.stats.accepted !== 'undefined', 'Stats should have accepted');
  });

  it('should get species ordering', async (t) => {
    if (!musica_wasm.hasWasm) {
      t.skip();
      return;
    }

    const module = await musica_wasm.initModule();
    const micm = module.MICM.fromConfigPath(getConfigPath(), 1);
    const state = micm.createState(1);
    
    const ordering = state.getSpeciesOrdering();
    assert.ok(ordering, 'Species ordering should be returned');
    assert.strictEqual(typeof ordering, 'object', 'Ordering should be an object');
  });
});
