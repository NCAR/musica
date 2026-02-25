// Registry for JavaScript rate functions that can be called from C++

const rateFunctionRegistry = new Map();
let nextId = 1;

/**
 * Register a JavaScript function to calculate reaction rates
 * @param {Function} fn - Function that takes (temperature, pressure, air_density) and returns a rate
 * @returns {number} ID of the registered function
 */
export function registerRateFunction(fn) {
  const id = nextId++;
  rateFunctionRegistry.set(id, fn);
  return id;
}

/**
 * Call a registered rate function by ID
 * @param {number} id - ID of the function to call
 * @param {number} T - Temperature [K]
 * @param {number} P - Pressure [Pa]
 * @param {number} rho - Air density [mol m-3]
 * @returns {number} Calculated rate constant
 */
export function callRateFunction(id, T, P, rho) {
  const fn = rateFunctionRegistry.get(id);
  if (!fn) {
    throw new Error(`Rate function with id ${id} not found`);
  }
  return fn(T, P, rho);
}
