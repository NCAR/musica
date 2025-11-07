// Mechanism API integration test
// Demonstrates creating a chemical mechanism entirely in code using the JavaScript API

const musica = require('musica-addon');
const { MICM, SolverType } = musica.micmSolver;
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


function testMechanismAPISimpleChemistry() {
    console.log('\n============================================================');
    console.log('Mechanism API Test - Simple Chemistry');
    console.log('============================================================\n');

    console.log('Creating mechanism components in code...');

    // Create species
    // Note: Despite parameter name "molecular_weight_kg_mol", Python uses g/mol values (API inconsistency)
    console.log('  - Creating species...');
    const o2 = new Species({
        name: 'O2',
        molecular_weight_kg_mol: 32.0  // g/mol (matches Python usage)
    });

    const o = new Species({
        name: 'O',
        molecular_weight_kg_mol: 16.0  // g/mol
    });

    const o1d = new Species({
        name: 'O1D',
        molecular_weight_kg_mol: 16.0  // g/mol
    });

    const o3 = new Species({
        name: 'O3',
        molecular_weight_kg_mol: 48.0  // g/mol
    });

    const m = new Species({
        name: 'M',
        molecular_weight_kg_mol: 28.97,  // g/mol
        other_properties: {
            '__const': 'true'
        }
    });

    // Create phase
    console.log('  - Creating gas phase...');
    const gasPhase = new Phase({
        name: 'gas',
        species: ['M', 'O2', 'O', 'O1D', 'O3']
    });

    // Create reactions
    console.log('  - Creating reactions...');

    // Photolysis reactions
    const photoO2 = new Photolysis({
        name: 'PHOTO.jO2',
        reactants: [new ReactionComponent({ species_name: 'O2', coefficient: 1.0 })],
        products: [new ReactionComponent({ species_name: 'O', coefficient: 2.0 })],
        gas_phase: 'gas'
    });

    const photoO3_O = new Photolysis({
        name: 'PHOTO.jO3->O',
        reactants: [new ReactionComponent({ species_name: 'O3', coefficient: 1.0 })],
        products: [
            new ReactionComponent({ species_name: 'O2', coefficient: 1.0 }),
            new ReactionComponent({ species_name: 'O', coefficient: 1.0 })
        ],
        gas_phase: 'gas'
    });

    const photoO3_O1D = new Photolysis({
        name: 'PHOTO.jO3->O1D',
        reactants: [new ReactionComponent({ species_name: 'O3', coefficient: 1.0 })],
        products: [
            new ReactionComponent({ species_name: 'O2', coefficient: 1.0 }),
            new ReactionComponent({ species_name: 'O1D', coefficient: 1.0 })
        ],
        gas_phase: 'gas'
    });

    // Arrhenius reactions
    const r1 = new Arrhenius({
        name: 'O+O2',
        reactants: [
            new ReactionComponent({ species_name: 'O', coefficient: 1.0 }),
            new ReactionComponent({ species_name: 'O2', coefficient: 1.0 })
        ],
        products: [new ReactionComponent({ species_name: 'O3', coefficient: 1.0 })],
        A: 6.0e-34,
        B: 0.0,
        C: -2.4,
        D: 300.0,
        E: 0.0,
        gas_phase: 'gas'
    });

    const r2 = new Arrhenius({
        name: 'O+O3',
        reactants: [
            new ReactionComponent({ species_name: 'O', coefficient: 1.0 }),
            new ReactionComponent({ species_name: 'O3', coefficient: 1.0 })
        ],
        products: [new ReactionComponent({ species_name: 'O2', coefficient: 2.0 })],
        A: 8.0e-12,
        B: 0.0,
        C: 2060.0,
        D: 300.0,
        E: 0.0,
        gas_phase: 'gas'
    });

    const r3 = new Arrhenius({
        name: 'O1D+M',
        reactants: [
            new ReactionComponent({ species_name: 'O1D', coefficient: 1.0 }),
            new ReactionComponent({ species_name: 'M', coefficient: 1.0 })
        ],
        products: [
            new ReactionComponent({ species_name: 'O', coefficient: 1.0 }),
            new ReactionComponent({ species_name: 'M', coefficient: 1.0 })
        ],
        A: 2.15e-11,
        B: 0.0,
        C: -110.0,
        D: 300.0,
        E: 0.0,
        gas_phase: 'gas'
    });

    const r4 = new Arrhenius({
        name: 'O1D+O3',
        reactants: [
            new ReactionComponent({ species_name: 'O1D', coefficient: 1.0 }),
            new ReactionComponent({ species_name: 'O3', coefficient: 1.0 })
        ],
        products: [new ReactionComponent({ species_name: 'O2', coefficient: 2.0 })],
        A: 1.2e-10,
        B: 0.0,
        C: 0.0,
        D: 300.0,
        E: 0.0,
        gas_phase: 'gas'
    });

    // Create mechanism
    console.log('  - Creating mechanism...');
    const mechanism = new Mechanism({
        name: 'Chapman (in-code)',
        species: [m, o2, o, o1d, o3],
        phases: [gasPhase],
        reactions: [photoO2, photoO3_O, photoO3_O1D, r1, r2, r3, r4]
    });

    console.log('Mechanism created successfully!');
    console.log('\nCreating MICM solver from in-code mechanism...');

    // Create MICM solver from mechanism object (not file path!)
    const solver = new MICM({
        mechanism: mechanism,
        solver_type: SolverType.rosenbrock_standard_order
    });

    console.log('Solver created from mechanism object!');

    // Create state and run simulation
    console.log('\nRunning simulation...');
    const state = solver.createState(1);

    const timeStep = 200.0;
    const temperature = 272.5;
    const pressure = 101253.3;

    const rateConstants = {
        'PHOTO.PHOTO.jO2': 2.42e-17,
        'PHOTO.PHOTO.jO3->O': 1.15e-5,
        'PHOTO.PHOTO.jO3->O1D': 6.61e-9
    };

    const initialConcentrations = {
        'M': 1.0,
        'O2': 0.75,
        'O': 0.0,
        'O1D': 0.0,
        'O3': 0.0000081
    };

    // Set conditions
    state.setConditions({
        temperatures: temperature,
        pressures: pressure
    });

    // Set initial concentrations
    state.setConcentrations(initialConcentrations);

    // Set user-defined rate parameters (photolysis rates)
    state.setUserDefinedRateParameters(rateConstants);

    // Solve
    solver.solve(state, timeStep);

    // Get results
    const concentrations = state.getConcentrations();

    // Verify results
    const tolerance = 1e-5;

    function isClose(a, b, tol = tolerance) {
        return Math.abs(a - b) / Math.max(Math.abs(a), Math.abs(b)) < tol;
    }

    console.log('\nFinal concentrations:');
    console.log('  M:', concentrations.M[0]);
    console.log('  O2:', concentrations.O2[0]);
    console.log('  O:', concentrations.O[0]);
    console.log('  O1D:', concentrations.O1D[0]);
    console.log('  O3:', concentrations.O3[0]);

    // Assertions
    if (!isClose(concentrations.M[0], 1.0)) {
        throw new Error(`M concentration mismatch: ${concentrations.M[0]} (expected ~1.0)`);
    }

    if (!isClose(concentrations.O2[0], 0.75, 0.01)) {
        throw new Error(`O2 concentration mismatch: ${concentrations.O2[0]} (expected ~0.75)`);
    }

    if (concentrations.O[0] <= 0.0) {
        throw new Error(`O concentration should be > 0, got ${concentrations.O[0]}`);
    }

    if (concentrations.O1D[0] <= 0.0) {
        throw new Error(`O1D concentration should be > 0, got ${concentrations.O1D[0]}`);
    }

    if (concentrations.O3[0] === 0.0000081) {
        throw new Error(`O3 concentration should have changed from initial value`);
    }

    console.log('\nAll assertions passed!');
    console.log('Mechanism API functionality verified!');
}

