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
const { initModule, getBackend } = require('./backend.js');

/**
 * Get the MUSICA version
 * @returns {Promise<string>} The version string
 */
async function getVersion() {
	const backend = getBackend();
  return backend.getVersion();
}

/**
 * Get the MICM version
 * @returns {Promise<string>} The version string
 */
async function getMicmVersion() {
	const backend = getBackend();
  return backend.getMicmVersion();
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
	...micmSolver,
	mechanismConfiguration,
	initModule,
	getBackend,
	getVersion,
	getMicmVersion,
};
