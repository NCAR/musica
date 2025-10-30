const addon = require('../build/Release/musica-addon.node');
const { GAS_CONSTANT, AVOGADRO, BOLTZMANN } = require('./micm/utils.js');
const { SolverType } = require('./micm/solver.js');
const { Conditions } = require('./micm/conditions.js');
const { MICM } = require('./micm/micm.js');
const { State } = require('./micm/state.js');

const micmSolver = {
	MICM,
	State,
	Conditions,
	SolverType,
	GAS_CONSTANT,
	AVOGADRO,
	BOLTZMANN,
};

Object.assign(addon, { micmSolver });
module.exports = addon;
