const path = require('path');
const addon = require(path.join(
	__dirname,
	'../../build/Release/musica-addon.node'
));

const { State } = require('./state.js');
const { SolverType } = require('./solver');

class MICM {
	constructor({ config_path = null, mechanism = null, solver_type = null } = {}) {
		if (solver_type === null) {
			solver_type = SolverType.rosenbrock_standard_order;
		}

		// Support both file-based (config_path) and in-code (mechanism) configurations
		if (config_path === null && mechanism === null) {
			throw new Error('Either config_path or mechanism must be provided');
		}

		if (config_path !== null && mechanism !== null) {
			throw new Error('Provide either config_path or mechanism, not both');
		}

		try {
			if (mechanism !== null) {
				// JavaScript Mechanism → JSON String → C++ Parser
				const mechanismJSON = mechanism.getJSON();
				const jsonString = JSON.stringify(mechanismJSON);

				// Pass JSON string to C++ with is_json_string flag
				this._nativeMICM = new addon.MICM(jsonString, solver_type, true);
			} else {
				// Traditional file-based configuration
				this._nativeMICM = new addon.MICM(config_path, solver_type, false);
			}

			this._solverType = solver_type;
		} catch (error) {
			throw new Error(`Failed to create MICM solver: ${error.message}`);
		}
	}

	solverType() {
		return this._solverType;
	}

	createState(numberOfGridCells = 1) {
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
