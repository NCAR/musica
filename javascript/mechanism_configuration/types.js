const { convertOtherProperties } = require('./utils');

// =========== TYPES ===========
class Species {
	#keys = [
		'name',
		'molecular_weight',
		'constant_concentration',
		'constant_mixing_ratio',
		'is_third_body',
	];
	constructor(params) {
		this.name = params['name'];
		this.molecular_weight = params['molecular_weight'];
		this.constant_concentration = params['constant_concentration'];
		this.constant_mixing_ratio = params['constant_mixing_ratio'];
		this.is_third_body = params['is_third_body'];
		this.other_properties = {};
		// Allows end-users to add arbitrary properties as simple key-value pairs just like the other defined properties
		Object.entries(params).forEach(([key, value]) => {
			if (this.#keys.includes(key) == false) {
				this.other_properties[key] = value;
			}
		});
	}
	getJSON() {
		let obj = {};
		obj['name'] = this.name;
		obj['molecular weight [kg mol-1]'] = this.molecular_weight;
		obj['constant concentration [mol m-3]'] = this.constant_concentration;
		obj['constant mixing ratio [mol mol-1]'] = this.constant_mixing_ratio;
		obj['is third body'] = this.is_third_body;
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

class PhaseSpecies {
	#keys = ['name', 'diffusion_coefficient'];
	constructor(params) {
		this.name = params['name'];
		this.diffusion_coefficient = params['diffusion_coefficient'];
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
		obj['diffusion coefficient [m2 s-1]'] = this.diffusion_coefficient;
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

class Phase {
	#keys = ['name', 'species'];
	constructor(params) {
		this.name = params['name'];
		this.species = params['species'];
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
		obj['species'] = this.species.map((s) => {
			if (s instanceof PhaseSpecies) return s.getJSON();
			if (s instanceof Species) {
				const ps = new PhaseSpecies({ name: s.name });
				return ps.getJSON();
			}
		});
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

class ReactionComponent {
	#keys = ['species_name', 'coefficient'];
	constructor(params) {
		this.species_name = params['species_name'];
		this.coefficient = params['coefficient'] || 1.0;
		this.other_properties = {};
		Object.entries(params).forEach(([key, value]) => {
			if (this.#keys.includes(key) == false) {
				this.other_properties[key] = value;
			}
		});
	}
	getJSON() {
		let obj = {};
		obj['species name'] = this.species_name;
		obj['coefficient'] = this.coefficient;
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

const types = { Species, PhaseSpecies, Phase, ReactionComponent };

module.exports = { types };
