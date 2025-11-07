// Debug test to isolate the error

const {
    Species,
    ReactionComponent,
    Phase,
    Arrhenius,
    Mechanism
} = require('../../../index.js');

try {
    console.log('Testing Species creation...');
    const spec1 = new Species({
        name: 'A',
        molecular_weight_kg_mol: 0.010
    });
    console.log('✓ Species created');

    console.log('\nTesting ReactionComponent creation...');
    const comp1 = new ReactionComponent({ species_name: 'A', coefficient: 1.0 });
    console.log('✓ ReactionComponent created');

    console.log('\nTesting Phase creation...');
    const phase1 = new Phase({
        name: 'gas',
        species: ['A']
    });
    console.log('✓ Phase created');

    console.log('\nTesting Arrhenius creation...');
    const reaction1 = new Arrhenius({
        name: 'test',
        reactants: [comp1],
        products: [comp1],
        A: 1.0,
        B: 0.0,
        C: 0.0,
        D: 300.0,
        E: 0.0,
        gas_phase: 'gas'
    });
    console.log('✓ Arrhenius created');

    console.log('\nTesting Mechanism creation with empty arrays...');
    const mech1 = new Mechanism({
        name: 'Test',
        species: [],
        phases: [],
        reactions: []
    });
    console.log('✓ Empty Mechanism created');

    console.log('\nTesting Mechanism creation with species...');
    const mech2 = new Mechanism({
        name: 'Test',
        species: [spec1],
        phases: [],
        reactions: []
    });
    console.log('✓ Mechanism with species created');

    console.log('\nTesting Mechanism creation with species and phase...');
    const mech3 = new Mechanism({
        name: 'Test',
        species: [spec1],
        phases: [phase1],
        reactions: []
    });
    console.log('✓ Mechanism with species and phase created');

    console.log('\nTesting Mechanism creation with all components...');
    const mech4 = new Mechanism({
        name: 'Test',
        species: [spec1],
        phases: [phase1],
        reactions: [reaction1]
    });
    console.log('✓ Full Mechanism created');

    console.log('\n✓ ALL DEBUG TESTS PASSED!');
    process.exit(0);
} catch (error) {
    console.error('\n✗ Error:', error.message);
    console.error(error.stack);
    process.exit(1);
}
