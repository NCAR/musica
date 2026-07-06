import * as micm from './micm/index.js';
import { initModule, getBackend } from './backend.js';

/**
 * Get MUSICA version
 * @returns {Promise<string>}
 */
export async function getVersion() {
  const backend = getBackend();
  return backend.getVersion();
}

/**
 * Get MICM version
 * @returns {Promise<string>}
 */
export async function getMicmVersion() {
  const backend = getBackend();
  return backend.getMicmVersion();
}

// Flatten exports
export const {
  MICM,
  State,
  Conditions,
  SolverType,
  SolverState,
  SolverStats,
  SolverResult,
  RosenbrockSolverParameters,
  BackwardEulerSolverParameters,
  GAS_CONSTANT,
  AVOGADRO,
  BOLTZMANN,
} = micm;

// Re-export as a namespace (rather than repackaging into an object literal) so
// the generated .d.ts preserves the nominal class identities of types.* /
// reactionTypes.* / Mechanism. Building an object literal here made tsc inline
// each class's structural instance shape, which broke assignability for
// downstream consumers constructing these objects programmatically.
export * as mechanismConfiguration from './mechanism_configuration/index.js';

export { initModule, getBackend };
