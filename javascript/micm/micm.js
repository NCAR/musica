import { State } from './state.js';
import { SolverType } from './solver.js';
import { SolverStats, SolverResult } from './solver_result.js';
import { getBackend } from '../backend.js';

export class MICM {
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
			const backend = getBackend();
			// In Node.js with NODEFS mounted, configuration files are exposed under /host.
			// In browser environments, this prefix is invalid, so only add it when running under Node.
			let resolvedConfigPath = configPath;
			const isNodeEnv =
				typeof process !== 'undefined' &&
				process.versions != null &&
				process.versions.node != null;
			if (isNodeEnv && !configPath.startsWith('/host/')) {
				resolvedConfigPath = `/host/${configPath}`;
			}
			const nativeMICM = backend.MICM.fromConfigPath(resolvedConfigPath, solverType);
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
			const backend = getBackend();
			const mechanismJSON = mechanism.getJSON();
			const jsonString = JSON.stringify(mechanismJSON);

			const nativeMICM = backend.MICM.fromConfigString(jsonString, solverType);
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
