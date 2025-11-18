const path = require('path');
const addon = require(path.join(
	__dirname,
	'../../build/Debug/musica-addon.node'
));

const { State } = require('./state.js');
const { SolverType } = require('./solver');

class MICM {
	constructor({ config_path = null, solver_type = null } = {}) {
		if (solver_type === null) {
			solver_type = SolverType.rosenbrock_standard_order;
		}

		if (config_path === null) {
			throw new Error('config_path must be provided');
		}

		// Create native MICM instance
		this._nativeMICM = new addon.MICM(config_path, solver_type);
		this._solverType = solver_type;
	}

	solverType() {
		return this._solverType;
	}

	createState(numberOfGridCells = 1) {
		if (numberOfGridCells <= 0) {
			throw new RangeError('number_of_grid_cells must be greater than 0');
		}
		const nativeState = this._nativeMICM.createState(numberOfGridCells);
		return new State(nativeState);
	}

	solve(state, timeStep) {
		if (!(state instanceof State)) {
			throw new TypeError('state must be an instance of State');
		}
		if (typeof timeStep !== 'number') {
			throw new TypeError('timeStep must be a number');
		}

		this._nativeMICM.solve(state._nativeState, timeStep);
	}
}
module.exports = { MICM };
