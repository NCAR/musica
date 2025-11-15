const { convertOtherProperties } = require('./utils.js');

// ========== REACTION TYPES ==========
class Arrhenius {
	#keys = [
		'A',
		'B',
		'C',
		'D',
		'E',
		'Ea',
		'reactants',
		'products',
		'name',
		'gas_phase',
	];
	constructor(params) {
		this.A = params['A'] || 1.0;
		this.B = params['B'] || 0.0;
		this.C = params['C'] || 0.0;
		this.D = params['D'] || 300.0;
		this.E = params['E'] || 0.0;
		// C and Ea are mutually exclusive
		this.Ea = params['Ea'];
		this.reactants = params['reactants'];
		this.products = params['products'];
		this.name = params['name'];
		this.gas_phase = params['gas_phase'];
		this.other_properties = {};
		Object.entries(params).forEach(([key, value]) => {
			if (this.#keys.includes(key) == false) {
				this.other_properties[key] = value;
			}
		});
	}
	getJSON() {
		let obj = {};
		obj['type'] = 'ARRHENIUS';
		obj['name'] = this.name;
		obj['A'] = this.A;
		obj['B'] = this.B;
		if (this.Ea === undefined) obj['C'] = this.C;
		else obj['Ea'] = this.Ea;
		obj['D'] = this.D;
		obj['E'] = this.E;
		obj['gas phase'] = this.gas_phase;
		obj['reactants'] = this.reactants.map((r) => r.getJSON());
		obj['products'] = this.products.map((p) => p.getJSON());
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

class Branched {
	#keys = [
		'X',
		'Y',
		'a0',
		'n',
		'reactants',
		'nitrate_products',
		'alkoxy_products',
		'name',
		'gas_phase',
	];
	constructor(params) {
		this.X = params['X'];
		this.Y = params['Y'];
		this.a0 = params['a0'];
		this.n = params['n'];
		this.reactants = params['reactants'];
		this.nitrate_products = params['nitrate_products'];
		this.alkoxy_products = params['alkoxy_products'];
		this.name = params['name'];
		this.gas_phase = params['gas_phase'];
		this.other_properties = {};
		Object.entries(params).forEach(([key, value]) => {
			if (this.#keys.includes(key) == false) {
				this.other_properties[key] = value;
			}
		});
	}
	getJSON() {
		let obj = {};
		obj['type'] = 'BRANCHED_NO_RO2';
		obj['name'] = this.name;
		obj['X'] = this.X;
		obj['Y'] = this.Y;
		obj['a0'] = this.a0;
		obj['n'] = this.n;
		obj['gas phase'] = this.gas_phase;
		obj['reactants'] = this.reactants.map((r) => r.getJSON());
		obj['nitrate products'] = this.nitrate_products.map((p) => p.getJSON());
		obj['alkoxy products'] = this.alkoxy_products.map((p) => p.getJSON());
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

class Emission {
	#keys = ['scaling_factor', 'products', 'name', 'gas_phase'];
	constructor(params) {
		this.scaling_factor = params['scaling_factor'] || 1.0;
		this.products = params['products'];
		this.name = params['name'];
		this.gas_phase = params['gas_phase'];
		this.other_properties = {};
		Object.entries(params).forEach(([key, value]) => {
			if (this.#keys.includes(key) == false) {
				this.other_properties[key] = value;
			}
		});
	}
	getJSON() {
		let obj = {};
		obj['type'] = 'EMISSION';
		obj['name'] = this.name;
		obj['scaling factor'] = this.scaling_factor;
		obj['gas phase'] = this.gas_phase;
		obj['products'] = this.products.map((p) => p.getJSON());
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

class FirstOrderLoss {
	#keys = ['scaling_factor', 'reactants', 'name', 'gas_phase'];
	constructor(params) {
		this.scaling_factor = params['scaling_factor'] || 1.0;
		this.reactants = params['reactants'];
		this.name = params['name'];
		this.gas_phase = params['gas_phase'];
		this.other_properties = {};
		Object.entries(params).forEach(([key, value]) => {
			if (this.#keys.includes(key) == false) {
				this.other_properties[key] = value;
			}
		});
	}
	getJSON() {
		let obj = {};
		obj['type'] = 'FIRST_ORDER_LOSS';
		obj['name'] = this.name;
		obj['scaling factor'] = this.scaling_factor;
		obj['gas phase'] = this.gas_phase;
		obj['reactants'] = this.reactants.map((r) => r.getJSON());
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

class Photolysis {
	#keys = ['scaling_factor', 'reactants', 'products', 'name', 'gas_phase'];
	constructor(params) {
		this.scaling_factor = params['scaling_factor'] || 1.0;
		this.reactants = params['reactants'];
		this.products = params['products'];
		this.name = params['name'];
		this.gas_phase = params['gas_phase'];
		this.other_properties = {};
		Object.entries(params).forEach(([key, value]) => {
			if (this.#keys.includes(key) == false) {
				this.other_properties[key] = value;
			}
		});
	}
	getJSON() {
		let obj = {};
		obj['type'] = 'PHOTOLYSIS';
		obj['name'] = this.name;
		obj['scaling factor'] = this.scaling_factor;
		obj['gas phase'] = this.gas_phase;
		obj['reactants'] = this.reactants.map((r) => r.getJSON());
		obj['products'] = this.products.map((p) => p.getJSON());
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

class Surface {
	#keys = [
		'reaction_probability',
		'gas_phase_species',
		'gas_phase_products',
		'name',
		'gas_phase',
	];
	constructor(params) {
		this.reaction_probability = params['reaction_probability'] || 1.0;
		this.gas_phase_species = params['gas_phase_species'];
		this.gas_phase_products = params['gas_phase_products'];
		this.name = params['name'];
		this.gas_phase = params['gas_phase'];
		this.other_properties = {};
		Object.entries(params).forEach(([key, value]) => {
			if (this.#keys.includes(key) == false) {
				this.other_properties[key] = value;
			}
		});
	}
	getJSON() {
		let obj = {};
		obj['type'] = 'SURFACE';
		obj['name'] = this.name;
		obj['reaction probability'] = this.reaction_probability;
		obj['gas phase'] = this.gas_phase;
		// b/c Kyle said so ??
		obj['gas-phase species'] = this.gas_phase_species.species_name;
		obj['gas-phase products'] = this.gas_phase_products.map((p) =>
			p.getJSON()
		);
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

class TaylorSeries {
	#keys = [
		'A',
		'B',
		'C',
		'D',
		'E',
		'Ea',
		'taylor_coefficients',
		'reactants',
		'products',
		'name',
		'gas_phase',
	];
	constructor(params) {
		this.A = params['A'] || 1.0;
		this.B = params['B'] || 0.0;
		this.C = params['C'] || 0.0;
		this.D = params['D'] || 300.0;
		this.E = params['E'] || 0.0;
		// C and Ea are mutually exclusive
		this.Ea = params['Ea'];
		this.taylor_coefficients = params['taylor_coefficients'] || [1.0];
		this.reactants = params['reactants'];
		this.products = params['products'];
		this.name = params['name'];
		this.gas_phase = params['gas_phase'];
		this.other_properties = {};
		Object.entries(params).forEach(([key, value]) => {
			if (this.#keys.includes(key) == false) {
				this.other_properties[key] = value;
			}
		});
	}
	getJSON() {
		let obj = {};
		obj['type'] = 'TAYLOR_SERIES';
		obj['name'] = this.name;
		obj['A'] = this.A;
		obj['B'] = this.B;
		if (this.Ea === undefined) obj['C'] = this.C;
		else obj['Ea'] = this.Ea;
		obj['D'] = this.D;
		obj['E'] = this.E;
		obj['taylor coefficients'] = this.taylor_coefficients;
		obj['gas phase'] = this.gas_phase;
		obj['reactants'] = this.reactants.map((r) => r.getJSON());
		obj['products'] = this.products.map((p) => p.getJSON());
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

class Troe {
	#keys = [
		'k0_A',
		'k0_B',
		'k0_C',
		'kinf_A',
		'kinf_B',
		'kinf_C',
		'Fc',
		'N',
		'reactants',
		'products',
		'name',
		'gas_phase',
	];
	constructor(params) {
		this.k0_A = params['k0_A'] || 1.0;
		this.k0_B = params['k0_B'] || 0.0;
		this.k0_C = params['k0_C'] || 0.0;
		this.kinf_A = params['kinf_A'] || 1.0;
		this.kinf_B = params['kinf_B'] || 0.0;
		this.kinf_C = params['kinf_C'] || 0.0;
		this.Fc = params['Fc'] || 0.6;
		this.N = params['N'] || 1.0;
		this.reactants = params['reactants'];
		this.products = params['products'];
		this.name = params['name'];
		this.gas_phase = params['gas_phase'];
		this.other_properties = {};
		Object.entries(params).forEach(([key, value]) => {
			if (this.#keys.includes(key) == false) {
				this.other_properties[key] = value;
			}
		});
	}
	getJSON() {
		let obj = {};
		obj['name'] = this.name;
		obj['type'] = 'TROE';
		obj['k0_A'] = this.k0_A;
		obj['k0_B'] = this.k0_B;
		obj['k0_C'] = this.k0_C;
		obj['kinf_A'] = this.kinf_A;
		obj['kinf_B'] = this.kinf_B;
		obj['kinf_C'] = this.kinf_C;
		obj['Fc'] = this.Fc;
		obj['N'] = this.N;
		obj['gas phase'] = this.gas_phase;
		obj['reactants'] = this.reactants.map((r) => r.getJSON());
		obj['products'] = this.products.map((p) => p.getJSON());
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

class TernaryChemicalActivation {
	#keys = [
		'k0_A',
		'k0_B',
		'k0_C',
		'kinf_A',
		'kinf_B',
		'kinf_C',
		'Fc',
		'N',
		'reactants',
		'products',
		'name',
		'gas_phase',
	];
	constructor(params) {
		this.k0_A = params['k0_A'] || 1.0;
		this.k0_B = params['k0_B'] || 0.0;
		this.k0_C = params['k0_C'] || 0.0;
		this.kinf_A = params['kinf_A'] || 1.0;
		this.kinf_B = params['kinf_B'] || 0.0;
		this.kinf_C = params['kinf_C'] || 0.0;
		this.Fc = params['Fc'] || 0.6;
		this.N = params['N'] || 1.0;
		this.reactants = params['reactants'];
		this.products = params['products'];
		this.name = params['name'];
		this.gas_phase = params['gas_phase'];
		this.other_properties = {};
		Object.entries(params).forEach(([key, value]) => {
			if (this.#keys.includes(key) == false) {
				this.other_properties[key] = value;
			}
		});
	}
	getJSON() {
		let obj = {};
		obj['type'] = 'TERNARY_CHEMICAL_ACTIVATION';
		obj['name'] = this.name;
		obj['gas phase'] = this.gas_phase;
		obj['k0_A'] = this.k0_A;
		obj['k0_B'] = this.k0_B;
		obj['k0_C'] = this.k0_C;
		obj['kinf_A'] = this.kinf_A;
		obj['kinf_B'] = this.kinf_B;
		obj['kinf_C'] = this.kinf_C;
		obj['Fc'] = this.Fc;
		obj['N'] = this.N;
		obj['reactants'] = this.reactants.map((r) => r.getJSON());
		obj['products'] = this.products.map((p) => p.getJSON());
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

class Tunneling {
	#keys = ['A', 'B', 'C', 'reactants', 'products', 'name', 'gas_phase'];
	constructor(params) {
		this.A = params['A'] || 1.0;
		this.B = params['B'] || 0.0;
		this.C = params['C'] || 0.0;
		this.reactants = params['reactants'];
		this.products = params['products'];
		this.name = params['name'];
		this.gas_phase = params['gas_phase'];
		this.other_properties = {};
		Object.entries(params).forEach(([key, value]) => {
			if (this.#keys.includes(key) == false) {
				this.other_properties[key] = value;
			}
		});
	}
	getJSON() {
		let obj = {};
		obj['type'] = 'TUNNELING';
		obj['name'] = this.name;
		obj['A'] = this.A;
		obj['B'] = this.B;
		obj['C'] = this.C;
		obj['gas phase'] = this.gas_phase;
		obj['reactants'] = this.reactants.map((r) => r.getJSON());
		obj['products'] = this.products.map((p) => p.getJSON());
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

class UserDefined {
	#keys = ['scaling_factor', 'reactants', 'products', 'name', 'gas_phase'];
	constructor(params) {
		this.scaling_factor = params['scaling_factor'] || 1.0;
		this.reactants = params['reactants'];
		this.products = params['products'];
		this.name = params['name'];
		this.gas_phase = params['gas_phase'];
		this.other_properties = {};
		Object.entries(params).forEach(([key, value]) => {
			if (this.#keys.includes(key) == false) {
				this.other_properties[key] = value;
			}
		});
	}
	getJSON() {
		let obj = {};
		obj['type'] = 'USER_DEFINED';
		obj['name'] = this.name;
		obj['scaling factor'] = this.scaling_factor;
		obj['gas phase'] = this.gas_phase;
		obj['reactants'] = this.reactants.map((r) => r.getJSON());
		obj['products'] = this.products.map((p) => p.getJSON());
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

const reactionTypes = {
	Arrhenius,
	Branched,
	Emission,
	FirstOrderLoss,
	Photolysis,
	Surface,
	TaylorSeries,
	Troe,
	TernaryChemicalActivation,
	Tunneling,
	UserDefined,
};

module.exports = { reactionTypes };
