// MUSICA JavaScript API using WASM backend
const { GAS_CONSTANT, AVOGADRO, BOLTZMANN } = require('./micm/utils.js');
const { SolverType } = require('./micm/solver.js');
const { SolverState, SolverStats, SolverResult } = require('./micm/solver_result.js');
const { Conditions } = require('./micm/conditions.js');
const { MICM } = require('./micm/micm.js');
const { State } = require('./micm/state.js');
const { types } = require('./mechanism_configuration/types.js');
const { reactionTypes } = require('./mechanism_configuration/reaction_types.js');
const { Mechanism } = require('./mechanism_configuration/mechanism.js');

// WASM module initialization
let musicaModule = null;

const isNodeEnv = typeof process !== 'undefined' && process.versions && process.versions.node;

function WasmBuilt() {
  if (!isNodeEnv) return;
  const path = require('path');
  const fs = require('fs');
  const wasmFile = path.resolve(__dirname, 'wasm/musica.wasm');
  const jsFile = path.resolve(__dirname, 'wasm/musica.js');
  return fs.existsSync(wasmFile) && fs.existsSync(jsFile);
}

/**
 * Initialize the MUSICA WASM module
 * This must be called before using any other functions
 * @returns {Promise<Object>} The initialized module
 */
async function initModule() {
  if (musicaModule) {
    return musicaModule;
  }

  // In a browser environment, the WASM files should be served from the same directory
  // In Node.js, we need to load from the filesystem
  const isNode = typeof process !== 'undefined' && process.versions && process.versions.node;

  if (isNode) {
    // Try to load the module factory
    try {
      const createMusicaModule = require('./wasm/musica.js');
      musicaModule = await createMusicaModule();
    } catch (error) {
      throw new Error('MUSICA WASM module not found. Please build the WASM module first. Error: ' + error.message);
    }
  } else {
    // Browser environment - requires musica.js to be loaded via script tag first
    if (typeof createMusicaModule === 'undefined') {
      throw new Error('Browser environment detected. Please load musica.js via script tag before using this module. See example.html for a working example.');
    }
    musicaModule = await createMusicaModule();
  }

  return musicaModule;
}

/**
 * Get the MUSICA version
 * @returns {Promise<string>} The version string
 */
async function getVersion() {
  const module = await initModule();
  return module.getVersion();
}

/**
 * Get the MICM version
 * @returns {Promise<string>} The version string
 */
async function getMicmVersion() {
  const module = await initModule();
  return module.getMicmVersion();
}

const micmSolver = {
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
};

const mechanismConfiguration = {
	types,
	reactionTypes,
	Mechanism,
};

module.exports = {
	micmSolver,
	mechanismConfiguration,
	getVersion,
	getMicmVersion,
	initModule,
	hasWasm: WasmBuilt(),
};
