// MICM solver class

const path = require('path');
const State = require('./state');

// Load the native addon
const addon = require(path.join(__dirname, '../../build/Release/musica'));

// Solver type enum - matches musica::MICMSolver
const SolverType = {
    rosenbrock: 1,                      // Vector-ordered Rosenbrock solver
    rosenbrock_standard_order: 2,       // Standard-ordered Rosenbrock solver
    backward_euler: 3,                  // Vector-ordered BackwardEuler solver
    backward_euler_standard_order: 4,   // Standard-ordered BackwardEuler solver
};

/**
 * MICM (Modular Integrated Chemistry Module) solver
 * simplified state management
 */
class MICM {
    /**
     * Create a MICM solver instance
     *
     * @param {Object} options - Solver configuration
     * @param {string} options.config_path - Path to the mechanism configuration directory (mutually exclusive with mechanism)
     * @param {Object} options.mechanism - Mechanism object created in code (mutually exclusive with config_path)
     * @param {number} options.solver_type - Type of solver to use (from SolverType enum)
     */
    constructor({
        config_path = null,
        mechanism = null,
        solver_type = null,
    } = {}) {
        if (solver_type === null) {
            solver_type = SolverType.rosenbrock_standard_order;
        }

        if (config_path === null && mechanism === null) {
            throw new Error('Either config_path or mechanism must be provided');
        }

        if (config_path !== null && mechanism !== null) {
            throw new Error('Cannot provide both config_path and mechanism - choose one');
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

    /**
     * Get the solver type
     *
     * @returns {number} Solver type enum value
     */
    solverType() {
        return this._solverType;
    }

    /**
     * Create a State object for this solver
     * Simplified  - single state for all grid cells
     *
     * @param {number} numberOfGridCells - Number of grid cells to simulate
     * @returns {State} State object with single internal state
     */
    createState(numberOfGridCells = 1) {
        return new State(this._nativeMICM, numberOfGridCells);
    }

    /**
     * Solve the chemical system for one time step
     * msingle state
     *
     * @param {State} state - State object containing concentrations and conditions
     * @param {number} timeStep - Time step in seconds
     */
    solve(state, timeStep) {
        if (!(state instanceof State)) {
            throw new TypeError('state must be an instance of State');
        }
        if (typeof timeStep !== 'number') {
            throw new TypeError('timeStep must be a number');
        }

        // Solve single internal state
        this._nativeMICM.solve(state.getInternalState(), timeStep);
    }
}

module.exports = {
    MICM,
    SolverType,
};
