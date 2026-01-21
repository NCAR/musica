import { getBackend } from '../backend.js';

/**
 * Enum for solver types
 * @readonly
 * @enum {number}
 */
export const SolverType = {
  rosenbrock: 1, // Vector-ordered Rosenbrock solver
  rosenbrock_standard_order: 2, // Standard-ordered Rosenbrock solver
  backward_euler: 3, // Vector-ordered BackwardEuler solver
  backward_euler_standard_order: 4, // Standard-ordered BackwardEuler solver
};

/**
 * Converts a solver type to the WASM SolverType enum.
 * Accepts either a number (1-5) or a WASM enum value.
 * Throws if input is invalid.
 *
 * @param {number|any} solverType - Numeric solver type or WASM enum
 * @returns {any} WASM SolverType enum value
 */
export function toWasmSolverType(solverType) {
  if (solverType === undefined || solverType === null) {
    throw new TypeError('solverType is required');
  }

  const backend = getBackend();
  const WASMEnum = backend.SolverType;

  if (typeof solverType === 'number') {
    switch (solverType) {
      case 1:
        return WASMEnum.Rosenbrock;
      case 2:
        return WASMEnum.RosenbrockStandardOrder;
      case 3:
        return WASMEnum.BackwardEuler;
      case 4:
        return WASMEnum.BackwardEulerStandardOrder;
      case 5:
        return WASMEnum.CudaRosenbrock;
      default:
        throw new RangeError(`Invalid numeric solverType: ${solverType}`);
    }
  }

  // If it’s already one of the WASM enum values, accept it
  for (const key of Object.keys(WASMEnum)) {
    if (solverType === WASMEnum[key]) {
      return solverType;
    }
  }

  throw new TypeError('solverType must be a valid WASM MICMSolver enum or number');
}
