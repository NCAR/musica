// Unit tests for MICM class

const path = require('path');
const musica = require('../../index.js');
const { MICM, SolverType } = musica.micmSolver;

const CONFIG_PATH = path.join(__dirname, '../../../configs/v0/analytical');

console.log('MICM Unit Tests\n' + '='.repeat(60));

/**
 * Test MICM constructor
 */
function testMICMConstructor() {
    console.log('\nTest 1: MICM constructor with valid config');

    try {
        const micm = new MICM({
            config_path: CONFIG_PATH,
            solver_type: SolverType.rosenbrock_standard_order
        });

        console.assert(micm !== null, 'MICM instance should not be null');
        console.assert(typeof micm.createState === 'function', 'createState should be a function');
        console.assert(typeof micm.solve === 'function', 'solve should be a function');
        console.assert(micm.solverType() === SolverType.rosenbrock_standard_order, 'Solver type should match');

        console.log('MICM constructor test passed');
    } catch (error) {
        console.error('MICM constructor test failed:', error.message);
        process.exit(1);
    }
}

/**
 * Test MICM constructor with missing config_path
 */
function testMICMConstructorMissingConfig() {
    console.log('\nTest 2: MICM constructor with missing config_path');

    try {
        const micm = new MICM({ solver_type: SolverType.rosenbrock });
        console.error('Should have thrown an error for missing config_path');
        process.exit(1);
    } catch (error) {
        console.assert(error.message.includes('config_path'), 'Error should mention config_path');
        console.log('MICM constructor correctly rejects missing config_path');
    }
}

/**
 * Test MICM createState for standard-order solver
 */
function testMICMCreateStateStandardOrder() {
    console.log('\nTest 3: MICM createState for standard-order solver');

    try {
        const micm = new MICM({
            config_path: CONFIG_PATH,
            solver_type: SolverType.rosenbrock_standard_order
        });

        const state = micm.createState(5);

        console.assert(state !== null, 'State should not be null');
        console.assert(state.getNumberOfGridCells() === 5, 'Should have 5 grid cells');
        console.assert(typeof state.getSpeciesOrdering === 'function', 'Should have getSpeciesOrdering method');

        const ordering = state.getSpeciesOrdering();
        console.assert(Object.keys(ordering).length > 0, 'Should have species ordering');

        console.log('MICM createState for standard-order solver passed');
    } catch (error) {
        console.error('MICM createState test failed:', error.message);
        process.exit(1);
    }
}

/**
 * Test MICM createState for vector-ordered solver
 */
function testMICMCreateStateVectorOrder() {
    console.log('\nTest 4: MICM createState for vector-ordered solver');

    try {
        const micm = new MICM({
            config_path: CONFIG_PATH,
            solver_type: SolverType.rosenbrock
        });

        const state = micm.createState(10);

        console.assert(state !== null, 'State should not be null');
        console.assert(state.getNumberOfGridCells() === 10, 'Should have 10 grid cells');

        // New architecture uses single internal state with vector-ordered indexing
        // Verify state has proper methods for vector-ordered access
        console.assert(typeof state.getSpeciesOrdering === 'function', 'Should have getSpeciesOrdering method');
        console.assert(typeof state.setConcentrations === 'function', 'Should have setConcentrations method');
        console.assert(typeof state.getConcentrations === 'function', 'Should have getConcentrations method');

        const ordering = state.getSpeciesOrdering();
        console.assert(Object.keys(ordering).length > 0, 'Should have species ordering');

        console.log('MICM createState for vector-ordered solver passed');
    } catch (error) {
        console.error('MICM createState vector-order test failed:', error.message);
        process.exit(1);
    }
}

/**
 * Test MICM solve method type checking
 */
function testMICMSolveTypeChecking() {
    console.log('\nTest 5: MICM solve method type checking');

    try {
        const micm = new MICM({
            config_path: CONFIG_PATH,
            solver_type: SolverType.rosenbrock_standard_order
        });

        const state = micm.createState(1);

        // Test invalid state type
        try {
            micm.solve({}, 1.0);
            console.error('Should have thrown TypeError for invalid state');
            process.exit(1);
        } catch (error) {
            console.assert(error instanceof TypeError, 'Should throw TypeError for invalid state');
        }

        // Test invalid timeStep type
        try {
            micm.solve(state, "not a number");
            console.error('✗ Should have thrown TypeError for invalid timeStep');
            process.exit(1);
        } catch (error) {
            console.assert(error instanceof TypeError, 'Should throw TypeError for invalid timeStep');
        }

        console.log('MICM solve type checking passed');
    } catch (error) {
        console.error('MICM solve type checking test failed:', error.message);
        process.exit(1);
    }
}

/**
 * Run all tests
 */
function runTests() {
    try {
        testMICMConstructor();
        testMICMConstructorMissingConfig();
        testMICMCreateStateStandardOrder();
        testMICMCreateStateVectorOrder();
        testMICMSolveTypeChecking();

        console.log('ALL MICM UNIT TESTS PASSED!');
    } catch (error) {
        console.error('MICM UNIT TESTS FAILED!');
        console.error(error);
        process.exit(1);
    }
}

if (require.main === module) {
    runTests();
}

module.exports = { runTests };

// Clean exit
process.exit(0);
