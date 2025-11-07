const path = require('path');
const musica = require('../../index.js');
const { MICM, SolverType } = musica.micmSolver;


// Test configuration
const CONFIG_PATH = path.join(__dirname, '../../../configs/v0/TS1');

/**
 * Test TS1 configuration with varying numbers of grid cells
 * to determine memory and performance limits
 */
function testTS1Performance() {
    console.log('Starting TS1 Performance Tests...');
    console.log('Configuration path:', CONFIG_PATH);
    console.log('='.repeat(60));

    // Test different grid cell counts
    const gridCellCounts = [1, 10, 50, 100, 200, 500, 1000];
    const results = [];

    for (const numCells of gridCellCounts) {
        console.log(`\nTesting with ${numCells} grid cells...`);

        try {
            const startTime = Date.now();

            // Create solver
            const micm = new MICM({
                config_path: CONFIG_PATH,
                solver_type: SolverType.rosenbrock_standard_order
            });

            const createTime = Date.now() - startTime;
            console.log(`  Solver created in ${createTime}ms`);

            // Create state
            const stateStartTime = Date.now();
            const state = micm.createState(numCells);
            const stateCreateTime = Date.now() - stateStartTime;
            console.log(`  State created in ${stateCreateTime}ms`);

            // Get species ordering
            const ordering = state.getSpeciesOrdering();
            const numSpecies = Object.keys(ordering).length;
            console.log(`  Number of species: ${numSpecies}`);

            // Set initial conditions
            const temperature = 272.5;
            const pressure = 101253.3;

            // Set conditions for all cells
            const temperatures = Array(numCells).fill(temperature);
            const pressures = Array(numCells).fill(pressure);

            const condStartTime = Date.now();
            state.setConditions({
                temperatures: temperatures,
                pressures: pressures
            });
            const condTime = Date.now() - condStartTime;
            console.log(`  Conditions set in ${condTime}ms`);

            // Set concentrations (all to 1.0 for testing)
            const concentrations = {};
            for (const species in ordering) {
                concentrations[species] = Array(numCells).fill(1.0);
            }

            const concStartTime = Date.now();
            state.setConcentrations(concentrations);
            const concTime = Date.now() - concStartTime;
            console.log(`  Concentrations set in ${concTime}ms`);

            // Perform solve
            const timeStep = 1.0;
            const solveStartTime = Date.now();
            micm.solve(state, timeStep);
            const solveTime = Date.now() - solveStartTime;
            console.log(`  Solve completed in ${solveTime}ms`);

            // Get memory usage (if available)
            const memUsage = process.memoryUsage();
            const memMB = {
                rss: (memUsage.rss / 1024 / 1024).toFixed(2),
                heapTotal: (memUsage.heapTotal / 1024 / 1024).toFixed(2),
                heapUsed: (memUsage.heapUsed / 1024 / 1024).toFixed(2),
                external: (memUsage.external / 1024 / 1024).toFixed(2)
            };
            console.log(`  Memory usage (MB): RSS=${memMB.rss}, Heap Used=${memMB.heapUsed}, External=${memMB.external}`);

            const totalTime = Date.now() - startTime;

            results.push({
                numCells,
                numSpecies,
                createTime,
                stateCreateTime,
                condTime,
                concTime,
                solveTime,
                totalTime,
                memoryMB: memMB,
                success: true
            });

            console.log(`Test passed for ${numCells} grid cells (total: ${totalTime}ms)`);

        } catch (error) {
            console.error(`Test failed for ${numCells} grid cells: ${error.message}`);
            results.push({
                numCells,
                success: false,
                error: error.message
            });
            // Continue to next test
        }
    }

    // Print summary
    console.log('\nPERFORMANCE TEST SUMMARY');
    console.log('\nSuccessful tests:');
    console.log('Grid Cells | Species | Solve Time | Total Time | Memory (Heap Used)');
    console.log('-'.repeat(70));

    const successful = results.filter(r => r.success);
    for (const result of successful) {
        console.log(
            `${result.numCells.toString().padStart(10)} | ` +
            `${result.numSpecies.toString().padStart(7)} | ` +
            `${(result.solveTime + 'ms').padStart(10)} | ` +
            `${(result.totalTime + 'ms').padStart(10)} | ` +
            `${result.memoryMB.heapUsed}MB`
        );
    }

    if (successful.length > 0) {
        const maxCells = Math.max(...successful.map(r => r.numCells));
        const lastResult = successful[successful.length - 1];

        console.log('\nRECOMMENDATIONS:');
        console.log(`• Maximum tested grid cells: ${maxCells}`);
        console.log(`• Number of species in TS1: ${lastResult.numSpecies}`);

        // Calculate acceptable limits based on solve time
        const acceptableTime = 5000; // 5 seconds
        const acceptableTests = successful.filter(r => r.solveTime <= acceptableTime);
        if (acceptableTests.length > 0) {
            const maxAcceptable = Math.max(...acceptableTests.map(r => r.numCells));
            console.log(`• Recommended max grid cells (solve < 5s): ${maxAcceptable}`);
        }
    }

    const failed = results.filter(r => !r.success);
    if (failed.length > 0) {
        console.log('\nFailed tests:');
        for (const result of failed) {
            console.log(`  • ${result.numCells} grid cells: ${result.error}`);
        }
    }

    console.log('\n=> TS1 PERFORMANCE TEST COMPLETE <=\n');
}

// Run the test
try {
    testTS1Performance();
} catch (error) {
    console.error('Fatal error:', error);
    process.exit(1);
}
