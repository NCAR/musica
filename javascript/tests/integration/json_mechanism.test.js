const path = require('path');
const musica = require('../../../build/Release/musica-addon.node');
const { Mechanism } = require('../../mechanism_configuration/mechanism');
const { types } = require('../../mechanism_configuration/types');
const { reactionTypes } = require('../../mechanism_configuration/reaction_types');

// Destructure types
const { Species, Phase, ReactionComponent } = types;
const { Arrhenius, UserDefined, Photolysis } = reactionTypes;

/**
 * Test creating MICM solver from JSON string
 * This test demonstrates the new JSON string approach for mechanism configuration
 */
function testJsonMechanismCreation() {
    console.log('\nTesting JSON Mechanism Creation\n');

    // 1. Create mechanism using JavaScript classes
    console.log('Step 1: Creating mechanism using JavaScript classes...');

    const species = [
        new Species({
            name: 'A',
            molecular_weight: 0.025
        }),
        new Species({
            name: 'B',
            molecular_weight: 0.030
        }),
        new Species({
            name: 'C',
            molecular_weight: 0.040
        })
    ];

    const gasPhase = new Phase({
        name: 'gas',
        species: species
    });

    const reactions = [
        new Arrhenius({
            name: 'A to B',
            reactants: [new ReactionComponent({ species_name: 'A', coefficient: 1.0 })],
            products: [new ReactionComponent({ species_name: 'B', coefficient: 1.0 })],
            A: 1.0,
            B: 0.0,
            C: 0.0,
            gas_phase: 'gas'
        })
    ];

    const mechanism = new Mechanism({
        name: 'Test Mechanism from JSON',
        species: species,
        phases: [gasPhase],
        reactions: reactions
    });

    console.log('Mechanism created');

    // 2. Generate JSON string
    console.log('\nStep 2: Generating JSON string...');
    const mechanismJson = mechanism.getJSON();
    const jsonString = JSON.stringify(mechanismJson, null, 2);

    console.log('JSON structure:');
    console.log(' - Version:', mechanismJson.version);
    console.log(' - Name:', mechanismJson.name);
    console.log(' - Species count:', mechanismJson.species.length);
    console.log(' - Phases count:', mechanismJson.phases.length);
    console.log(' - Reactions count:', mechanismJson.reactions.length);
    console.log('JSON generated');

    // 3. Create MICM solver from JSON string
    console.log('\nStep 3: Creating MICM solver from JSON string...');

    const MICM_ADDON = musica.MICM;
    const SolverType = 1; // RosenbrockStandardOrder

    try {
        const solver = new MICM_ADDON(
            { config_json: jsonString },
            SolverType
        );
        console.log('MICM solver created successfully from JSON string!');

        // 4. Create state and verify
        console.log('\nStep 4: Creating solver state...');
        const state = solver.createState(1);
        console.log('State created successfully');

        console.log('\nTest PASSED\n');
        return true;
    } catch (error) {
        console.error('Failed to create MICM solver from JSON string');
        console.error('Error:', error.message);
        console.log('\nTest FAILED\n');
        return false;
    }
}

/**
 * Test backward compatibility with config_path
 */
function testConfigPathBackwardCompatibility() {
    console.log('\nTesting Backward Compatibility (config_path)\n');

    const CONFIG_PATH = path.join(__dirname, '../../../configs/v0/analytical');
    const MICM_ADDON = musica.MICM;
    const SolverType = 1; // RosenbrockStandardOrder

    try {
        // Old way: passing config_path as string (backward compatible)
        const solver1 = new MICM_ADDON(CONFIG_PATH, SolverType);
        console.log('Method 1: Direct string parameter works (backward compatible)');

        // New way: passing config_path in options object
        const solver2 = new MICM_ADDON({ config_path: CONFIG_PATH }, SolverType);
        console.log('Method 2: Options object with config_path works');

        console.log('\nBackward Compatibility Test PASSED\n');
        return true;
    } catch (error) {
        console.error('Backward compatibility test failed');
        console.error('Error:', error.message);
        console.log('\nBackward Compatibility Test FAILED\n');
        return false;
    }
}

/**
 * Main test runner
 */
function runTests() {
    console.log('\n');
    console.log('MUSICA JSON Mechanism Configuration Test Suite');
    console.log('\n');

    let allPassed = true;

    // Test 1: JSON mechanism creation
    allPassed = testJsonMechanismCreation() && allPassed;

    // Test 2: Backward compatibility
    allPassed = testConfigPathBackwardCompatibility() && allPassed;

    // Summary
    if (allPassed) {
        console.log('ALL TESTS PASSED \n');
        process.exit(0);
    } else {
        console.log('SOME TESTS FAILED \n');
        process.exit(1);
    }
}

// Run tests if this file is executed directly
if (require.main === module) {
    runTests();
}

module.exports = {
    testJsonMechanismCreation,
    testConfigPathBackwardCompatibility,
    runTests
};
