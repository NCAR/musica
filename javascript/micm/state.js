const { isScalarNumber } = require('./utils');

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
		return this._nativeState.concentrationStrides();
	}

	userDefinedRateParameterStrides() {
		return this._nativeState.userDefinedRateParameterStrides();
	}
}
module.exports = { State };
