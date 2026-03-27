import { isScalarNumber } from './utils.js';
import { getBackend } from '../backend.js';
import { toWasmSolverType } from './solver.js';
import { GAS_CONSTANT } from './utils.js';

/**
 * Chemical state for one or more grid cells.
 *
 * Holds species concentrations, environmental conditions, and user-defined
 * rate parameters. Create via {@link MICM#createState} rather than directly.
 */
export class State {
  /** @private */
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

  /**
   * Set species concentrations.
   *
   * @param {Object.<string, number|number[]>} concentrations - Map of species name to
   *   concentration(s) in mol m⁻³. For a single grid cell, values may be scalars.
   *   For multiple grid cells, provide arrays of length `numberOfGridCells`.
   */
  setConcentrations(concentrations) {
    const formatted = {};
    for (const [name, value] of Object.entries(concentrations)) {
      formatted[name] = isScalarNumber(value) ? [value] : value;
    }
    this._nativeState.set_concentrations(formatted, this._solverType);
  }

  /**
   * Get species concentrations for all grid cells.
   *
   * @returns {Object.<string, number[]>} Map of species name to array of concentrations
   *   (one element per grid cell) in mol m⁻³.
   */
  getConcentrations() {
    return this._nativeState.get_concentrations(this._solverType);
  }

  /**
   * Set user-defined rate parameters (e.g. photolysis rates or emission fluxes).
   *
   * @param {Object.<string, number|number[]>} params - Map of parameter name to value(s).
   *   For a single grid cell, values may be scalars. For multiple grid cells, provide
   *   arrays of length `numberOfGridCells`.
   */
  setUserDefinedRateParameters(params) {
    const formatted = {};
    for (const [name, value] of Object.entries(params)) {
      formatted[name] = isScalarNumber(value) ? [value] : value;
    }
    this._nativeState.set_user_defined_constants(formatted, this._solverType);
  }

  /**
   * Get user-defined rate parameters for all grid cells.
   *
   * @returns {Object.<string, number[]>} Map of parameter name to array of values
   *   (one element per grid cell).
   */
  getUserDefinedRateParameters() {
    return this._nativeState.get_user_defined_constants(this._solverType);
  }

  /**
   * Set environmental conditions for each grid cell.
   *
   * All parameters accept a scalar (single grid cell) or an array of length
   * `numberOfGridCells`. If `airDensities` is omitted, air density is computed
   * from the Ideal Gas Law using the provided temperature and pressure.
   *
   * @param {Object} [options]
   * @param {number|number[]|null} [options.temperatures=null] - Temperature(s) in Kelvin
   * @param {number|number[]|null} [options.pressures=null] - Pressure(s) in Pascals
   * @param {number|number[]|null} [options.airDensities=null] - Air number density in mol m⁻³
   */
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

  /**
   * Get environmental conditions for all grid cells.
   *
   * @returns {Array<{temperature: number, pressure: number, air_density: number}>}
   *   One object per grid cell.
   */
  getConditions() {
    return this._nativeState.get_conditions();
  }

  /**
   * Get the number of grid cells in this state.
   * @returns {number}
   */
  getNumberOfGridCells() {
    return this._numberOfGridCells;
  }

  /**
   * Free the underlying WASM object. Call when done with this instance.
   */
  delete() {
    this._nativeState.delete();
  }
}
