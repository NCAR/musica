const { test } = require('node:test');
const assert = require('node:assert');
const musica = require('musica-addon');
const { types, reactionTypes, Mechanism } = musica.mechanismConfiguration;
const { Species, PhaseSpecies, Phase, ReactionComponent } = types;

// ===== Species =====
const A = new Species({
	name: 'A',
	molecular_weight: 0.04607,
	'absolute tolerance': 1.0e-30,
});
const B = new Species({ name: 'B', constant_concentration: 1.0e19 });
const C = new Species({ name: 'C', constant_mixing_ratio: 1.0e-20 });
const M = new Species({ name: 'M', is_third_body: true });
const H2O2 = new Species({
	name: 'H2O2',
	molecular_weight: 0.0340147,
	'absolute tolerance': 1.0e-10,
});
const ethanol = new Species({
	name: 'ethanol',
	molecular_weight: 0.04607,
	'absolute tolerance': 1.0e-20,
});
const H2O = new Species({ name: 'H2O', molecular_weight: 0.01801 });

// ===== Phases =====
const gas = new Phase({
	name: 'gas',
	species: [
		new PhaseSpecies({ name: A.name, diffusion_coefficient: 2.1e-5 }),
		B,
		C,
		new PhaseSpecies({ name: ethanol.name, diffusion_coefficient: 2.1e-5 }),
		new PhaseSpecies({ name: H2O2.name, diffusion_coefficient: 2.1e-5 }),
	],
});

// ===== Reactions =====
const my_emission = new reactionTypes.Emission({
	name: 'my emission',
	scaling_factor: 12.3,
	gas_phase: 'gas',
	products: [new ReactionComponent({ species_name: 'B' })],
});
const my_first_order_loss = new reactionTypes.FirstOrderLoss({
	name: 'my first order loss',
	scaling_factor: 12.3,
	gas_phase: 'gas',
	reactants: [new ReactionComponent({ species_name: 'C' })],
});
const my_photolysis = new reactionTypes.Photolysis({
	name: 'photo B',
	scaling_factor: 12.3,
	gas_phase: 'gas',
	reactants: [new ReactionComponent({ species_name: 'B' })],
	products: [new ReactionComponent({ species_name: 'C' })],
});
// BUG: gas_phase_species should be a ReactionComponent, but is a string in the full_config expected output
const my_surface = new reactionTypes.Surface({
	name: 'my surface',
	reaction_probability: 2.0e-2,
	gas_phase: 'gas',
	gas_phase_species: new ReactionComponent({ species_name: 'A' }),
	gas_phase_products: [
		new ReactionComponent({ species_name: 'B' }),
		new ReactionComponent({ species_name: 'C' }),
	],
});
const my_troe = new reactionTypes.Troe({
	name: 'my troe',
	k0_A: 1.2e-12,
	k0_B: 167,
	k0_C: 3,
	kinf_A: 136,
	kinf_B: 5,
	kinf_C: 24,
	Fc: 0.9,
	N: 0.8,
	gas_phase: 'gas',
	reactants: [
		new ReactionComponent({ species_name: 'B' }),
		new ReactionComponent({ species_name: 'M' }),
	],
	products: [new ReactionComponent({ species_name: 'C' })],
});
const my_tca = new reactionTypes.TernaryChemicalActivation({
	name: 'my ternary chemical activation',
	gas_phase: 'gas',
	k0_A: 32.1,
	k0_B: -2.3,
	k0_C: 102.3,
	kinf_A: 63.4,
	kinf_B: -1.3,
	kinf_C: 908.5,
	Fc: 1.3,
	N: 32.1,
	reactants: [
		new ReactionComponent({ species_name: 'bar' }),
		new ReactionComponent({ species_name: 'baz' }),
	],
	products: [
		new ReactionComponent({ species_name: 'bar', coefficient: 0.5 }),
		new ReactionComponent({ species_name: 'foo', coefficient: 0.3 }),
	],
});
const my_branched = new reactionTypes.Branched({
	name: 'my branched',
	X: 1.2e-4,
	Y: 167,
	a0: 0.15,
	n: 9,
	gas_phase: 'gas',
	reactants: [new ReactionComponent({ species_name: 'A' })],
	alkoxy_products: [new ReactionComponent({ species_name: 'B' })],
	nitrate_products: [new ReactionComponent({ species_name: 'C' })],
});
const my_tunneling = new reactionTypes.Tunneling({
	name: 'my tunneling',
	A: 123.45,
	B: 1200.0,
	C: 1.0e8,
	gas_phase: 'gas',
	reactants: [new ReactionComponent({ species_name: 'B' })],
	products: [new ReactionComponent({ species_name: 'C' })],
});
const my_arrhenius = new reactionTypes.Arrhenius({
	name: 'my arrhenius',
	A: 32.1,
	B: -2.3,
	C: 102.3,
	D: 63.4,
	E: -1.3,
	gas_phase: 'gas',
	reactants: [new ReactionComponent({ species_name: 'B' })],
	products: [new ReactionComponent({ species_name: 'C' })],
});
const my_arrhenius2 = new reactionTypes.Arrhenius({
	name: 'my other arrhenius',
	A: 29.3,
	B: -1.5,
	Ea: 101.2,
	D: 82.6,
	E: -0.98,
	gas_phase: 'gas',
	reactants: [new ReactionComponent({ species_name: 'A' })],
	products: [new ReactionComponent({ species_name: 'B', coefficient: 1.2 })],
});
const my_ud = new reactionTypes.UserDefined({
	name: 'my user defined',
	gas_phase: 'gas',
	scaling_factor: 12.3,
	reactants: [
		new ReactionComponent({ species_name: 'A' }),
		new ReactionComponent({ species_name: 'B' }),
	],
	products: [new ReactionComponent({ species_name: 'C', coefficient: 1.3 })],
});
const my_ts = new reactionTypes.TaylorSeries({
	name: 'my taylor series',
	gas_phase: 'gas',
	A: 12.3,
	B: -1.5,
	C: 1.0e-6,
	D: 340.0,
	E: 0.00032,
	taylor_coefficients: [1.0, 0.1, -0.01],
	reactants: [new ReactionComponent({ species_name: 'B' })],
	products: [new ReactionComponent({ species_name: 'C' })],
});

