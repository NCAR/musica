import { State } from './state.js';
import { SolverType, toWasmSolverType } from './solver.js';
import { SolverStats, SolverResult } from './solver_result.js';
import { RosenbrockSolverParameters, BackwardEulerSolverParameters } from './solver_parameters.js';
import { getBackend } from '../backend.js';

export class MICM {
  /**
   * Create a MICM solver instance from a configuration file path
   *
   * @param {string} configPath - Path to the mechanism configuration directory
   * @param {number} [solverType=SolverType.rosenbrock_standard_order] - Type of solver to use
   * @param {RosenbrockSolverParameters|BackwardEulerSolverParameters} [solverParameters] - Optional solver parameters
   * @returns {MICM} A new MICM instance
   */
  static fromConfigPath(
    configPath,
    solverType = SolverType.rosenbrock_standard_order,
    solverParameters = null
  ) {
    if (typeof configPath !== 'string') {
      throw new TypeError('configPath must be a string');
    }

    try {
      const backend = getBackend();
      const wasmSolver = toWasmSolverType(solverType);
      // In Node.js with NODEFS mounted, configuration files are exposed under /host.
      // In browser environments, this prefix is invalid, so only add it when running under Node.
      let resolvedConfigPath = configPath;
      const isNodeEnv =
        typeof process !== 'undefined' && process.versions != null && process.versions.node != null;
      if (isNodeEnv && !configPath.startsWith('/host/')) {
        resolvedConfigPath = `/host/${configPath}`;
      }
      const nativeMICM = backend.MICM.fromConfigPath(resolvedConfigPath, wasmSolver);
      const micm = new MICM(nativeMICM, solverType);
      if (solverParameters) {
        micm.setSolverParameters(solverParameters);
      }
      return micm;
    } catch (error) {
      throw new Error(`Failed to create MICM solver from config path: ${error.message}`);
    }
  }

  /**
   * Create a MICM solver instance from a Mechanism object
   *
   * @param {Object} mechanism - Mechanism object created in code
   * @param {number} [solverType=SolverType.rosenbrock_standard_order] - Type of solver to use
   * @param {RosenbrockSolverParameters|BackwardEulerSolverParameters} [solverParameters] - Optional solver parameters
   * @returns {MICM} A new MICM instance
   */
  static fromMechanism(
    mechanism,
    solverType = SolverType.rosenbrock_standard_order,
    solverParameters = null
  ) {
    if (!mechanism || typeof mechanism.getJSON !== 'function') {
      throw new TypeError('mechanism must be a valid Mechanism object with getJSON() method');
    }

    try {
      const backend = getBackend();
      const wasmSolver = toWasmSolverType(solverType);
      const mechanismJSON = mechanism.getJSON();
      const jsonString = JSON.stringify(mechanismJSON);

      const nativeMICM = backend.MICM.fromConfigString(jsonString, wasmSolver);
      const micm = new MICM(nativeMICM, solverType);
      if (solverParameters) {
        micm.setSolverParameters(solverParameters);
      }
      return micm;
    } catch (error) {
      throw new Error(`Failed to create MICM solver from mechanism: ${error.message}`);
    }
  }

  /**
   * Private constructor - use static factory methods instead
   * @private
   */
  constructor(nativeMICM, solverType) {
    this._nativeMICM = nativeMICM;
    this._solverType = solverType;
  }

  solverType() {
    return this._solverType;
  }

  createState(numberOfGridCells = 1) {
    if (numberOfGridCells <= 0) {
      throw new RangeError('number_of_grid_cells must be greater than 0');
    }
    return new State(this._nativeMICM, numberOfGridCells, this._solverType);
  }

  solve(state, timeStep) {
    if (!(state instanceof State)) {
      throw new TypeError('state must be an instance of State');
    }
    if (typeof timeStep !== 'number') {
      throw new TypeError('timeStep must be a number');
    }

    const result = this._nativeMICM.solve(state._nativeState, timeStep);

    // Convert the plain object to a SolverResult instance
    const stats = new SolverStats(result.stats);
    return new SolverResult(result.state, stats);
  }

  /**
   * Set solver-specific parameters.
   *
   * @param {RosenbrockSolverParameters|BackwardEulerSolverParameters} params
   */
  setSolverParameters(params) {
    const backend = getBackend();
    const toVectorDouble = (arr) => {
      const vec = new backend.VectorDouble();
      if (arr) {
        for (const v of arr) vec.push_back(v);
      }
      return vec;
    };
    if (params instanceof RosenbrockSolverParameters) {
      const wasmParams = {
        relative_tolerance: params.relative_tolerance,
        absolute_tolerances: toVectorDouble(params.absolute_tolerances),
        h_min: params.h_min,
        h_max: params.h_max,
        h_start: params.h_start,
        max_number_of_steps: params.max_number_of_steps,
      };
      this._nativeMICM.set_rosenbrock_solver_parameters(wasmParams);
    } else if (params instanceof BackwardEulerSolverParameters) {
      const wasmParams = {
        relative_tolerance: params.relative_tolerance,
        absolute_tolerances: toVectorDouble(params.absolute_tolerances),
        max_number_of_steps: params.max_number_of_steps,
        time_step_reductions: toVectorDouble(params.time_step_reductions),
      };
      this._nativeMICM.set_backward_euler_solver_parameters(wasmParams);
    } else {
      throw new TypeError(
        'params must be RosenbrockSolverParameters or BackwardEulerSolverParameters'
      );
    }
  }

  /**
   * Get the current solver parameters.
   *
   * @returns {RosenbrockSolverParameters|BackwardEulerSolverParameters}
   */
  getSolverParameters() {
    const fromVectorDouble = (vec) => {
      const arr = [];
      for (let i = 0; i < vec.size(); i++) arr.push(vec.get(i));
      return arr;
    };
    if (
      this._solverType === SolverType.rosenbrock ||
      this._solverType === SolverType.rosenbrock_standard_order
    ) {
      const wasmParams = this._nativeMICM.get_rosenbrock_solver_parameters();
      const absTol = fromVectorDouble(wasmParams.absolute_tolerances);
      return new RosenbrockSolverParameters({
        relative_tolerance: wasmParams.relative_tolerance,
        absolute_tolerances: absTol.length > 0 ? absTol : null,
        h_min: wasmParams.h_min,
        h_max: wasmParams.h_max,
        h_start: wasmParams.h_start,
        max_number_of_steps: wasmParams.max_number_of_steps,
      });
    } else if (
      this._solverType === SolverType.backward_euler ||
      this._solverType === SolverType.backward_euler_standard_order
    ) {
      const wasmParams = this._nativeMICM.get_backward_euler_solver_parameters();
      const absTol = fromVectorDouble(wasmParams.absolute_tolerances);
      return new BackwardEulerSolverParameters({
        relative_tolerance: wasmParams.relative_tolerance,
        absolute_tolerances: absTol.length > 0 ? absTol : null,
        max_number_of_steps: wasmParams.max_number_of_steps,
        time_step_reductions: fromVectorDouble(wasmParams.time_step_reductions),
      });
    } else {
      throw new Error(`Unknown solver type: ${this._solverType}`);
    }
  }

  delete() {
    this._nativeMICM.delete();
  }
}
