var addon = require('bindings')('musica-addon.node')


const { State } = require('./state.js');
const { SolverType } = require('./solver');
const { SolverStats, SolverResult } = require('./solver_result');

class MICM {
	/**
	 * Create a MICM solver instance from a configuration file path
	 * 
	 * @param {string} configPath - Path to the mechanism configuration directory
	 * @param {number} [solverType=SolverType.rosenbrock_standard_order] - Type of solver to use
	 * @returns {MICM} A new MICM instance
	 */
	static fromConfigPath(configPath, solverType = SolverType.rosenbrock_standard_order) {
		if (typeof configPath !== 'string') {
			throw new TypeError('configPath must be a string');
		}

		try {
			const nativeMICM = addon.MICM.fromConfigPath(configPath, solverType);
			return new MICM(nativeMICM, solverType);
		} catch (error) {
			throw new Error(`Failed to create MICM solver from config path: ${error.message}`);
		}
	}

	/**
	 * Create a MICM solver instance from a Mechanism object
	 * 
	 * @param {Object} mechanism - Mechanism object created in code
	 * @param {number} [solverType=SolverType.rosenbrock_standard_order] - Type of solver to use
	 * @returns {MICM} A new MICM instance
	 */
	static fromMechanism(mechanism, solverType = SolverType.rosenbrock_standard_order) {
		if (!mechanism || typeof mechanism.getJSON !== 'function') {
			throw new TypeError('mechanism must be a valid Mechanism object with getJSON() method');
		}

		try {
			// JavaScript Mechanism → JSON String → C++ Parser
			const mechanismJSON = mechanism.getJSON();
			const jsonString = JSON.stringify(mechanismJSON);
			
			const nativeMICM = addon.MICM.fromConfigString(jsonString, solverType);
			return new MICM(nativeMICM, solverType);
		} catch (error) {
			throw new Error(`Failed to create MICM solver from mechanism: ${error.message}`);
		}
	}

	/**
	 * Private constructor - use static factory methods instead
	 * @private
	 */
	constructor(nativeMICM, solverType) {
		this._nativeMICM = nativeMICM;
		this._solverType = solverType;
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

		const result = this._nativeMICM.solve(state._nativeState, timeStep);

		// Convert the plain object to a SolverResult instance
		const stats = new SolverStats(result.stats);
		return new SolverResult(result.state, stats);
	}
}
module.exports = { MICM };
