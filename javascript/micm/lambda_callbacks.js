// javascript/micm/lambda_callbacks.js
//
// Thin wrapper around the WASM registerReactionRateCallback function.
//
// Usage:
//   const id = registerReactionRateCallback((T, P, airDensity) => T * 1.5e-4);
//   solver.setReactionRateCallback('Lambda.my_rxn', id);

import { getBackend } from '../backend.js';

/**
 * Register a JavaScript function as a MICM lambda-rate-constant callback.
 *
 * @param {function(number, number, number): number} fn
 *   A function that receives (temperature [K], pressure [Pa], air_density
 *   [mol/m³]) and returns the rate-constant value.
 * @returns {number} A non-negative integer ID to pass to
 *   {@link MICM#setReactionRateCallback}.
 */
export function registerReactionRateCallback(fn) {
  const backend = getBackend();
  return backend.registerReactionRateCallback(fn);
}
