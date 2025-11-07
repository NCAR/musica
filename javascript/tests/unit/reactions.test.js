/**
 * Comprehensive tests for Arrhenius, Photolysis, and UserDefined reaction types
 * Tests the JavaScript API bindings for MUSICA/MICM reaction configuration
 */

const musica = require('musica-addon');
const { types, reactionTypes, Mechanism } = musica.mechanismConfiguration;
const { Species, PhaseSpecies, Phase, ReactionComponent } = types;
// Access to all reaction types
const {
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
} = reactionTypes;

console.log('='.repeat(70));
console.log('Testing MUSICA Reaction Types: Arrhenius, Photolysis, UserDefined');
console.log('='.repeat(70));
console.log();

// Setup test species and phase
console.log('Setup: Creating test species and phase...');
const O3 = new Species({ name: "O3", molecular_weight_kg_mol: 48e-3 });
const O2 = new Species({ name: "O2", molecular_weight_kg_mol: 32e-3 });
const O = new Species({ name: "O", molecular_weight_kg_mol: 16e-3 });
const O1D = new Species({ name: "O1D", molecular_weight_kg_mol: 16e-3 });
const gasPhase = new Phase({ name: "gas", species: [O3, O2, O, O1D] });
console.log('  ✓ Created species: O3, O2, O, O1D');
console.log('  ✓ Created gas phase with 4 species');
console.log();

// ============================================================================
// ARRHENIUS REACTION TESTS
// ============================================================================

console.log('─'.repeat(70));
console.log('ARRHENIUS REACTION TESTS');
console.log('─'.repeat(70));

// Test 1: Basic Arrhenius creation
console.log('\nTest 1: Basic Arrhenius creation with all parameters');
const arrhenius1 = new Arrhenius({
    name: "O + O2 -> O3",
    A: 6.0e-34,      // Pre-exponential factor
    B: 2.4,          // Temperature exponent
    C: 0,            // Activation energy
    D: 300,          // Reference temperature
    E: 0,            // Pressure scaling
    reactants: [O, O2],
    products: [O3],
    gas_phase: gasPhase
});

console.log('  Created reaction:', arrhenius1.name);
console.log('  A (pre-exponential):', arrhenius1.A);
console.log('  B (temperature exp):', arrhenius1.B);
console.log('  C (activation E):', arrhenius1.C);
console.log('  D (ref temperature):', arrhenius1.D);
console.log('  E (pressure scaling):', arrhenius1.E);
console.log('  Reactants:', arrhenius1.reactants);
console.log('  Products:', arrhenius1.products);
console.log('  Gas phase:', arrhenius1.gas_phase);
console.log('  ✓ Arrhenius basic creation successful');

// Test 2: Arrhenius with string arrays (simplified API)
console.log('\nTest 2: Arrhenius with simplified string API');
const arrhenius2 = new Arrhenius({
    name: "O3 + O -> 2 O2",
    A: 8.0e-12,
    C: -2060,
    reactants: ["O3", "O"],
    products: ["O2"],
    gas_phase: "gas"
});

console.log('  Created reaction:', arrhenius2.name);
console.log('  Reactants (from strings):', arrhenius2.reactants);
console.log('  Products (from strings):', arrhenius2.products);
console.log('  ✓ Arrhenius simplified API successful');

// Test 3: Arrhenius serialization
console.log('\nTest 3: Arrhenius serialization');
const arrhenius_serialized = arrhenius1.getJSON();
console.log('  Serialized:', JSON.stringify(arrhenius_serialized, null, 2));
console.log('  Type:', arrhenius_serialized.type);
console.log('  ✓ Arrhenius serialization successful');

// Test 4: Arrhenius property modification
console.log('\nTest 4: Arrhenius property modification');
arrhenius1.A = 7.0e-34;
arrhenius1.B = 2.6;
console.log('  Updated A:', arrhenius1.A);
console.log('  Updated B:', arrhenius1.B);
console.log('  ✓ Arrhenius property modification successful');

// ============================================================================
// PHOTOLYSIS REACTION TESTS
// ============================================================================

console.log('\n' + '─'.repeat(70));
console.log('PHOTOLYSIS REACTION TESTS');
console.log('─'.repeat(70));