function testMechanismExport() {
    console.log('\n============================================================');
    console.log('Mechanism API Test - Export Functionality');
    console.log('============================================================\n');

    console.log('Creating simple mechanism for export test...');

    const species1 = new Species({ name: 'A', molecular_weight_kg_mol: 0.010 });
    const species2 = new Species({ name: 'B', molecular_weight_kg_mol: 0.020 });

    const phase = new Phase({
        name: 'gas',
        species: ['A', 'B']
    });

    const reaction = new Arrhenius({
        name: 'A->B',
        reactants: [new ReactionComponent({ species_name: 'A', coefficient: 1.0 })],
        products: [new ReactionComponent({ species_name: 'B', coefficient: 1.0 })],
        A: 1.0e-10,
        gas_phase: 'gas'
    });

    const mechanism = new Mechanism({
        name: 'Simple Export Test',
        species: [species1, species2],
        phases: [phase],
        reactions: [reaction]
    });

    // Test serialize
    console.log('  - Testing serialize()...');
    const serialized = mechanism.getJSON();

    if (!serialized || typeof serialized !== 'object') {
        throw new Error('serialize() should return an object');
    }

    if (serialized.name !== 'Simple Export Test') {
        throw new Error(`Mechanism name mismatch: ${serialized.name}`);
    }

    if (!Array.isArray(serialized.species) || serialized.species.length !== 2) {
        throw new Error('serialized species should be an array with 2 elements');
    }

    if (!Array.isArray(serialized.phases) || serialized.phases.length !== 1) {
        throw new Error('serialized phases should be an array with 1 element');
    }

    console.log('serialize() works correctly!');
    console.log('\nExport functionality verified!');
}

// Run tests
try {
    testMechanismAPISimpleChemistry();
    testMechanismExport();
    console.log('\n============================================================');
    console.log('=> ALL MECHANISM API TESTS PASSED! <=');
    console.log('=> REQUIREMENTS SATISFIED! <=');
    console.log('============================================================\n');
    process.exit(0);
} catch (error) {
    console.error('\n=> TEST FAILED! <=');
    console.error(error.message);
    console.error(error.stack);
    process.exit(1);
}
