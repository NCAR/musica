/**
 * Tests for the State class.
 * Converted from test_state.py
 */

const musica = require('musica-addon');
const { MICM } = musica.micmSolver;

/**
 * Helper function to create a test mechanism
 * Equivalent to create_test_mechanism() in Python
 */
function createTestMechanism() {
    // In the JS bindings, mechanism configuration is typically done via JSON/config files
    // rather than programmatically. This function would need to match your actual
    // JavaScript API for mechanism creation.
    // 
    // For now, assuming you have a similar programmatic API or config file path
    throw new Error('createTestMechanism needs to be implemented based on your JS API');
}

/**
 * Helper function to create a test solver
 * Equivalent to get_test_solver() in Python
 */
function getTestSolver(mech) {
    return new MICM({ mechanism: mech });
}

/**
 * Test State initialization with various grid cell configurations
 * Equivalent to test_state_initialization() in Python
 */
function test_state_initialization() {
    console.log('Running test_state_initialization...');

    const mech = createTestMechanism();
    const solver = getTestSolver(mech);

    // Test with valid input (single grid cell)
    const state = solver.createState(1);
    console.assert(state !== null && state !== undefined, 'State should be created');

    // Test with multiple grid cells
    const stateMulti = solver.createState(3);
    console.assert(stateMulti !== null && stateMulti !== undefined, 'Multi-cell state should be created');

    // Test with invalid input (0 grid cells)
    try {
        solver.createState(0);
        throw new Error('Should have thrown error for 0 grid cells');
    } catch (error) {
        console.assert(
            error.message.includes('number_of_grid_cells must be greater than 0') ||
            error.message.includes('must be greater than 0'),
            'Error message should mention grid cells > 0'
        );
    }

    console.log('✓ test_state_initialization passed');
}

/**
 * Test setting and getting concentrations
 * Equivalent to test_set_get_concentrations() in Python
 */
function test_set_get_concentrations() {
    console.log('Running test_set_get_concentrations...');

    const mech = createTestMechanism();
    const solver = getTestSolver(mech);

    // Test single grid cell
    const state = solver.createState(1);
    const concentrations = { A: 1.0, B: 2.0, C: 3.0 };
    state.setConcentrations(concentrations);
    const result = state.getConcentrations();

    console.assert(result.A[0] === 1.0, 'A concentration should be 1.0');
    console.assert(result.B[0] === 2.0, 'B concentration should be 2.0');
    console.assert(result.C[0] === 3.0, 'C concentration should be 3.0');

    // Test multiple grid cells
    const stateMulti = solver.createState(2);
    const concentrationsMulti = {
        A: [1.0, 2.0],
        B: [3.0, 4.0],
        C: [5.0, 6.0]
    };
    stateMulti.setConcentrations(concentrationsMulti);
    const resultMulti = stateMulti.getConcentrations();

    console.assert(
        JSON.stringify(resultMulti.A) === JSON.stringify([1.0, 2.0]),
        'A concentrations should match'
    );
    console.assert(
        JSON.stringify(resultMulti.B) === JSON.stringify([3.0, 4.0]),
        'B concentrations should match'
    );
    console.assert(
        JSON.stringify(resultMulti.C) === JSON.stringify([5.0, 6.0]),
        'C concentrations should match'
    );

    // Test invalid species
    try {
        state.setConcentrations({ D: 1.0 });
        throw new Error('Should have thrown error for invalid species');
    } catch (error) {
        console.assert(
            error.message.includes('Species D not found'),
            'Error should mention species not found'
        );
    }

    // Test invalid length
    try {
        stateMulti.setConcentrations({ A: [1.0] });
        throw new Error('Should have thrown error for invalid array length');
    } catch (error) {
        console.assert(
            error.message.includes('must have length'),
            'Error should mention invalid length'
        );
    }

    // Test cannot set third-body species
    try {
        state.setConcentrations({ M: 1.0 });
        throw new Error('Should have thrown error for third-body species');
    } catch (error) {
        console.assert(
            error.message.includes('Species M not found'),
            'Error should mention species not found'
        );
    }

    console.log('✓ test_set_get_concentrations passed');
}