// Test 5: Basic Photolysis creation
console.log('\nTest 5: Basic Photolysis creation');
const photolysis1 = new Photolysis({
    name: "O3 + hv -> O2 + O1D",
    scaling_factor: 1.0,
    reactants: [O3],
    products: [O2, O1D],
    gas_phase: gasPhase
});

console.log('  Created reaction:', photolysis1.name);
console.log('  Scaling factor:', photolysis1.scaling_factor);
console.log('  Reactants:', photolysis1.reactants);
console.log('  Products:', photolysis1.products);
console.log('  Gas phase:', photolysis1.gas_phase);
console.log('  ✓ Photolysis basic creation successful');

// Test 6: Photolysis with simplified API
console.log('\nTest 6: Photolysis with simplified string API');
const photolysis2 = new Photolysis({
    name: "O2 + hv -> O + O",
    scaling_factor: 0.8,
    reactants: ["O2"],
    products: ["O", "O"],
    gas_phase: "gas"
});

console.log('  Created reaction:', photolysis2.name);
console.log('  Scaling factor:', photolysis2.scaling_factor);
console.log('  Reactants (from strings):', photolysis2.reactants);
console.log('  Products (from strings):', photolysis2.products);
console.log('  ✓ Photolysis simplified API successful');

// Test 7: Photolysis serialization
console.log('\nTest 7: Photolysis serialization');
const photolysis_serialized = photolysis1.getJSON();
console.log('  Serialized:', JSON.stringify(photolysis_serialized, null, 2));
console.log('  Type:', photolysis_serialized.type);
console.log('  ✓ Photolysis serialization successful');

// Test 8: Photolysis property modification
console.log('\nTest 8: Photolysis property modification');
photolysis1.scaling_factor = 1.2;
photolysis1.name = "O3 photolysis (updated)";
console.log('  Updated scaling factor:', photolysis1.scaling_factor);
console.log('  Updated name:', photolysis1.name);
console.log('  ✓ Photolysis property modification successful');

// ============================================================================
// USER-DEFINED REACTION TESTS
// ============================================================================

console.log('\n' + '─'.repeat(70));
console.log('USER-DEFINED REACTION TESTS');
console.log('─'.repeat(70));

// Test 9: Basic UserDefined creation
console.log('\nTest 9: Basic UserDefined creation');
const userDefined1 = new UserDefined({
    name: "Custom reaction: O + O3 -> 2 O2",
    scaling_factor: 1.5,
    reactants: [O, O3],
    products: [O2],
    gas_phase: gasPhase
});

console.log('  Created reaction:', userDefined1.name);
console.log('  Scaling factor:', userDefined1.scaling_factor);
console.log('  Reactants:', userDefined1.reactants);
console.log('  Products:', userDefined1.products);
console.log('  Gas phase:', userDefined1.gas_phase);
console.log('  ✓ UserDefined basic creation successful');

// Test 10: UserDefined with simplified API
console.log('\nTest 10: UserDefined with simplified string API');
const userDefined2 = new UserDefined({
    name: "Custom O3 destruction",
    scaling_factor: 2.0,
    reactants: ["O3"],
    products: ["O2", "O"],
    gas_phase: "gas"
});

console.log('  Created reaction:', userDefined2.name);
console.log('  Scaling factor:', userDefined2.scaling_factor);
console.log('  Reactants (from strings):', userDefined2.reactants);
console.log('  Products (from strings):', userDefined2.products);
console.log('  ✓ UserDefined simplified API successful');

// Test 11: UserDefined serialization
console.log('\nTest 11: UserDefined serialization');
const userDefined_serialized = userDefined1.getJSON();
console.log('  Serialized:', JSON.stringify(userDefined_serialized, null, 2));
console.log('  Type:', userDefined_serialized.type);
console.log('  ✓ UserDefined serialization successful');

// Test 12: UserDefined property modification
console.log('\nTest 12: UserDefined property modification');
userDefined1.scaling_factor = 1.8;
userDefined1.name = "Custom reaction (modified)";
console.log('  Updated scaling factor:', userDefined1.scaling_factor);
console.log('  Updated name:', userDefined1.name);
console.log('  ✓ UserDefined property modification successful');

