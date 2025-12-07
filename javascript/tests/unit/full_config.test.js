const musica = require('../../index.js');
const { types, reactionTypes, Mechanism } = musica.mechanismConfiguration;
const { Species, PhaseSpecies, Phase, ReactionComponent } = types;

// ===== Species =====
// A = new Species({ name: 'A', 'absolute tolerance': 1.0e-30 });
B = new Species({ name: 'B', constant_concentration: 1.0e19 });
C = new Species({ name: 'C', constant_mixing_ratio: 1.0e-20 });
M = new Species({ name: 'M', is_third_body: true });
H2O2 = new Species({ name: 'H2O2', molecular_weight: 0.0340147 });
ethanol = new Species({
	name: 'ethanol',
	molecular_weight: 0.04607,
	// 'absolute tolerance': 1.0e-20,
});
H2O = new Species({ name: 'H2O', molecular_weight: 0.01801 });

// ===== Phases =====
gas = new Phase({
	name: 'gas',
	species: [
		// A,
		B,
		C,
		new PhaseSpecies({ name: ethanol.name, diffusion_coefficient: 2.1e-5 }),
		new PhaseSpecies({ name: H2O2.name, diffusion_coefficient: 2.1e-5 }),
	],
});

// ===== Reactions =====
my_emission = new reactionTypes.Emission({
	name: 'my emission',
	scaling_factor: 12.3,
	gas_phase: 'gas',
	products: [new ReactionComponent({ species_name: 'B' })],
});
my_first_order_loss = new reactionTypes.FirstOrderLoss({
	name: 'my first order loss',
	scaling_factor: 12.3,
	gas_phase: 'gas',
	reactants: [new ReactionComponent({ species_name: 'C' })],
});
my_photolysis = new reactionTypes.Photolysis({
	name: 'my photolysis',
	scaling_factor: 12.3,
	gas_phase: 'gas',
	reactants: [new ReactionComponent({ species_name: 'B' })],
	products: [new ReactionComponent({ species_name: 'C' })],
});
my_surface = new reactionTypes.Surface({
	name: 'my surface',
	reaction_probability: 2.0e-2,
	gas_phase: 'gas',
	// gas_phase_species can be: string, ReactionComponent, or array
	// All convert to string in JSON output
	gas_phase_species: [new ReactionComponent({ species_name: 'A' })],
	gas_phase_products: [
		new ReactionComponent({ species_name: 'B' }),
		new ReactionComponent({ species_name: 'C' }),
	],
});
my_troe = new reactionTypes.Troe({
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
my_tca = new reactionTypes.TernaryChemicalActivation({
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
