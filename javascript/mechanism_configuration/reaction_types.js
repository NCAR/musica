const { convertOtherProperties } = require('./utils.js');

// ========== REACTION TYPES ==========
// REVIEW: Do you want these to all be in seprate files?
class Arrhenius {
	constructor({
		A = 1.0,
		B = 0.0,
		C = 0.0,
		D = 300.0,
		E = 0.0,
		Ea,
		reactants,
		products,
		name,
		gas_phase,
		// FIXME: parse for other properties //vexplain why I do what I do
		other_properties = {},
	}) {
		this.A = A;
		this.B = B;
		this.C = C;
		this.D = D;
		this.E = E;
		// C and Ea are mutually exclusive
		this.Ea = Ea;
		this.reactants = reactants;
		this.products = products;
		this.name = name;
		this.gas_phase = gas_phase;
		this.other_properties = other_properties;
	}
	getJSON() {
		let obj = {};
		obj['type'] = 'ARRHENIUS';
		obj['name'] = this.name;
		obj['A'] = this.A;
		obj['B'] = this.B;
		obj['C'] = this.C;
		obj['D'] = this.D;
		obj['E'] = this.E;
		obj['Ea'] = this.Ea;
		obj['gas phase'] = this.gas_phase;
		obj['reactants'] = this.reactants.map((r) => r.getJSON());
		obj['products'] = this.products.map((p) => p.getJSON());
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

class Branched {
	constructor({
		X,
		Y,
		a0,
		n,
		reactants,
		nitrate_products,
		alkoxy_products,
		name,
		gas_phase,
		other_properties = {},
	}) {
		this.X = X;
		this.Y = Y;
		this.a0 = a0;
		this.n = n;
		this.reactants = reactants;
		this.nitrate_products = nitrate_products;
		this.alkoxy_products = alkoxy_products;
		this.name = name;
		this.gas_phase = gas_phase;
		this.other_properties = other_properties;
	}
	getJSON() {
		let obj = {};
		// REVIEW: is this how to identify reaction types?
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
	constructor({
		scaling_factor = 1.0,
		products,
		name,
		gas_phase,
		other_properties = {},
	}) {
		this.scaling_factor = scaling_factor;
		this.products = products;
		this.name = name;
		this.gas_phase = gas_phase;
		this.other_properties = other_properties;
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
	constructor({
		scaling_factor = 1.0,
		reactants,
		name,
		gas_phase,
		other_properties = {},
	}) {
		this.scaling_factor = scaling_factor;
		this.reactants = reactants;
		this.name = name;
		this.gas_phase = gas_phase;
		this.other_properties = other_properties;
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
	constructor({
		scaling_factor = 1.0,
		reactants,
		products,
		name,
		gas_phase,
		other_properties = {},
	}) {
		this.scaling_factor = scaling_factor;
		this.reactants = reactants;
		this.products = products;
		this.name = name;
		this.gas_phase = gas_phase;
		this.other_properties = other_properties;
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
	constructor({
		reaction_probability = 1.0,
		gas_phase_species,
		gas_phase_products,
		name,
		gas_phase,
		other_properties = {},
	}) {
		this.reaction_probability = reaction_probability;
		this.gas_phase_species = gas_phase_species;
		this.gas_phase_products = gas_phase_products;
		this.name = name;
		this.gas_phase = gas_phase;
		this.other_properties = other_properties;
	}
	getJSON() {
		let obj = {};
		obj['type'] = 'SURFACE';
		obj['name'] = this.name;
		obj['reaction probability'] = this.reaction_probability;
		obj['gas phase'] = this.gas_phase;
		obj['gas-phase species'] = this.gas_phase_species.map((r) =>
			r.getJSON()
		);
		obj['gas-phase products'] = this.gas_phase_products.map((p) =>
			p.getJSON()
		);
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

class TaylorSeries {
	constructor({
		A = 1.0,
		B = 0.0,
		C = 0.0,
		D = 300.0,
		E = 0.0,
		Ea,
		taylor_coefficients = [1.0],
		reactants,
		products,
		name,
		gas_phase,
		other_properties = {},
	}) {
		this.A = A;
		this.B = B;
		this.C = C;
		this.D = D;
		this.E = E;
		this.Ea = Ea;
		this.taylor_coefficients = taylor_coefficients;
		this.reactants = reactants;
		this.products = products;
		this.name = name;
		this.gas_phase = gas_phase;
		this.other_properties = other_properties;
	}
	getJSON() {
		let obj = {};
		obj['type'] = 'TAYLOR_SERIES';
		obj['name'] = this.name;
		obj['A'] = this.A;
		obj['B'] = this.B;
		obj['C'] = this.C;
		obj['D'] = this.D;
		obj['E'] = this.E;
		obj['Ea'] = this.Ea;
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
	constructor({
		k0_A = 1.0,
		k0_B = 0.0,
		k0_C = 0.0,
		kinf_A = 1.0,
		kinf_B = 0.0,
		kinf_C = 0.0,
		Fc = 0.6,
		N = 1.0,
		reactants,
		products,
		name,
		gas_phase,
		other_properties = {},
	}) {
		this.k0_A = k0_A;
		this.k0_B = k0_B;
		this.k0_C = k0_C;
		this.kinf_A = kinf_A;
		this.kinf_B = kinf_B;
		this.kinf_C = kinf_C;
		this.Fc = Fc;
		this.N = N;
		this.reactants = reactants;
		this.products = products;
		this.name = name;
		this.gas_phase = gas_phase;
		this.other_properties = other_properties;
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
	constructor({
		k0_A = 1.0,
		k0_B = 0.0,
		k0_C = 0.0,
		kinf_A = 1.0,
		kinf_B = 0.0,
		kinf_C = 0.0,
		Fc = 0.6,
		N = 1.0,
		reactants,
		products,
		name,
		gas_phase,
		other_properties = {},
	}) {
		this.k0_A = k0_A;
		this.k0_B = k0_B;
		this.k0_C = k0_C;
		this.kinf_A = kinf_A;
		this.kinf_B = kinf_B;
		this.kinf_C = kinf_C;
		this.Fc = Fc;
		this.N = N;
		this.reactants = reactants;
		this.products = products;
		this.name = name;
		this.gas_phase = gas_phase;
		this.other_properties = other_properties;
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
	constructor({
		A = 1.0,
		B = 0.0,
		C = 0.0,
		reactants,
		products,
		name,
		gas_phase,
		other_properties = {},
	}) {
		this.A = A;
		this.B = B;
		this.C = C;
		this.reactants = reactants;
		this.products = products;
		this.name = name;
		this.gas_phase = gas_phase;
		this.other_properties = other_properties;
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
	constructor({
		scaling_factor = 1.0,
		reactants,
		products,
		name,
		gas_phase,
		other_properties = {},
	}) {
		this.scaling_factor = scaling_factor;
		this.reactants = reactants;
		this.products = products;
		this.name = name;
		this.gas_phase = gas_phase;
		this.other_properties = other_properties;
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

class Reactions {
	constructor({ reactions = [] }) {
		for (const reaction in reactions) {
			switch (typeof reaction) {
				case 'Arrhenius':
					this.arrhenius.push_back(reaction);
					break;
				case 'Branched':
					this.branched.push_back(reaction);
					break;
				case 'Emission':
					this.emission.push_back(reaction);
					break;
				case 'FirstOrderLoss':
					this.first_order_loss.push_back(reaction);
					break;
				case 'Photolysis':
					this.photolysis.push_back(reaction);
					break;
				case 'Surface':
					this.surface.push_back(reaction);
					break;
				case 'TaylorSeries':
					this.taylor_series.push_back(reaction);
					break;
				case 'Troe':
					this.troe.push_back(reaction);
					break;
				case 'TernaryChemicalActivation':
					this.ternary_chemical_activation.push_back(reaction);
					break;
				case 'Tunneling':
					this.tunneling.push_back(reaction);
					break;
				case 'UserDefined':
					this.user_defined.push_back(reaction);
					break;
			}
		}
	}
	getJSON() {
		let obj = {};
		for (const a in this.arrhenius) Object.assign(obj, a.getJSON());
		for (const b in this.branched) Object.assign(obj, b.getJSON());
		for (const e in this.emission) Object.assign(obj, e.getJSON());
		for (const fol in this.first_order_loss)
			Object.assign(obj, fol.getJSON());
		for (const p in this.photolysis) Object.assign(obj, p.getJSON());
		for (const s in this.surface) Object.assign(obj, s.getJSON());
		for (const ts in this.taylor_series) Object.assign(obj, ts.getJSON());
		for (const t in this.troe) Object.assign(obj, t.getJSON());
		for (const tca in this.ternary_chemical_activation)
			Object.assign(obj, tca.getJSON());
		for (const tu in this.tunneling) Object.assign(obj, tu.getJSON());
		for (const ud in this.user_defined) Object.assign(obj, ud.getJSON());
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
	Reactions,
};

module.exports = { reactionTypes };