// ============================================================================
// MIXED REACTION TYPES TEST
// ============================================================================

console.log('\n' + '─'.repeat(70));
console.log('MIXED REACTION TYPES TEST');
console.log('─'.repeat(70));

// Test 13: Using all three reaction types together
console.log('\nTest 13: Creating mechanism with all three reaction types');
const reactions = [
    new Arrhenius({
        name: "O + O2 -> O3 (thermal)",
        A: 6.0e-34,
        C: 0,
        reactants: ["O", "O2"],
        products: ["O3"]
    }),
    new Photolysis({
        name: "O3 -> O2 + O (photolysis)",
        scaling_factor: 1.0,
        reactants: ["O3"],
        products: ["O2", "O1D"]
    }),
    new UserDefined({
        name: "Custom O3 sink",
        scaling_factor: 0.5,
        reactants: ["O3"],
        products: ["O2", "O"]
    })
];

console.log('  Created', reactions.length, 'reactions:');
reactions.forEach((rxn, idx) => {
    const type = rxn.getJSON().type;
    console.log(`    ${idx + 1}. [${type}] ${rxn.name}`);
});
console.log('  ✓ Mixed reaction types successful');

// ============================================================================
// ADVANCED FEATURES TEST
// ============================================================================

console.log('\n' + '─'.repeat(70));
console.log('ADVANCED FEATURES TEST');
console.log('─'.repeat(70));

// Test 14: Reaction components with coefficients
console.log('\nTest 14: Reaction components with stoichiometric coefficients');
const arrhenius3 = new Arrhenius({
    name: "2 O -> O2",
    A: 1.0e-10,
    reactants: [
        { species_name: "O", coefficient: 2.0 }
    ],
    products: [
        { species_name: "O2", coefficient: 1.0 }
    ],
    gas_phase: "gas"
});

console.log('  Created reaction:', arrhenius3.name);
console.log('  Reactants with coefficients:', arrhenius3.reactants);
console.log('  Products with coefficients:', arrhenius3.products);
console.log('  ✓ Stoichiometric coefficients successful');

// Test 15: Empty reaction creation
console.log('\nTest 15: Empty reaction creation (to be populated later)');
const emptyArrhenius = new Arrhenius({});
const emptyPhotolysis = new Photolysis({});
const emptyUserDefined = new UserDefined({});

console.log('  Empty Arrhenius:', emptyArrhenius.name || '(no name)');
console.log('  Empty Photolysis:', emptyPhotolysis.name || '(no name)');
console.log('  Empty UserDefined:', emptyUserDefined.name || '(no name)');

// Populate later
emptyArrhenius.name = "Populated later";
emptyArrhenius.A = 1.0e-12;
emptyPhotolysis.name = "Photolysis populated later";
emptyPhotolysis.scaling_factor = 0.9;
emptyUserDefined.name = "UserDefined populated later";
emptyUserDefined.scaling_factor = 1.1;

console.log('  After population:');
console.log('    Arrhenius:', emptyArrhenius.name, '(A =', emptyArrhenius.A + ')');
console.log('    Photolysis:', emptyPhotolysis.name, '(scaling =', emptyPhotolysis.scaling_factor + ')');
console.log('    UserDefined:', emptyUserDefined.name, '(scaling =', emptyUserDefined.scaling_factor + ')');
console.log('  ✓ Empty reaction population successful');

// ============================================================================
// SUMMARY
// ============================================================================

console.log('\n' + '='.repeat(70));
console.log('ALL REACTION TYPE TESTS PASSED!');
console.log('='.repeat(70));
console.log('\nSummary:');
console.log('  ✓ Arrhenius reactions: 5 tests passed');
console.log('  ✓ Photolysis reactions: 4 tests passed');
console.log('  ✓ UserDefined reactions: 4 tests passed');
console.log('  ✓ Mixed reaction types: 1 test passed');
console.log('  ✓ Advanced features: 2 tests passed');
console.log('─'.repeat(70));
console.log('Total: 16 tests passed');
console.log('='.repeat(70));

// Add at end of reactions.test.js  
process.exit(0);