/**
 * Helper function to check if values are close
 * Equivalent to np.isclose in Python
 */
function isClose(a, b, rtol = 1e-5, atol = 1e-8) {
    const diff = Math.abs(a - b);
    return diff <= (atol + rtol * Math.abs(b));
}

/**
 * Test setting and getting environmental conditions
 * Equivalent to test_set_get_conditions() in Python
 */
function test_set_get_conditions() {
    console.log('Running test_set_get_conditions...');

    const mech = createTestMechanism();
    const solver = getTestSolver(mech);

    // Test single grid cell
    const state = solver.createState(1);
    state.setConditions({ temperatures: 300.0, pressures: 101325.0 });
    const conditions = state.getConditions();

    console.assert(conditions.temperature[0] === 300.0, 'Temperature should be 300.0');
    console.assert(conditions.pressure[0] === 101325.0, 'Pressure should be 101325.0');
    console.assert(
        isClose(conditions.air_density[0], 40.9, 0.1),
        'Air density should be approximately 40.9'
    );

    // Test multiple grid cells
    const stateMulti = solver.createState(2);
    stateMulti.setConditions({
        temperatures: [300.0, 310.0],
        pressures: [101325.0, 101325.0],
        air_densities: [40.9, 39.5]
    });
    const conditionsMulti = stateMulti.getConditions();

    console.assert(
        JSON.stringify(conditionsMulti.temperature) === JSON.stringify([300.0, 310.0]),
        'Temperatures should match'
    );
    console.assert(
        JSON.stringify(conditionsMulti.pressure) === JSON.stringify([101325.0, 101325.0]),
        'Pressures should match'
    );
    console.assert(
        JSON.stringify(conditionsMulti.air_density) === JSON.stringify([40.9, 39.5]),
        'Air densities should match'
    );

    // Test invalid input length
    try {
        stateMulti.setConditions({ temperatures: [300.0] });
        throw new Error('Should have thrown error for invalid array length');
    } catch (error) {
        console.assert(
            error.message.includes('must be a list of length'),
            'Error should mention invalid length'
        );
    }

    console.log('✓ test_set_get_conditions passed');
}

/**
 * Test setting and getting user-defined rate parameters
 * Equivalent to test_set_get_user_defined_rate_parameters() in Python
 */
