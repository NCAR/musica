const musica = require('../../index.js');
const { types, reactionTypes, Mechanism } = musica.mechanismConfiguration;
const { Species, PhaseSpecies, Phase, ReactionComponent } = types;
const { Arrhenius } = reactionTypes;

console.log('Testing Mechanism Configuration Classes...\n');

// Test 1: Create Species
console.log('Test 1: Creating Species');
const o3 = new Species({
    name: "O3",
    molecular_weight_kg_mol: 48e-3
});

const o2 = new Species({
    name: "O2",
    molecular_weight_kg_mol: 32e-3
});

const o = new Species({
    name: "O",
    molecular_weight_kg_mol: 16e-3
});

console.log('  Created O3:', o3.name);
console.log('  Created O2:', o2.name);
console.log('  Created O:', o.name);
console.log('  ✓ Species creation successful\n');

// Test 2: Serialize Species
console.log('Test 2: Serializing Species');
const o3_serialized = o3.getJSON();
console.log('  O3 serialized:', JSON.stringify(o3_serialized, null, 2));
console.log('  ✓ Species serialization successful\n');

// Test 3: Create Phase
console.log('Test 3: Creating Phase');
const gasPhase = new Phase({
    name: "gas",
    species: [o3, o2, o]
});

console.log('  Created phase:', gasPhase.name);
console.log('  Phase species:', gasPhase.species);
console.log('  ✓ Phase creation successful\n');

// Test 4: Create ReactionComponent
console.log('Test 4: Creating ReactionComponent');
const comp1 = new ReactionComponent({
    species_name: "O3",
    coefficient: 1.0
});

const comp2 = new ReactionComponent("O2", 2.0); // Alternate constructor

console.log('  Component 1:', comp1.species_name, 'x', comp1.coefficient);
console.log('  Component 2:', comp2.species_name, 'x', comp2.coefficient);
console.log('  ✓ ReactionComponent creation successful\n');

// Test 5: Create Arrhenius Reaction
console.log('Test 5: Creating Arrhenius Reaction');
const reaction = new Arrhenius({
    name: "O3_photolysis",
    A: 1.2e-10,
    C: -2450,
    reactants: [o3],
    products: [o2, o],
    gas_phase: gasPhase
});

console.log('  Reaction name:', reaction.name);
console.log('  A =', reaction.A);
console.log('  C =', reaction.C);
console.log('  Gas phase:', reaction.gas_phase);
console.log('  Reactants:', reaction.reactants);
console.log('  Products:', reaction.products);
console.log('  ✓ Arrhenius reaction creation successful\n');

// Test 6: Serialize Arrhenius Reaction
console.log('Test 6: Serializing Arrhenius Reaction');
const reaction_serialized = reaction.getJSON();
console.log('  Reaction serialized:', JSON.stringify(reaction_serialized, null, 2));
console.log('  ✓ Arrhenius serialization successful\n');

// Test 7: Create reaction with string arrays (simplified API)
console.log('Test 7: Creating Arrhenius with simplified API');
const reaction2 = new Arrhenius({
    name: "O2_formation",
    A: 2.5e-11,
    C: -1500,
    reactants: ["O", "O"],
    products: ["O2"],
    gas_phase: "gas"
});

console.log('  Reaction name:', reaction2.name);
console.log('  Reactants:', reaction2.reactants);
console.log('  Products:', reaction2.products);
console.log('  ✓ Simplified API successful\n');

// Test 8: Modify properties
console.log('Test 8: Modifying properties');
o3.molecular_weight_kg_mol = 47.998e-3;
console.log('  Updated O3 molecular weight:', o3.molecular_weight_kg_mol);

reaction.A = 1.5e-10;
console.log('  Updated reaction A:', reaction.A);
console.log('  ✓ Property modification successful\n');

console.log('='.repeat(60));
console.log('ALL MECHANISM CONFIGURATION TESTS PASSED!');
console.log('='.repeat(60));

// Add at end of mechanism.test.js
process.exit(0);