// ===== Mechanism =====
const full_config_mechanism = new Mechanism({
	name: 'Full Configuration',
	version: '1.0.0',
	species: [A, B, C, M, H2O2, ethanol, H2O],
	phases: [gas],
	reactions: [
		my_emission,
		my_first_order_loss,
		my_photolysis,
		my_surface,
		my_troe,
		my_tca,
		my_branched,
		my_tunneling,
		my_arrhenius,
		my_arrhenius2,
		my_ud,
		my_ts,
	],
});

const expected = {
	version: '1.0.0',
	name: 'Full Configuration',
	species: [
		{
			name: 'A',
			'molecular weight [kg mol-1]': 0.04607,
			'__absolute tolerance': 1.0e-30,
		},
		{
			name: 'B',
			'constant concentration [mol m-3]': 1.0e19,
		},
		{
			name: 'C',
			'constant mixing ratio [mol mol-1]': 1.0e-20,
		},
		{
			name: 'M',
			'is third body': true,
		},
		{
			name: 'H2O2',
			'molecular weight [kg mol-1]': 0.0340147,
			'__absolute tolerance': 1.0e-10,
		},
		{
			name: 'ethanol',
			'molecular weight [kg mol-1]': 0.04607,
			'__absolute tolerance': 1.0e-20,
		},
		{
			name: 'H2O',
			'molecular weight [kg mol-1]': 0.01801,
		},
	],
	phases: [
		{
			name: 'gas',
			species: [
				{
					name: 'A',
					'diffusion coefficient [m2 s-1]': 2.1e-5,
				},
				{ name: 'B' },
				{ name: 'C' },
				{
					name: 'ethanol',
					'diffusion coefficient [m2 s-1]': 2.1e-5,
				},
				{
					name: 'H2O2',
					'diffusion coefficient [m2 s-1]': 2.1e-5,
				},
			],
		},
	],
	reactions: [
		{
			type: 'EMISSION',
			name: 'my emission',
			'scaling factor': 12.3,
			'gas phase': 'gas',
			products: [
				{
					'species name': 'B',
					coefficient: 1,
				},
			],
		},
		{
			type: 'FIRST_ORDER_LOSS',
			name: 'my first order loss',
			'scaling factor': 12.3,
			'gas phase': 'gas',
			reactants: [
				{
					'species name': 'C',
					coefficient: 1,
				},
			],
		},
		{
			type: 'PHOTOLYSIS',
			name: 'photo B',
			'scaling factor': 12.3,
			'gas phase': 'gas',
			reactants: [
				{
					'species name': 'B',
					coefficient: 1,
				},
			],
			products: [
				{
					'species name': 'C',
					coefficient: 1,
				},
			],
		},
		{
			type: 'SURFACE',
			name: 'my surface',
			'reaction probability': 2.0e-2,
			'gas phase': 'gas',
			'gas-phase species': 'A',
			'gas-phase products': [
				{
					'species name': 'B',
					coefficient: 1,
				},
				{
					'species name': 'C',
					coefficient: 1,
				},
			],
		},
		{
			type: 'TROE',
			name: 'my troe',
			k0_A: 1.2e-12,
			k0_B: 167,
			k0_C: 3,
			kinf_A: 136,
			kinf_B: 5,
			kinf_C: 24,
			Fc: 0.9,
			N: 0.8,
			'gas phase': 'gas',
			reactants: [
				{
					'species name': 'B',
					coefficient: 1,
				},
				{
					'species name': 'M',
					coefficient: 1,
				},
			],
			products: [
				{
					'species name': 'C',
					coefficient: 1,
				},
			],
		},
		{
			type: 'TERNARY_CHEMICAL_ACTIVATION',
			name: 'my ternary chemical activation',
			'gas phase': 'gas',
			k0_A: 32.1,
			k0_B: -2.3,
			k0_C: 102.3,
			kinf_A: 63.4,
			kinf_B: -1.3,
			kinf_C: 908.5,
			Fc: 1.3,
			N: 32.1,
			reactants: [
				{
					'species name': 'bar',
					coefficient: 1,
				},
				{
					'species name': 'baz',
					coefficient: 1,
				},
			],
			products: [
				{
					'species name': 'bar',
					coefficient: 0.5,
				},
				{
					'species name': 'foo',
					coefficient: 0.3,
				},
			],
		},
		{
			type: 'BRANCHED_NO_RO2',
			name: 'my branched',
			X: 1.2e-4,
			Y: 167,
			a0: 0.15,
			n: 9,
			'gas phase': 'gas',
			reactants: [
				{
					'species name': 'A',
					coefficient: 1,
				},
			],
			'alkoxy products': [
				{
					'species name': 'B',
					coefficient: 1,
				},
			],
			'nitrate products': [
				{
					'species name': 'C',
					coefficient: 1,
				},
			],
		},
		{
			type: 'TUNNELING',
			name: 'my tunneling',
			A: 123.45,
			B: 1200.0,
			C: 1.0e8,
			'gas phase': 'gas',
			reactants: [
				{
					'species name': 'B',
					coefficient: 1,
				},
			],
			products: [
				{
					'species name': 'C',
					coefficient: 1,
				},
			],
		},
		{
			type: 'ARRHENIUS',
			name: 'my arrhenius',
			'gas phase': 'gas',
			A: 32.1,
			B: -2.3,
			C: 102.3,
			D: 63.4,
			E: -1.3,
			reactants: [
				{
					'species name': 'B',
					coefficient: 1,
				},
			],
			products: [
				{
					'species name': 'C',
					coefficient: 1,
				},
			],
		},
		{
			type: 'ARRHENIUS',
			name: 'my other arrhenius',
			A: 29.3,
			B: -1.5,
			Ea: 101.2,
			D: 82.6,
			E: -0.98,
			'gas phase': 'gas',
			reactants: [
				{
					'species name': 'A',
					coefficient: 1,
				},
			],
			products: [
				{
					'species name': 'B',
					coefficient: 1.2,
				},
			],
		},
		{
			type: 'USER_DEFINED',
			name: 'my user defined',
			'scaling factor': 12.3,
			'gas phase': 'gas',
			reactants: [
				{
					'species name': 'A',
					coefficient: 1,
				},
				{
					'species name': 'B',
					coefficient: 1,
				},
			],
			products: [
				{
					'species name': 'C',
					coefficient: 1.3,
				},
			],
		},
		{
			type: 'TAYLOR_SERIES',
			name: 'my taylor series',
			A: 12.3,
			B: -1.5,
			C: 1.0e-6,
			D: 340.0,
			E: 0.00032,
			'taylor coefficients': [1.0, 0.1, -0.01],
			'gas phase': 'gas',
			reactants: [
				{
					'species name': 'B',
					coefficient: 1,
				},
			],
			products: [
				{
					'species name': 'C',
					coefficient: 1,
				},
			],
		},
	],
};

