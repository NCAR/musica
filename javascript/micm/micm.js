const { State } = require('./state.js');
const { SolverType } = require('./solver');
const { SolverStats, SolverResult } = require('./solver_result');

// Try to load backend - will be null if not initialized
let backendModule = null;
let backendType = null;

// Lazy load the backend on first use
function getBackend() {
	if (!backendModule) {
		// Try Node.js addon first (synchronous)
		try {
			const addon = require('../load_addon');
			backendModule = addon;
			backendType = 'node';
		} catch (error) {
			throw new Error('Backend not available. For WASM, please initialize with "await MICM.initWasm()" first.');
		}
	}
	return { backend: backendModule, type: backendType };
}

class MICM {
	/**
	 * Initialize WASM backend (if available)
	 * Call this before using MICM with WASM
	 * @returns {Promise<void>}
	 */
	static async initWasm() {
		const wasm = require('../wasm/index.js');
		if (!wasm.hasWasm) {
			throw new Error('WASM module not built. Please run npm run build:wasm');
		}
		backendModule = await wasm.initModule();
		backendType = 'wasm';
	}

	/**
	 * Get the current backend type
	 * @returns {string|null} 'wasm' or 'node' or null
	 */
	static getBackendType() {
		return backendType;
	}

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
			const { backend, type } = getBackend();
			const nativeMICM = backend.MICM.fromConfigPath(configPath, solverType);
			return new MICM(nativeMICM, solverType, type);
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
			const { backend, type } = getBackend();
			// JavaScript Mechanism → JSON String → C++ Parser
			const mechanismJSON = mechanism.getJSON();
			const jsonString = JSON.stringify(mechanismJSON);
			
			const nativeMICM = backend.MICM.fromConfigString(jsonString, solverType);
			return new MICM(nativeMICM, solverType, type);
		} catch (error) {
			throw new Error(`Failed to create MICM solver from mechanism: ${error.message}`);
		}
	}

	/**
	 * Private constructor - use static factory methods instead
	 * @private
	 */
	constructor(nativeMICM, solverType, backendType) {
		this._nativeMICM = nativeMICM;
		this._solverType = solverType;
		this._backendType = backendType;
	}

	solverType() {
		return this._solverType;
	}

	createState(numberOfGridCells = 1) {
		if (numberOfGridCells <= 0) {
			throw new RangeError('number_of_grid_cells must be greater than 0');
		}
		const nativeState = this._nativeMICM.createState(numberOfGridCells);
		return new State(nativeState, this._backendType);
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
