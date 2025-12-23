const { isScalarNumber } = require('./utils');

class State {
	constructor(nativeState, backendType = 'node') {
		this._nativeState = nativeState;
		this._backendType = backendType;
	}

	setConcentrations(concentrations) {
		// Convert to format expected by native addon/wasm
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
			cond.temperatures = isScalarNumber(temperatures)
				? [temperatures]
				: temperatures;
		}
		if (pressures !== null) {
			cond.pressures = isScalarNumber(pressures)
				? [pressures]
				: pressures;
		}
		if (air_densities !== null) {
			cond.air_densities = isScalarNumber(air_densities)
				? [air_densities]
				: air_densities;
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
		const result = this._nativeState.concentrationStrides();
		// WASM returns an object, Node.js might return different format
		// Normalize to object format
		if (typeof result === 'object' && 'cell_stride' in result) {
			return result;
		}
		return result;
	}

	userDefinedRateParameterStrides() {
		const result = this._nativeState.userDefinedRateParameterStrides();
		// WASM returns an object, Node.js might return different format
		// Normalize to object format
		if (typeof result === 'object' && 'cell_stride' in result) {
			return result;
		}
		return result;
	}
}
module.exports = { State };
