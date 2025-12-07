// simulation routes
// runs chemistry sims with musica/micm

import express from 'express';
import path from 'path';
import fs from 'fs';
import { randomUUID } from 'crypto';
import { fileURLToPath } from 'url';
import { loadV1MechanismAsString } from '../utils/loadV1Mechanism.js';
import { createDynamicMechanism } from '../utils/DynamicMechanism.js';
import { interpolateValue, validateEvolvingConditions } from '../utils/interpolation.js';
import { createRequire } from 'module';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const require = createRequire(import.meta.url);

const router = express.Router();

// load musica addon
let MICM, SolverType, musica;
try {
    musica = require(path.join(__dirname, '../../../javascript/index.js'));
    MICM = musica.micmSolver.MICM;
    SolverType = musica.micmSolver.SolverType;
    console.log('MUSICA addon loaded successfully');
} catch (error) {
    console.error('Failed to load MUSICA addon:', error.message);
    console.error('  Make sure to build the Node.js addon first with: npm run build');
}

// path to configs
const CONFIGS_BASE_PATH = path.join(__dirname, '../../../configs');

// mechanism configs
// using directory paths, loadV1Mechanism appends config.json
// NOTE: v0 CAMP format crashes - stick with v1 only
const MECHANISM_CONFIGS = {
    chapman: path.join(CONFIGS_BASE_PATH, 'v1/chapman'),
    ts1: path.join(CONFIGS_BASE_PATH, 'v1/ts1'),
    full_configuration: path.join(CONFIGS_BASE_PATH, 'v1/full_configuration'),  // Comprehensive test with all reaction types
    // analytical: path.join(CONFIGS_BASE_PATH, 'v0/analytical'),  // No v1 version available yet
};

// cache for simulation results
// keep max 100 to avoid memory issues
const MAX_SIMULATIONS = 100;
const simulations = new Map();

