/**
 * WASM MICM tests that use `fromMechanism` exclusively.
 * These tests will run once the WASM module is built
 * Run with: node --test javascript/tests/unit/wasm/micm.test.js
 */

const { describe, it } = require('node:test');
const assert = require('node:assert');
const { musica_wasm } = require('../../util/testUtils.js');
// Use the JS mechanism configuration types (pure-JS) to construct
// Mechanism objects that are compatible with the WASM `fromMechanism` API.
const { mechanismConfiguration } = require('../../../index.js');
const { MICM } = require('../../../micm/micm.js');

function makeMechanism() {
  const { types, reactionTypes, Mechanism } = mechanismConfiguration;
  const { Species, Phase, ReactionComponent } = types;

  const A = new Species({ name: 'A' });
  const B = new Species({ name: 'B' });
  const C = new Species({ name: 'C' });

  const gas = new Phase({ name: 'gas', species: [A, B, C] });

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

  return new Mechanism({
    name: 'WASM Test Mechanism',
    version: '1.0.0',
    species: [A, B, C],
    phases: [gas],
    reactions: [reaction1, reaction2]
  });
}

describe('WASM MICM Tests (fromMechanism only)', () => {
  it('should create MICM from a Mechanism object', async (t) => {
    if (!musica_wasm.hasWasm) { t.skip(); return; }

    await MICM.initWasm();
    const mechanism = makeMechanism();
    const micm = MICM.fromMechanism(mechanism, 1);
    assert.ok(micm, 'MICM class should create instance from Mechanism');
  });

  it('should create state from MICM', async (t) => {
    if (!musica_wasm.hasWasm) { t.skip(); return; }

    await MICM.initWasm();
    const mechanism = makeMechanism();
    const micm = MICM.fromMechanism(mechanism, 1);

    const state = micm.createState(1);
    assert.ok(state, 'State should be created');
    const numGridCells = state.getNumberOfGridCells();
    assert.strictEqual(numGridCells, 1, 'Number of grid cells should be 1');
  });

  it('should set and get concentrations', async (t) => {
    if (!musica_wasm.hasWasm) { t.skip(); return; }

    await MICM.initWasm();
    const mechanism = makeMechanism();
    const micm = MICM.fromMechanism(mechanism, 1);
    const state = micm.createState(1);

    state.setConcentrations({ A: [1.0], B: [0.5], C: [0.25] });
    const concentrations = state.getConcentrations();

    assert.ok(concentrations, 'Concentrations should be returned');
    assert.ok(concentrations.A, 'Species A should exist');
    assert.ok(Array.isArray(concentrations.A), 'Concentration should be an array');
  });

  it('should set and get conditions', async (t) => {
    if (!musica_wasm.hasWasm) { t.skip(); return; }

    const module = await musica_wasm.initModule();
    const mechanism = makeMechanism();
    const micm = MICM.fromMechanism(mechanism, 1);
    const state = micm.createState(1);

    state.setConditions({ temperatures: [298.15], pressures: [101325.0], air_densities: [1.0] });
    const conditions = state.getConditions();

    assert.ok(conditions, 'Conditions should be returned');
    assert.ok(conditions.temperature, 'Temperature should exist');
    assert.ok(Array.isArray(conditions.temperature), 'Temperature should be an array');
  });

  it('should solve chemical system', async (t) => {
    if (!musica_wasm.hasWasm) { t.skip(); return; }

    await MICM.initWasm();
    const mechanism = makeMechanism();
    const micm = MICM.fromMechanism(mechanism, 1);
    const state = micm.createState(1);

    state.setConcentrations({ A: [1.0], B: [2.0], C: [3.0] });
    state.setConditions({ temperatures: [298.15], pressures: [101325.0], air_densities: [1.0] });
    state.setUserDefinedRateParameters({
      'USER.reaction 1': 0.001,
      'USER.reaction 2': 0.002
    });

    const result = micm.solve(state, 60.0);
    let concentrations = state.getConcentrations();

    assert.ok(concentrations.A !== 1.0, 'Concentration of A should have changed after solve');
    assert.ok(concentrations.B !== 2.0, 'Concentration of B should have changed after solve');
    assert.ok(concentrations.C !== 3.0, 'Concentration of C should have changed after solve');

    assert.ok(result, 'Result should be returned');
    assert.ok(result.stats, 'Result should have stats');
    assert.ok(typeof result.stats.function_calls !== 'undefined', 'Stats should have function_calls');
    assert.ok(typeof result.stats.accepted !== 'undefined', 'Stats should have accepted');
  });

  it('should get species ordering', async (t) => {
    if (!musica_wasm.hasWasm) { t.skip(); return; }

    await MICM.initWasm();
    const mechanism = makeMechanism();
    const micm = MICM.fromMechanism(mechanism, 1);
    const state = micm.createState(1);

    const ordering = state.getSpeciesOrdering();
    assert.ok(ordering, 'Species ordering should be returned');
    assert.strictEqual(typeof ordering, 'object', 'Ordering should be an object');
  });
});
