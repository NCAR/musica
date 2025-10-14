// JavaScript API for MUSICA - Compatibility layer over native addon
const path = require('path');
const addon = require(path.join(__dirname, '../build/Release/musica-addon.node'));

const AVOGADRO = 6.02214076e23;  // mol^-1
const BOLTZMANN = 1.380649e-23;  // J K^-1
const GAS_CONSTANT = AVOGADRO * BOLTZMANN;  // J K^-1 mol^-1

function isScalarNumber(x) {
    return (
        (typeof x === 'number' || x instanceof Number) &&
        !Number.isNaN(x) &&
        !(typeof x === 'boolean')
    );
}

// Solver type enum - matches musica::MICMSolver
const SolverType = {
    rosenbrock: 1,                      // Vector-ordered Rosenbrock solver
    rosenbrock_standard_order: 2,       // Standard-ordered Rosenbrock solver
    backward_euler: 3,                  // Vector-ordered BackwardEuler solver
    backward_euler_standard_order: 4,   // Standard-ordered BackwardEuler solver
    // cuda_rosenbrock: 5,                 // Cuda Rosenbrock solver (for later)
};

class Conditions {
    constructor({
        temperature = null,
        pressure = null,
        air_density = null,
    } = {}) {
        this.temperature = temperature;
        this.pressure = pressure;
        if (air_density !== null) {
            this.air_density = air_density;
        } else if (temperature !== null && pressure !== null) {
            this.air_density = 1.0 / ((GAS_CONSTANT * temperature) / pressure);
        } else {
            this.air_density = null;
        }
    }
}

class State {
    constructor(nativeState) {
        this._nativeState = nativeState;
    }

    setConcentrations(concentrations) {
        // Convert to format expected by native addon
        const formatted = {};
        for (const [name, value] of Object.entries(concentrations)) {
            formatted[name] = isScalarNumber(value) ? [value] : value;
        }
        this._nativeState.setConcentrations(formatted);
    }

    getConcentrations() {
        return this._nativeState.getConcentrations();
    }

    setUserDefinedRateParameters(params) {
        const formatted = {};
        for (const [name, value] of Object.entries(params)) {
            formatted[name] = isScalarNumber(value) ? [value] : value;
        }
        this._nativeState.setUserDefinedRateParameters(formatted);
    }

    getUserDefinedRateParameters() {
        return this._nativeState.getUserDefinedRateParameters();
    }

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

        this._nativeState.setConditions(cond);
    }

    getConditions() {
        return this._nativeState.getConditions();
    }

    getSpeciesOrdering() {
        return this._nativeState.getSpeciesOrdering();
    }

    getUserDefinedRateParametersOrdering() {
        return this._nativeState.getUserDefinedRateParametersOrdering();
    }

    getNumberOfGridCells() {
        return this._nativeState.getNumberOfGridCells();
    }

    concentrationStrides() {
        return this._nativeState.concentrationStrides();
    }

    userDefinedRateParameterStrides() {
        return this._nativeState.userDefinedRateParameterStrides();
    }
}

class MICM {
    constructor({
        config_path = null,
        solver_type = null,
    } = {}) {
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

module.exports = {
    MICM,
    State,
    Conditions,
    SolverType,
    GAS_CONSTANT,
    AVOGADRO,
    BOLTZMANN,
};
