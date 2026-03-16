import { describe, it, before } from 'node:test';
import assert from 'node:assert';
import path from 'path';
import * as musica from '../../index.js';
import { isClose } from '../util/testUtils.js';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const { MICM, SolverType, GAS_CONSTANT } = musica;
const CONFIG_PATH = path.join(__dirname, '../../../configs/v0/analytical');

before(async () => {
  await musica.initModule();
});

// Combined single/multiple grid cell test
function testAnalytical(solver, state, numCells = 1, step = 1, places = 5) {
  const temperature = 272.5;
  const pressure = 101253.3;
  const airDensity = pressure / (GAS_CONSTANT * temperature);

  const conc = { A: [], B: [], C: [], D: [], E: [], F: [] };
  const rates = { 'USER.reaction 1': [], 'USER.reaction 2': [] };
  const temps = [];
  const press = [];

  for (let i = 0; i < numCells; i++) {
    temps.push(temperature);
    press.push(pressure);
    conc.A.push(0.75);
    conc.B.push(0);
    conc.C.push(0.4);
    conc.D.push(0.8);
    conc.E.push(0);
    conc.F.push(0.1);
    rates['USER.reaction 1'].push(0.001);
    rates['USER.reaction 2'].push(0.002);
  }

  state.setConditions({
    temperatures: temps,
    pressures: press,
    airDensities: Array(numCells).fill(airDensity),
  });
  state.setConcentrations(conc);
  state.setUserDefinedRateParameters(rates);

  const tolerance = Math.pow(10, -places);

  for (let t = step; t <= 100; t += step) {
    solver.solve(state, step);
    const c = state.getConcentrations();

    for (let i = 0; i < numCells; i++) {
      const k1 = rates['USER.reaction 1'][i];
      const k2 = rates['USER.reaction 2'][i];
      const k3 = 0.004 * Math.exp(50.0 / temps[i]);
      const k4 =
        0.012 *
        Math.exp(75.0 / temps[i]) *
        Math.pow(temps[i] / 50.0, -2) *
        (1.0 + 1.0e-6 * press[i]);

      const A = conc.A[i] * Math.exp(-k3 * t);
      const B = conc.A[i] * (k3 / (k4 - k3)) * (Math.exp(-k3 * t) - Math.exp(-k4 * t));
      const C =
        conc.C[i] +
        conc.A[i] * (1.0 + (k3 * Math.exp(-k4 * t) - k4 * Math.exp(-k3 * t)) / (k4 - k3));
      const D = conc.D[i] * Math.exp(-k1 * t);
      const E = conc.D[i] * (k1 / (k2 - k1)) * (Math.exp(-k1 * t) - Math.exp(-k2 * t));
      const F =
        conc.F[i] +
        conc.D[i] * (1.0 + (k1 * Math.exp(-k2 * t) - k2 * Math.exp(-k1 * t)) / (k2 - k1));

      assert.ok(isClose(c.A[i], A, tolerance), `Grid cell ${i}: A mismatch`);
      assert.ok(isClose(c.B[i], B, tolerance), `Grid cell ${i}: B mismatch`);
      assert.ok(isClose(c.C[i], C, tolerance), `Grid cell ${i}: C mismatch`);
      assert.ok(isClose(c.D[i], D, tolerance), `Grid cell ${i}: D mismatch`);
      assert.ok(isClose(c.E[i], E, tolerance), `Grid cell ${i}: E mismatch`);
      assert.ok(isClose(c.F[i], F, tolerance), `Grid cell ${i}: F mismatch`);
    }
  }
}

const solvers = [
  { name: 'Rosenbrock', type: SolverType.rosenbrock, step: 200.0, places: 5 },
  {
    name: 'Rosenbrock Standard',
    type: SolverType.rosenbrock_standard_order,
    step: 200.0,
    places: 5,
  },
  { name: 'Backward Euler', type: SolverType.backward_euler, step: 10.0, places: 2 },
  {
    name: 'Backward Euler Standard',
    type: SolverType.backward_euler_standard_order,
    step: 10.0,
    places: 2,
  },
];
const gridCellsList = [1, 5, 10, 20];

solvers.forEach(({ name, type, step, places }) => {
  describe(`Analytical - ${name}`, () => {
    for (const nCells of gridCellsList) {
      it(`should match analytical solution for ${nCells} grid cell${nCells > 1 ? 's' : ''}`, () => {
        const solver = MICM.fromConfigPath(CONFIG_PATH, type);
        const state = solver.createState(nCells);
        testAnalytical(solver, state, nCells, step, places);
      });
    }
  });
});
