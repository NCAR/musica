import assert from 'node:assert';
import * as musica from '../index.js';

await musica.initModule();

const { MICM, GAS_CONSTANT } = musica;
const { types, reactionTypes, Mechanism } = musica.mechanismConfiguration;
const { Species, Phase, ReactionComponent } = types;
const { UserDefined } = reactionTypes;

const A = new Species({ name: 'A' });
const B = new Species({ name: 'B' });

const gas = new Phase({
  name: 'gas',
  species: [A, B],
});

const reactions = [
  new UserDefined({
    name: 'mine',
    gas_phase: 'gas',
    reactants: [new ReactionComponent({ species_name: 'A' })],
    products: [new ReactionComponent({ species_name: 'B' })],
  })
];

const mechanism = new Mechanism({
  name: 'Test Lambda',
  version: '1.0.0',
  species: [A, B],
  phases: [gas],
  reactions: reactions,
});

const solver = MICM.fromMechanism(mechanism);
const state = solver.createState(1);
state.setConcentrations({ A: [1.0], B: [0.0] });
state.setConditions({ temperatures: [298.15], pressures: [101325.0] });

// Set a user-defined rate parameter for the reaction
state.setUserDefinedRateParameters({ 'USER.mine': [0.01] });

const result = solver.solve(state, 60.0);
const concentrations = state.getConcentrations();

assert.ok(concentrations.A[0] !== 1.0, 'Concentration of A should have changed after solve');
assert.ok(concentrations.B[0] > 0.0, 'Concentration of B should have changed after solve');
console.log(result)
console.log('Test passed!');

