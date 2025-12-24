// Load the appropriate backend
let backend = null;
let backendType = null;

// Try to load Node.js addon first (sync, always available if built)
try {
	backend = require('./load_addon');
	backendType = 'node';
} catch (error) {
	// Node addon not available, will use WASM if initialized
	console.warn('Node.js addon not available. WASM backend can be initialized via "await MICM.initWasm()"');
}

const { GAS_CONSTANT, AVOGADRO, BOLTZMANN } = require('./micm/utils.js');
const { SolverType } = require('./micm/solver.js');
const { SolverState, SolverStats, SolverResult } = require('./micm/solver_result.js');
const { Conditions } = require('./micm/conditions.js');
const { MICM } = require('./micm/micm.js');
const { State } = require('./micm/state.js');
const { types } = require('./mechanism_configuration/types.js');
const { reactionTypes } = require('./mechanism_configuration/reaction_types.js');
const { Mechanism } = require('./mechanism_configuration/mechanism.js');

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

// Unified API for version functions that works with both backends
// Node.js addon provides sync functions, so we keep them sync
// For WASM, users should use the async versions from the wasm module directly
const api = {
	micmSolver,
	mechanismConfiguration,
};

// Add version functions if Node.js addon is available
if (backend) {
	api.getVersion = backend.getVersion;
	api.getMicmVersion = backend.getMicmVersion;
}

module.exports = api;
