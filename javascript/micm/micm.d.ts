export class MICM {
    /**
     * Create a MICM solver instance from a configuration file path
     *
     * @param {string} configPath - Path to the mechanism configuration directory
     * @param {number} [solverType=SolverType.rosenbrock_standard_order] - Type of solver to use
     * @param {RosenbrockSolverParameters|BackwardEulerSolverParameters} [solverParameters] - Optional solver parameters
     * @returns {MICM} A new MICM instance
     */
    static fromConfigPath(configPath: string, solverType?: number, solverParameters?: RosenbrockSolverParameters | BackwardEulerSolverParameters): MICM;
    /**
     * Create a MICM solver instance from a Mechanism object
     *
     * @param {Object} mechanism - Mechanism object created in code
     * @param {number} [solverType=SolverType.rosenbrock_standard_order] - Type of solver to use
     * @param {RosenbrockSolverParameters|BackwardEulerSolverParameters} [solverParameters] - Optional solver parameters
     * @returns {MICM} A new MICM instance
     */
    static fromMechanism(mechanism: Object, solverType?: number, solverParameters?: RosenbrockSolverParameters | BackwardEulerSolverParameters): MICM;
    /**
     * Private constructor - use static factory methods instead
     * @private
     */
    private constructor();
    _nativeMICM: any;
    _solverType: any;
    /**
     * Get the solver type this instance was created with.
     * @returns {number} A {@link SolverType} value
     */
    solverType(): number;
    /**
     * Create a new {@link State} object for this solver.
     *
     * @param {number} [numberOfGridCells=1] - Number of independent atmospheric columns
     * @returns {State}
     */
    createState(numberOfGridCells?: number): State;
    /**
     * Integrate the chemical system forward by `timeStep` seconds.
     * Species concentrations in `state` are updated in-place.
     *
     * @param {State} state - The chemical state to integrate
     * @param {number} timeStep - Time step in seconds
     * @returns {SolverResult}
     */
    solve(state: State, timeStep: number): SolverResult;
    /**
     * Set solver-specific parameters.
     *
     * @param {RosenbrockSolverParameters|BackwardEulerSolverParameters} params
     */
    setSolverParameters(params: RosenbrockSolverParameters | BackwardEulerSolverParameters): void;
    /**
     * Register a callback for a user-defined (lambda) reaction rate.
     *
     * The callback is invoked by the solver at each time step to obtain the
     * current rate constant for the named reaction.
     *
     * @param {string} label - Reaction label as defined in the mechanism configuration
     * @param {Function} fn - Callback returning the rate constant (s⁻¹ or appropriate units)
     */
    setReactionRateCallback(label: string, fn: Function): void;
    /**
     * Get the current solver parameters.
     *
     * @returns {RosenbrockSolverParameters|BackwardEulerSolverParameters}
     */
    getSolverParameters(): RosenbrockSolverParameters | BackwardEulerSolverParameters;
    /**
     * Free the underlying WASM object. Call when done with this instance.
     */
    delete(): void;
}
import { State } from './state.js';
import { SolverResult } from './solver_result.js';
import { RosenbrockSolverParameters } from './solver_parameters.js';
import { BackwardEulerSolverParameters } from './solver_parameters.js';
