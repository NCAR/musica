var addon = require('bindings')('musica-addon.node');
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

Object.assign(addon, { micmSolver, mechanismConfiguration });
module.exports = addon;
