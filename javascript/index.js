import * as micm from './micm/index.js';
import * as mc from './mechanism_configuration/index.js';
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
  GAS_CONSTANT,
  AVOGADRO,
  BOLTZMANN,
} = micm;

export const mechanismConfiguration = {
  types: mc.types,
  reactionTypes: mc.reactionTypes,
  Mechanism: mc.Mechanism,
};

export {
  initModule,
  getBackend,
};