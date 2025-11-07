// Unit tests for State class

const path = require('path');
const musica = require('../../index.js');
const { MICM, SolverType } = musica.micmSolver;

const CONFIG_PATH = path.join(__dirname, '../../../configs/v0/analytical');

console.log('State Unit Tests\n' + '='.repeat(60));

/**
 * Test State constructor validation
 */
function testStateConstructorValidation() {
    console.log('\nTest 1: State constructor validation');

    try {
        const micm = new MICM({
            config_path: CONFIG_PATH,
            solver_type: SolverType.rosenbrock_standard_order
        });

        // Test invalid number_of_grid_cells
        try {
            micm.createState(0);
            console.error('✗ Should have thrown error for 0 grid cells');
            process.exit(1);
        } catch (error) {
            console.assert(error.message.includes('must be greater than 0'), 'Should validate grid cells > 0');
        }

        console.log('State constructor validation passed');
    } catch (error) {
        console.error('State constructor validation failed:', error.message);
        process.exit(1);
    }
}

/**
 * Test State getSpeciesOrdering
 */
function testStateGetSpeciesOrdering() {
    console.log('\nTest 2: State getSpeciesOrdering');

    try {
        const micm = new MICM({
            config_path: CONFIG_PATH,
            solver_type: SolverType.rosenbrock_standard_order
        });

        const state = micm.createState(1);
        const ordering = state.getSpeciesOrdering();

        console.assert(typeof ordering === 'object', 'Ordering should be an object');
        console.assert('A' in ordering, 'Should have species A');
        console.assert('B' in ordering, 'Should have species B');
        console.assert('C' in ordering, 'Should have species C');
        console.assert(typeof ordering['A'] === 'number', 'Species index should be a number');

        console.log('State getSpeciesOrdering passed');
    } catch (error) {
        console.error('State getSpeciesOrdering failed:', error.message);
        process.exit(1);
    }
}

/**
 * Test State setConcentrations and getConcentrations
 */
function testStateConcentrations() {
    console.log('\nTest 3: State setConcentrations and getConcentrations');

    try {
        const micm = new MICM({
            config_path: CONFIG_PATH,
            solver_type: SolverType.rosenbrock_standard_order
        });

        const state = micm.createState(3);

        // Test scalar values (single grid cell behavior)
        state.setConcentrations({ A: [0.75, 0.80, 0.85] });

        const conc = state.getConcentrations();
        console.assert(Array.isArray(conc.A), 'Concentrations should be an array');
        console.assert(conc.A.length === 3, 'Should have 3 concentrations');
        console.assert(Math.abs(conc.A[0] - 0.75) < 1e-10, 'First concentration should be 0.75');
        console.assert(Math.abs(conc.A[1] - 0.80) < 1e-10, 'Second concentration should be 0.80');
        console.assert(Math.abs(conc.A[2] - 0.85) < 1e-10, 'Third concentration should be 0.85');

        console.log('State concentrations test passed');
    } catch (error) {
        console.error('State concentrations test failed:', error.message);
        process.exit(1);
    }
}

/**
 * Test State setConditions and getConditions
 */
function testStateConditions() {
    console.log('\nTest 4: State setConditions and getConditions');

    try {
        const micm = new MICM({
            config_path: CONFIG_PATH,
            solver_type: SolverType.rosenbrock_standard_order
        });

        const state = micm.createState(2);

        const temps = [273.15, 298.15];
        const pressures = [101325, 101325];

        state.setConditions({ temperatures: temps, pressures: pressures });

        const cond = state.getConditions();
        console.assert(Array.isArray(cond.temperature), 'Temperature should be an array');
        console.assert(cond.temperature.length === 2, 'Should have 2 temperatures');
        console.assert(Math.abs(cond.temperature[0] - 273.15) < 1e-10, 'First temperature should be 273.15');
        console.assert(Math.abs(cond.pressure[1] - 101325) < 1e-10, 'Second pressure should be 101325');
        console.assert(cond.air_density[0] > 0, 'Air density should be calculated and positive');

        console.log('State conditions test passed');
    } catch (error) {
        console.error('State conditions test failed:', error.message);
        process.exit(1);
    }
}

/**
 * Test State with vector-ordered solver (multiple internal states)
 */
function testStateVectorOrdered() {
    console.log('\nTest 5: State with vector-ordered solver');

    try {
        const micm = new MICM({
            config_path: CONFIG_PATH,
            solver_type: SolverType.rosenbrock
        });

        const state = micm.createState(10);

        // New architecture uses single internal state with vector-ordered indexing
        // No more getInternalStates() - test the concentration API directly
        console.assert(state.getNumberOfGridCells() === 10, 'Should have 10 grid cells');

        // Test that concentrations work with vector-ordered indexing
        const concentrations = { A: [] };
        for (let i = 0; i < 10; i++) {
            concentrations.A.push(0.1 * (i + 1));
        }

        state.setConcentrations(concentrations);
        const retrieved = state.getConcentrations();

        console.assert(retrieved.A.length === 10, 'Should have 10 concentration values');
        for (let i = 0; i < 10; i++) {
            console.assert(Math.abs(retrieved.A[i] - concentrations.A[i]) < 1e-10,
                `Concentration ${i} should match: expected ${concentrations.A[i]}, got ${retrieved.A[i]}`);
        }

        console.log('State vector-ordered test passed');
    } catch (error) {
        console.error('State vector-ordered test failed:', error.message);
        console.error(error.stack);
        process.exit(1);
    }
}

/**
 * Test State getUserDefinedRateParametersOrdering
 */
function testStateUserDefinedRateParameters() {
    console.log('\nTest 6: State user-defined rate parameters');

    try {
        const micm = new MICM({
            config_path: CONFIG_PATH,
            solver_type: SolverType.rosenbrock_standard_order
        });

        const state = micm.createState(2);

        const paramsOrdering = state.getUserDefinedRateParametersOrdering();
        console.assert(typeof paramsOrdering === 'object', 'Parameters ordering should be an object');
        console.assert('USER.reaction 1' in paramsOrdering, 'Should have USER.reaction 1');

        // Test setting and getting rate parameters
        state.setUserDefinedRateParameters({
            'USER.reaction 1': [0.001, 0.002],
            'USER.reaction 2': [0.003, 0.004]
        });

        const params = state.getUserDefinedRateParameters();
        console.assert(params['USER.reaction 1'][0] === 0.001, 'First rate parameter should be 0.001');
        console.assert(params['USER.reaction 2'][1] === 0.004, 'Second rate parameter should be 0.004');

        console.log('State user-defined rate parameters test passed');
    } catch (error) {
        console.error('State user-defined rate parameters test failed:', error.message);
        process.exit(1);
    }
}

/**
 * Run all tests
 */
function runTests() {
    try {
        testStateConstructorValidation();
        testStateGetSpeciesOrdering();
        testStateConcentrations();
        testStateConditions();
        testStateVectorOrdered();
        testStateUserDefinedRateParameters();

        console.log('ALL STATE UNIT TESTS PASSED!');
    } catch (error) {
        console.error('STATE UNIT TESTS FAILED!');
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
