const { State } = require('./state.js');
const { SolverType } = require('./solver');
const { SolverStats, SolverResult } = require('./solver_result');

// Try to load backend - will be null if not initialized
let backendModule = null;
let backendType = null;

// Lazy load the backend on first use
function getBackend() {
	if (backendModule) {
		return { backend: backendModule, type: backendType };
	}

	// Prefer the Node.js addon when available (synchronous path)
	try {
		const addon = require('../load_addon');
		backendModule = addon;
		backendType = 'node';
		return { backend: backendModule, type: backendType };
	} catch (err) {
		// Node addon not available. If WASM has been initialized earlier,
		// it will have populated `backendModule` — check and return if so.
		if (backendModule && backendType === 'wasm') {
			return { backend: backendModule, type: backendType };
		}

		// If a WASM package exists but hasn't been initialized, provide
		// an actionable error telling the caller to initialize it.
		try {
			const wasm = require('../wasm/index.js');
			if (wasm && wasm.hasWasm) {
				throw new Error('Node addon not available. For WASM, call "await MICM.initWasm()" before using MICM.');
			}
		} catch (e) {
			// wasm package not present or not built — fall through to generic error
		}

		throw new Error('No usable backend found. Build the Node addon or run `await MICM.initWasm()` to initialize the WASM backend.');
	}
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
