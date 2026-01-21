import { isScalarNumber } from './utils.js';
import { getBackend } from '../backend.js';
import { toWasmSolverType } from './solver.js';
import { GAS_CONSTANT } from './utils.js';

export class State {
  constructor(nativeMICM, numberOfGridCells, solverType) {
    if (!nativeMICM) {
      throw new TypeError('nativeMICM is required');
    }
    if (numberOfGridCells < 1) {
      throw new RangeError('number_of_grid_cells must be greater than 0');
    }

    const backend = getBackend();
    this._nativeState = backend.create_state(nativeMICM, numberOfGridCells);

    this._numberOfGridCells = numberOfGridCells;
    this._solverType = toWasmSolverType(solverType);
  }

  setConcentrations(concentrations) {
    const formatted = {};
    for (const [name, value] of Object.entries(concentrations)) {
      formatted[name] = isScalarNumber(value) ? [value] : value;
    }
    this._nativeState.set_concentrations(formatted, this._solverType);
  }

  getConcentrations() {
    return this._nativeState.get_concentrations(this._solverType);
  }

  setUserDefinedRateParameters(params) {
    const formatted = {};
    for (const [name, value] of Object.entries(params)) {
      formatted[name] = isScalarNumber(value) ? [value] : value;
    }
    this._nativeState.set_user_defined_constants(formatted, this._solverType);
  }

  getUserDefinedRateParameters() {
    return this._nativeState.get_user_defined_constants(this._solverType);
  }

  setConditions({ temperatures = null, pressures = null, airDensities = null } = {}) {
    const backend = getBackend();
    const vec = new backend.VectorConditions();

    const expand = (param) => {
      if (param === null || param === undefined) {
        return Array(this._numberOfGridCells).fill(null);
      } else if (!Array.isArray(param)) {
        if (this._numberOfGridCells > 1) {
          throw new Error('Scalar input requires a single grid cell');
        }
        return [param];
      } else if (param.length !== this._numberOfGridCells) {
        throw new Error(`Array input must have length ${this._numberOfGridCells}`);
      }
      return param;
    };

    const temps = expand(temperatures);
    const pres = expand(pressures);
    const dens = expand(airDensities);

    for (let i = 0; i < this._numberOfGridCells; i++) {
      const T = temps[i] !== null ? temps[i] : NaN;
      const P = pres[i] !== null ? pres[i] : NaN;
      let rho =
        dens[i] !== null
          ? dens[i]
          : !Number.isNaN(T) && !Number.isNaN(P)
            ? P / (GAS_CONSTANT * T)
            : NaN;

      const cond = new backend.Condition(T, P, rho);
      vec.push_back(cond);
    }

    this._nativeState.set_conditions(vec);
  }

  getConditions() {
    return this._nativeState.get_conditions();
  }

  getNumberOfGridCells() {
    return this._numberOfGridCells;
  }

  delete() {
    this._nativeState.delete();
  }
}
