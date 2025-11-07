// State class for MICM solver

const { isScalarNumber, GAS_CONSTANT } = require('./utils');

/**
 * State class that manages a single internal native state.
 * Mirrors Python's State class (state.py)
 */
class State {
    /**
     * Create a State object with a single internal native state.
     * Updated to match Python's simplified approach 
     *
     * @param {Object} solver - The native MICM solver instance
     * @param {number} numberOfGridCells - Number of grid cells to simulate
     */
    constructor(solver, numberOfGridCells) {
        if (numberOfGridCells < 1) {
            throw new Error('number_of_grid_cells must be greater than 0');
        }

        this._numberOfGridCells = numberOfGridCells;

        // Create single internal state for all grid cells 
        this._nativeState = solver.createState(numberOfGridCells);

        // Cache orderings
        this._speciesOrdering = this._nativeState.getSpeciesOrdering();
        this._userDefinedRateParametersOrdering = this._nativeState.getUserDefinedRateParametersOrdering();
    }

    /**
     * Set concentrations for species in the state.
     * Simplified to work with single state 
     *
     * @param {Object} concentrations - Dictionary of species names and their concentrations
     */
    setConcentrations(concentrations) {
        // Convert to format expected by native addon
        const formatted = {};
        for (const [name, value] of Object.entries(concentrations)) {
            formatted[name] = isScalarNumber(value) ? [value] : value;
        }

        // Set concentrations on single state
        this._nativeState.setConcentrations(formatted);
    }

    /**
     * Get concentrations for all species in the state.
     * Simplified to work with single state 
     *
     * @returns {Object} Dictionary of species names and their concentrations
     */
    getConcentrations() {
        return this._nativeState.getConcentrations();
    }

    /**
     * Set user-defined rate parameters.
     * Simplified to work with single state
     *
     * @param {Object} params - Dictionary of parameter names and their values
     */
    setUserDefinedRateParameters(params) {
        const formatted = {};
        for (const [name, value] of Object.entries(params)) {
            formatted[name] = isScalarNumber(value) ? [value] : value;
        }

        // Set parameters on single state
        this._nativeState.setUserDefinedRateParameters(formatted);
    }

    /**
     * Get user-defined rate parameters.
     * Simplified to work with single state
     *
     * @returns {Object} Dictionary of parameter names and their values
     */
    getUserDefinedRateParameters() {
        return this._nativeState.getUserDefinedRateParameters();
    }

    /**
     * Set environmental conditions.
     * Simplified to work with single state
     *
     * @param {Object} options - Condition parameters
     * @param {number|Array} options.temperatures - Temperature(s) in Kelvin
     * @param {number|Array} options.pressures - Pressure(s) in Pascals
     * @param {number|Array} options.air_densities - Air density/densities in mol m^-3
     */
    setConditions({
        temperatures = null,
        pressures = null,
        air_densities = null,
    } = {}) {
        const cond = {};

        if (temperatures !== null) {
            cond.temperatures = isScalarNumber(temperatures) ? [temperatures] : temperatures;
        }
        if (pressures !== null) {
            cond.pressures = isScalarNumber(pressures) ? [pressures] : pressures;
        }
        if (air_densities !== null) {
            cond.air_densities = isScalarNumber(air_densities) ? [air_densities] : air_densities;
        }

        // Set conditions on single state
        this._nativeState.setConditions(cond);
    }

    /**
     * Get environmental conditions.
     * Simplified to work with single state
     *
     * @returns {Object} Dictionary of condition names and their values
     */
    getConditions() {
        return this._nativeState.getConditions();
    }

    /**
     * Get species ordering.
     * Mirrors Python's get_species_ordering (state.py lines 221-230)
     *
     * @returns {Object} Dictionary of species names and their indices
     */
    getSpeciesOrdering() {
        return this._speciesOrdering;
    }

    /**
     * Get user-defined rate parameters ordering.
     * Mirrors Python's get_user_defined_rate_parameters_ordering (state.py lines 232-241)
     *
     * @returns {Object} Dictionary of parameter names and their indices
     */
    getUserDefinedRateParametersOrdering() {
        return this._userDefinedRateParametersOrdering;
    }

    /**
     * Get total number of grid cells.
     *
     * @returns {number} Number of grid cells
     */
    getNumberOfGridCells() {
        return this._numberOfGridCells;
    }

    /**
     * Get concentration strides for memory layout.
     *
     * @returns {Array} [cell_stride, species_stride]
     */
    concentrationStrides() {
        return this._nativeState.concentrationStrides();
    }

    /**
     * Get user-defined rate parameter strides for memory layout.
     *
     * @returns {Array} [cell_stride, param_stride]
     */
    userDefinedRateParameterStrides() {
        return this._nativeState.userDefinedRateParameterStrides();
    }

    /**
     * Get internal native state (for advanced use).
     *
     * @returns {Object} Native state object
     */
    getInternalState() {
        return this._nativeState;
    }
}

module.exports = State;