function test_set_get_user_defined_rate_parameters() {
    console.log('Running test_set_get_user_defined_rate_parameters...');

    const mech = createTestMechanism();
    const solver = getTestSolver(mech);

    // Test single grid cell
    const state = solver.createState(1);
    let params = { 'EMIS.my emission': 1.0 };
    state.setUserDefinedRateParameters(params);
    let result = state.getUserDefinedRateParameters();

    console.assert(
        result['EMIS.my emission'][0] === 1.0,
        'Emission parameter should be 1.0'
    );

    params = {
        'SURF.my surface.effective radius [m]': 0.5,
        'SURF.my surface.particle number concentration [# m-3]': 1000.0,
    };
    state.setUserDefinedRateParameters(params);
    result = state.getUserDefinedRateParameters();

    console.assert(
        result['SURF.my surface.effective radius [m]'][0] === 0.5,
        'Surface radius should be 0.5'
    );
    console.assert(
        result['SURF.my surface.particle number concentration [# m-3]'][0] === 1000.0,
        'Particle concentration should be 1000.0'
    );

    // Test multiple grid cells
    const stateMulti = solver.createState(2);
    const paramsMulti = { 'PHOTO.photo B': [1.0, 2.0] };
    stateMulti.setUserDefinedRateParameters(paramsMulti);
    const resultMulti = stateMulti.getUserDefinedRateParameters();

    console.assert(
        JSON.stringify(resultMulti['PHOTO.photo B']) === JSON.stringify([1.0, 2.0]),
        'Photo parameters should match'
    );

    // Test invalid parameter
    try {
        state.setUserDefinedRateParameters({ invalid_param: 1.0 });
        throw new Error('Should have thrown error for invalid parameter');
    } catch (error) {
        console.assert(
            error.message.includes('User-defined rate parameter invalid_param not found'),
            'Error should mention parameter not found'
        );
    }

    // Test species ordering
    const speciesOrdering = state.getSpeciesOrdering();
    console.assert(
        typeof speciesOrdering === 'object' && speciesOrdering !== null,
        'Species ordering should be an object'
    );
    console.assert(
        Object.keys(speciesOrdering).length === 3,
        'Should have 3 species (A, B, C - M is third-body)'
    );

    // Dictionary style access
    console.assert(speciesOrdering.A >= 0, 'A should have valid index');
    console.assert(speciesOrdering.B >= 0, 'B should have valid index');
    console.assert(speciesOrdering.C >= 0, 'C should have valid index');

    // Test key membership (JavaScript equivalent of 'in' operator)
    console.assert('A' in speciesOrdering, 'A should be in species ordering');
    console.assert('B' in speciesOrdering, 'B should be in species ordering');
    console.assert('C' in speciesOrdering, 'C should be in species ordering');
    console.assert(!('M' in speciesOrdering), 'M should not be in species ordering (third-body)');

    // Test parameter ordering
    const paramOrdering = state.getUserDefinedRateParametersOrdering();
    console.assert(
        typeof paramOrdering === 'object' && paramOrdering !== null,
        'Parameter ordering should be an object'
    );
    console.assert(
        Object.keys(paramOrdering).length === 6,
        'Should have 6 parameters'
    );

    // Convert dict keys to list using Object.keys()
    const paramNames = Object.keys(paramOrdering);
    console.assert(paramNames.length === 6, 'Should have 6 parameter names');
    console.assert(typeof paramNames[0] === 'string', 'Parameter names should be strings');

    // Sort the keys if needed - useful for consistent ordering in tests
    const sortedParamNames = Object.keys(paramOrdering).sort();
    console.assert(sortedParamNames.length === 6, 'Sorted list should have 6 items');
    console.assert(
        sortedParamNames.every(name => typeof name === 'string'),
        'All parameter names should be strings'
    );

    // Verify all expected keys are present
    const expectedParams = [
        'PHOTO.photo B',
        'EMIS.my emission',
        'LOSS.my first order loss',
        'SURF.my surface.effective radius [m]',
        'SURF.my surface.particle number concentration [# m-3]',
        'USER.my user defined'
    ];
    const sortedExpectedParams = expectedParams.slice().sort();
    console.assert(
        JSON.stringify(sortedExpectedParams) === JSON.stringify(sortedParamNames),
        'Expected parameters should match actual parameters'
    );

    console.log('✓ test_set_get_user_defined_rate_parameters passed');
}

/**
 * Main test runner
 */
function runTests() {
    console.log('Starting State Tests (converted from test_state.py)...\n');
    console.log('='.repeat(60));

    try {
        test_state_initialization();
        test_set_get_concentrations();
        test_set_get_conditions();
        test_set_get_user_defined_rate_parameters();

        console.log('\n' + '='.repeat(60));
        console.log('ALL TESTS PASSED! ✓');
        console.log('='.repeat(60));

    } catch (error) {
        console.error('\n' + '='.repeat(60));
        console.error('TEST FAILED! ✗');
        console.error('='.repeat(60));
        console.error('Error:', error.message);
        console.error(error.stack);
        process.exit(1);
    }
}

// Run tests if this file is executed directly
if (require.main === module) {
    runTests();
}

module.exports = {
    test_state_initialization,
    test_set_get_concentrations,
    test_set_get_conditions,
    test_set_get_user_defined_rate_parameters,
    runTests,
    isClose
};