// POST /api/simulation/run
router.post('/run', async (req, res) => {
    let tempConfigPath = null;

    try {
        if (!MICM) {
            return res.status(503).json({
                success: false,
                error: 'MUSICA addon not available',
                message: 'The MUSICA C++ addon is not built. Run "npm run build" in the root directory.',
            });
        }

        const {
            mechanism,
            species = [],
            reactions = [],
            temperature = 298.15,
            pressure = 101325,
            timeStep = 200,
            duration = 3600,
            initialConcentrations = {},
            rateConstants = {},
            outputFrequency = 10,
            evolvingConditions = null,
        } = req.body;

        // check required params
        if (!mechanism) {
            return res.status(400).json({
                success: false,
                error: 'Missing required parameter: mechanism',
            });
        }

        // custom or predefined mechanism?
        const isCustomMechanism = mechanism === 'custom';
        const configPath = MECHANISM_CONFIGS[mechanism];

        if (!isCustomMechanism && !configPath) {
            return res.status(400).json({
                success: false,
                error: 'Invalid mechanism',
                message: `Mechanism "${mechanism}" not found. Available: ${Object.keys(MECHANISM_CONFIGS).join(', ')}, custom`,
            });
        }

        // custom needs species and reactions
        if (isCustomMechanism && (species.length === 0 || reactions.length === 0)) {
            return res.status(400).json({
                success: false,
                error: 'Custom mechanism requires species and reactions',
                message: 'Please provide at least one species and one reaction for custom mechanisms',
            });
        }

        // validate evolving conditions if present
        let useEvolvingConditions = false;
        if (evolvingConditions && evolvingConditions.enabled) {
            const validation = validateEvolvingConditions(evolvingConditions);
            if (!validation.isValid) {
                return res.status(400).json({
                    success: false,
                    error: 'Invalid evolving conditions',
                    message: validation.errors.join('; '),
                });
            }
            useEvolvingConditions = true;
        }

        // generate sim id
        const simulationId = randomUUID();
        console.log(`\nStarting simulation ${simulationId}`);
        console.log(`   Mechanism: ${mechanism}`);
        console.log(`   Temperature: ${useEvolvingConditions ? 'EVOLVING' : temperature + ' K'}`);
        console.log(`   Pressure: ${useEvolvingConditions ? 'EVOLVING' : pressure + ' Pa'}`);
        console.log(`   Time step: ${timeStep} s`);
        console.log(`   Duration: ${duration} s`);
        if (useEvolvingConditions) {
            console.log(`   Evolving conditions enabled with ${evolvingConditions.times.length} time points`);
        }

        // init solver
        let solver;

        if (isCustomMechanism) {
            console.log(`   Creating custom mechanism`);
            const mechanismObj = createDynamicMechanism(musica, species, reactions);
            const mechanismJSON = mechanismObj.getJSON();

            tempConfigPath = `/tmp/custom_mechanism_${simulationId}.json`;
            fs.writeFileSync(tempConfigPath, JSON.stringify(mechanismJSON, null, 2));
            console.log(`   Wrote mechanism JSON to: ${tempConfigPath}`);

            solver = new MICM({
                config_path: tempConfigPath,
                solver_type: SolverType.rosenbrock_standard_order,
            });
        } else {
            // using predefined mech
            const isV1Config = configPath.includes('/v1/');

            if (isV1Config) {
                console.log(`   Loading v1 mechanism from: ${configPath}/config.json`);
                const mechanismJSON = loadV1MechanismAsString(configPath);

                tempConfigPath = `/tmp/predefined_mechanism_${simulationId}.json`;
                fs.writeFileSync(tempConfigPath, mechanismJSON);
                console.log(`   Wrote mechanism JSON to: ${tempConfigPath}`);

                solver = new MICM({
                    config_path: tempConfigPath,
                    solver_type: SolverType.rosenbrock_standard_order,
                });
            } else {
                // DEPRECATED: v0 configs crash with segfaults
                // kept for reference, don't use
                // convert everything to v1
                throw new Error(`v0 CAMP format configs are deprecated and cause crashes. Please convert ${mechanism} to v1 format.`);
            }
        }

        let state = solver.createState(1);

        // set initial conditions
        // if evolving, use first time point, otherwise use params
        const initialTemp = useEvolvingConditions && evolvingConditions.temperature.length > 0
            ? evolvingConditions.temperature[0]
            : temperature;
        const initialPressure = useEvolvingConditions && evolvingConditions.pressure.length > 0
            ? evolvingConditions.pressure[0]
            : pressure;

        state.setConditions({
            temperatures: initialTemp,
            pressures: initialPressure,
        });

        // auto-init reactants for custom mechs
        // makes it easier for non-chemists
        let enhancedConcentrations = { ...initialConcentrations };

        if (isCustomMechanism && reactions.length > 0) {
            const DEFAULT_REACTANT_CONC = 1e-6; // small default (1 ppb)

            // collect all reactants
            const allReactants = new Set();
            reactions.forEach(rxn => {
                if (rxn.reactants) {
                    rxn.reactants.forEach(reactant => {
                        const speciesName = reactant.name || reactant.species || reactant['species name'] || reactant;
                        if (typeof speciesName === 'string') {
                            allReactants.add(speciesName);
                        }
                    });
                }
            });

            // auto-init anything with zero or missing conc
            allReactants.forEach(speciesName => {
                if (!enhancedConcentrations[speciesName] || enhancedConcentrations[speciesName] === 0) {
                    enhancedConcentrations[speciesName] = DEFAULT_REACTANT_CONC;
                    console.log(`   Auto-initialized ${speciesName} with default concentration: ${DEFAULT_REACTANT_CONC}`);
                }
            });
        }

        // set initial concentrations
        if (Object.keys(enhancedConcentrations).length > 0) {
            const validConcentrations = {};
            const skippedSpecies = [];

            // validate without creating temp state objects
            // SAFER: just set everything and catch errors
            // avoids segfaults from temp state creation
            for (const [species, value] of Object.entries(enhancedConcentrations)) {
                // assume all valid for now
                // setConcentrations will warn if species doesn't exist
                validConcentrations[species] = value;
            }

            if (skippedSpecies.length > 0) {
                console.log(`   Skipping species not in mechanism: ${skippedSpecies.join(', ')}`);
            }

            // set all valid concentrations
            if (Object.keys(validConcentrations).length > 0) {
                state.setConcentrations(validConcentrations);
                console.log(`   Set concentrations for: ${Object.keys(validConcentrations).join(', ')}`);
            }
        }

        // set photolysis and user-defined rate params
        // IMPORTANT: PHOTOLYSIS and USER_DEFINED need runtime rates
        // via setUserDefinedRateParameters(), even with scaling_factor set
        const runtimeRates = { ...rateConstants };

        // extract photolysis rates from reactions
        // NOTE: USER_DEFINED doesn't need runtime params with Mechanism API
        // scaling_factor in constructor is enough
        if (isCustomMechanism && reactions.length > 0) {
            reactions.forEach(rxn => {
                if (rxn.type === 'PHOTOLYSIS') {
                    const scalingFactor = rxn.scalingFactor || rxn.scaling_factor || rxn['scaling factor'] || 1.0;

                    // get reactant name (handles different formats)
                    let reactantName = '';
                    if (rxn.reactants && rxn.reactants.length > 0) {
                        const first = rxn.reactants[0];
                        reactantName = first.name || first.species || first['species name'] || first;
                    }

                    // runtime param names: PHOTO.jSpecies
                    const paramName = `PHOTO.j${reactantName}`;
                    runtimeRates[paramName] = scalingFactor;
                    console.log(`   Setting PHOTOLYSIS rate for "${paramName}": ${scalingFactor}`);
                }
            });
        }

        if (Object.keys(runtimeRates).length > 0) {
            console.log(`   Applying runtime rate parameters:`, Object.keys(runtimeRates));
            state.setUserDefinedRateParameters(runtimeRates);
        }

        // run simulation
        const results = [];
        const environmentalData = []; // track temp/pressure over time
        const steps = Math.floor(duration / timeStep);
        let currentTime = 0;

        // store initial state
        results.push({
            time: currentTime,
            concentrations: state.getConcentrations(),
        });
        environmentalData.push({
            time: currentTime,
            temperature: initialTemp,
            pressure: initialPressure,
        });

        // solve each time step
        for (let step = 0; step < steps; step++) {
            currentTime += timeStep;

            // update conditions if evolving
            if (useEvolvingConditions) {
                const interpolationMethod = evolvingConditions.interpolationMethod || 'linear';
                const currentTemp = interpolateValue(
                    currentTime,
                    evolvingConditions.times,
                    evolvingConditions.temperature,
                    temperature,
                    interpolationMethod
                );
                const currentPressure = interpolateValue(
                    currentTime,
                    evolvingConditions.times,
                    evolvingConditions.pressure,
                    pressure,
                    interpolationMethod
                );

                state.setConditions({
                    temperatures: currentTemp,
                    pressures: currentPressure,
                });

                // log updates occasionally
                if (step % 100 === 0) {
                    console.log(`   t=${currentTime}s: T=${currentTemp.toFixed(2)}K, P=${currentPressure.toFixed(0)}Pa`);
                }
            }

            solver.solve(state, timeStep);

            // store results at output freq
            if (step % outputFrequency === 0 || step === steps - 1) {
                const interpolationMethod = (useEvolvingConditions && evolvingConditions.interpolationMethod) || 'linear';
                const currentTemp = useEvolvingConditions
                    ? interpolateValue(currentTime, evolvingConditions.times, evolvingConditions.temperature, temperature, interpolationMethod)
                    : temperature;
                const currentPressure = useEvolvingConditions
                    ? interpolateValue(currentTime, evolvingConditions.times, evolvingConditions.pressure, pressure, interpolationMethod)
                    : pressure;

                results.push({
                    time: currentTime,
                    concentrations: state.getConcentrations(),
                });
                environmentalData.push({
                    time: currentTime,
                    temperature: currentTemp,
                    pressure: currentPressure,
                });
            }
        }

        // store sim results
        const simulationData = {
            id: simulationId,
            mechanism,
            parameters: {
                temperature,
                pressure,
                timeStep,
                duration,
                outputFrequency,
                evolvingConditions: useEvolvingConditions ? evolvingConditions : null,
            },
            initialConcentrations,
            rateConstants,
            results,
            environmentalData,
            status: 'completed',
            createdAt: new Date().toISOString(),
        };

        // add to cache
        simulations.set(simulationId, simulationData);

        // remove oldest if cache too big
        if (simulations.size > MAX_SIMULATIONS) {
            const firstKey = simulations.keys().next().value;
            simulations.delete(firstKey);
            console.log(`   Evicted old simulation from cache: ${firstKey}`);
        }

        console.log(`Simulation ${simulationId} completed - ${results.length} output points`);

        // return results
        res.json({
            success: true,
            simulationId,
            results,
            environmentalData,
            metadata: {
                mechanism,
                temperature: useEvolvingConditions ? 'EVOLVING' : temperature,
                pressure: useEvolvingConditions ? 'EVOLVING' : pressure,
                timeStep,
                duration,
                outputPoints: results.length,
                rateConstants: runtimeRates || {},
                evolvingConditions: useEvolvingConditions,
            },
        });

        // CRITICAL: delayed cleanup to avoid crashes
        // wait for response to send before cleanup
        // 100ms prevents destructor crashes
        setTimeout(() => {
            try {
                // clear refs for gc
                // delay ensures response finishes sending
                solver = null;
                state = null;
                console.log(`   Cleaned up C++ objects for simulation ${simulationId}`);
            } catch (err) {
                // ignore cleanup errors
                console.error(`   Cleanup error (non-fatal):`, err.message);
            }
        }, 100); // 100ms delay

    } catch (error) {
        console.error('Error running simulation:', error);

        res.status(500).json({
            success: false,
            error: 'Simulation failed',
            message: error.message,
            stack: process.env.NODE_ENV === 'development' ? error.stack : undefined,
        });
    } finally {
        if (tempConfigPath && fs.existsSync(tempConfigPath)) {
            fs.unlinkSync(tempConfigPath);
            console.log(`   Cleaned up temp file: ${tempConfigPath}`);
        }

        // NOTE: We do NOT explicitly null out state and solver here
        // The C++ destructors have bugs causing double-free errors
        // Let JavaScript garbage collector handle cleanup naturally
        // This may cause minor memory leaks, but prevents crashes
    }
});

