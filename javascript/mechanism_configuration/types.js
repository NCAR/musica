const { convertOtherProperties } = require('./utils');

// =========== TYPES ===========
class Species {
	constructor({
		name,
		molecular_weight,
		constant_concentration,
		constant_mixing_ratio,
		/* REVIEW: in the full_config,
		 * the species don't have is_third_body if it's false
		 * So should I not initialize it?
		 */
		is_third_body = false,
		other_properties = {},
	}) {
		this.name = name;
		this.molecular_weight = molecular_weight;
		this.constant_concentration = constant_concentration;
		this.constant_mixing_ratio = constant_mixing_ratio;
		this.is_third_body = is_third_body;
		this.other_properties = other_properties;
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
	constructor({ name, diffusion_coefficient, other_properties = {} }) {
		this.name = name;
		this.diffusion_coefficient = diffusion_coefficient;
		this.other_properties = other_properties;
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

// REVIEW: Did I do this right?
class Phase {
	constructor({ name, species, other_properties = {} }) {
		this.name = name;
		this.species = species;
		this.other_properties = other_properties;
	}
	getJSON() {
		let obj = {};
		obj['name'] = this.name;
		obj['species'] = this.species.map((s) => {
			// Handle string names (v1 format)
			if (typeof s === 'string') return s;
			// Handle PhaseSpecies objects
			if (s instanceof PhaseSpecies) return s.getJSON();
			// Handle Species objects - convert to PhaseSpecies
			if (s instanceof Species) {
				return new PhaseSpecies({ name: s.name }).getJSON();
			}
			// Default: return as-is
			return s;
		});
		const ops = convertOtherProperties(this.other_properties);
		Object.assign(obj, ops);
		return obj;
	}
}

// Reaction component with species and coefficient
class ReactionComponent {
	constructor({ species_name, coefficient = 1.0, other_properties = {} }) {
		this.species_name = species_name;
		this.coefficient = coefficient;
		this.other_properties = other_properties;
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
