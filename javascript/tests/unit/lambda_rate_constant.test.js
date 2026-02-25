import assert from 'node:assert';
import { describe, it, before } from 'node:test';
import path from 'path';
import * as musica from '../../index.js';
import { fileURLToPath } from 'url';

const {
  MICM,
  SolverType,
  State,
  SolverResult,
  SolverStats,
  RosenbrockSolverParameters,
  BackwardEulerSolverParameters,
  registerReactionRateCallback,
} = musica;

await musica.initModule();


const { types, reactionTypes, Mechanism } = musica.mechanismConfiguration;
const { Species, Phase, ReactionComponent } = types;
const A = new Species({ name: 'A' })
const B = new Species({ name: 'B' })

let species = [A, B]
let reactions = [new reactionTypes.LambdaRateConstant({
  reactants: [new ReactionComponent({ species_name: 'A' })],
  products: [new ReactionComponent({ species_name: 'B' })],
  name: 'mine',
  gas_phase: 'gas',
})]

const gas = new Phase({
  name: 'gas',
  species: [A, B],
});

let mechanism = new Mechanism({
  name: 'Lambda Test',
  version: '1.0.0',
  phases: [gas],
  species: species,
  reactions: reactions
})

const solver = MICM.fromMechanism(mechanism);
const state = solver.createState(1);

state.setConcentrations({ A: [1.0], B: [0.0]});
state.setConditions({ temperatures: [298.15], pressures: [101325.0]});

// Register a JS lambda: constant rate fast enough to drive observable chemistry
const id = registerReactionRateCallback((T, P, airDensity) => 1e-3);
solver.setReactionRateCallback('Lambda.mine', id);

const result = solver.solve(state, 60.0);
const concentrations = state.getConcentrations();

assert.ok(concentrations.A[0] !== 1.0, 'Concentration of A should have changed after solve');
assert.ok(concentrations.B[0] > 0.0, 'Concentration of B should have changed after solve');
