/**
 * Converts a solver type to the WASM SolverType enum.
 * Accepts either a number (1-5) or a WASM enum value.
 * Throws if input is invalid.
 *
 * @param {number|any} solverType - Numeric solver type or WASM enum
 * @returns {any} WASM SolverType enum value
 */
export function toWasmSolverType(solverType: number | any): any;
/**
 * Enum for solver types
 */
export type SolverType = number;
export namespace SolverType {
    let rosenbrock: number;
    let rosenbrock_standard_order: number;
    let backward_euler: number;
    let backward_euler_standard_order: number;
}
