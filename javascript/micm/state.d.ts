/**
 * Chemical state for one or more grid cells.
 *
 * Holds species concentrations, environmental conditions, and user-defined
 * rate parameters. Create via {@link MICM#createState} rather than directly.
 */
export class State {
    /** @private */
    private constructor();
    _nativeState: any;
    _numberOfGridCells: any;
    _solverType: any;
    /**
     * Set species concentrations.
     *
     * @param {Object.<string, number|number[]>} concentrations - Map of species name to
     *   concentration(s) in mol m⁻³. For a single grid cell, values may be scalars.
     *   For multiple grid cells, provide arrays of length `numberOfGridCells`.
     */
    setConcentrations(concentrations: {
        [x: string]: number | number[];
    }): void;
    /**
     * Get species concentrations for all grid cells.
     *
     * @returns {Object.<string, number[]>} Map of species name to array of concentrations
     *   (one element per grid cell) in mol m⁻³.
     */
    getConcentrations(): {
        [x: string]: number[];
    };
    /**
     * Set user-defined rate parameters (e.g. photolysis rates or emission fluxes).
     *
     * @param {Object.<string, number|number[]>} params - Map of parameter name to value(s).
     *   For a single grid cell, values may be scalars. For multiple grid cells, provide
     *   arrays of length `numberOfGridCells`.
     */
    setUserDefinedRateParameters(params: {
        [x: string]: number | number[];
    }): void;
    /**
     * Get user-defined rate parameters for all grid cells.
     *
     * @returns {Object.<string, number[]>} Map of parameter name to array of values
     *   (one element per grid cell).
     */
    getUserDefinedRateParameters(): {
        [x: string]: number[];
    };
    /**
     * Set environmental conditions for each grid cell.
     *
     * All parameters accept a scalar (single grid cell) or an array of length
     * `numberOfGridCells`. If `airDensities` is omitted, air density is computed
     * from the Ideal Gas Law using the provided temperature and pressure.
     *
     * @param {Object} [options]
     * @param {number|number[]|null} [options.temperatures=null] - Temperature(s) in Kelvin
     * @param {number|number[]|null} [options.pressures=null] - Pressure(s) in Pascals
     * @param {number|number[]|null} [options.airDensities=null] - Air number density in mol m⁻³
     */
    setConditions({ temperatures, pressures, airDensities }?: {
        temperatures?: number | number[] | null | undefined;
        pressures?: number | number[] | null | undefined;
        airDensities?: number | number[] | null | undefined;
    } | undefined): void;
    /**
     * Get environmental conditions for all grid cells.
     *
     * @returns {Array<{temperature: number, pressure: number, air_density: number}>}
     *   One object per grid cell.
     */
    getConditions(): Array<{
        temperature: number;
        pressure: number;
        air_density: number;
    }>;
    /**
     * Get the number of grid cells in this state.
     * @returns {number}
     */
    getNumberOfGridCells(): number;
    /**
     * Free the underlying WASM object. Call when done with this instance.
     */
    delete(): void;
}