// GET /api/simulation/:id - Get simulation results
router.get('/:id', (req, res) => {
    try {
        const { id } = req.params;
        const simulation = simulations.get(id);

        if (!simulation) {
            return res.status(404).json({
                success: false,
                error: 'Simulation not found',
                message: `No simulation found with id: ${id}`,
            });
        }

        res.json({
            success: true,
            simulation,
        });
    } catch (error) {
        console.error('Error getting simulation:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to get simulation',
            message: error.message,
        });
    }
});

// GET /api/simulation - List all simulations
router.get('/', (req, res) => {
    try {
        const simulationsList = Array.from(simulations.values()).map(sim => ({
            id: sim.id,
            mechanism: sim.mechanism,
            status: sim.status,
            createdAt: sim.createdAt,
            outputPoints: sim.results.length,
        }));

        res.json({
            success: true,
            simulations: simulationsList,
            count: simulationsList.length,
        });
    } catch (error) {
        console.error('Error listing simulations:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to list simulations',
            message: error.message,
        });
    }
});

// DELETE /api/simulation/:id - Delete a simulation
router.delete('/:id', (req, res) => {
    try {
        const { id } = req.params;
        const deleted = simulations.delete(id);

        if (!deleted) {
            return res.status(404).json({
                success: false,
                error: 'Simulation not found',
            });
        }

        res.json({
            success: true,
            message: 'Simulation deleted',
        });
    } catch (error) {
        console.error('Error deleting simulation:', error);
        res.status(500).json({
            success: false,
            error: 'Failed to delete simulation',
            message: error.message,
        });
    }
});

export default router;