/**
 * Recursively compare two objects (or arrays) for deep equality.
 * Arrays are compared as unordered collections.
 */
function deepEqual(a, b, path = '') {
	if (a === b) return true;
	if (typeof a !== typeof b) {
		return false;
	}
	if (a === null || b === null) {
		if (a !== b) {
			return false;
		}
		return true;
	}
	if (typeof a !== 'object') {
		return false;
	}

	// Arrays (unordered comparison)
	if (Array.isArray(a)) {
		if (!Array.isArray(b)) {
			return false;
		}
		if (a.length !== b.length) {
			return false;
		}
		// Track which elements in b have been matched
		const matched = new Array(b.length).fill(false);
		for (let i = 0; i < a.length; i++) {
			let found = false;
			for (let j = 0; j < b.length; j++) {
				if (!matched[j] && deepEqual(a[i], b[j], `${path}[${i}]`)) {
					matched[j] = true;
					found = true;
					break;
				}
			}
			if (!found) {
				console.log(`No match for element at ${path}[${i}]:`, a[i]);
				return false;
			}
		}
		return true;
	}

	// Objects
	const aKeys = Object.keys(a);
	const bKeys = Object.keys(b);
	if (aKeys.length !== bKeys.length) {
		return false;
	}
	for (const key of aKeys) {
		if (!b.hasOwnProperty(key)) {
			return false;
		}
		if (!deepEqual(a[key], b[key], path ? `${path}.${key}` : key))
			return false;
	}
	return true;
}

test('Full configuration export matches expected output', (t) => {
	const full_config_sanatized = JSON.parse(full_config_mechanism.getString());

	assert.ok(
		deepEqual(full_config_sanatized, expected, 'root'),
		'Full configuration should match expected output'
	);
});
