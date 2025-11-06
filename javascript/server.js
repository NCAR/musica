// MUSICA Backend API Server
// Provides REST API endpoints for MusicBox Interactive frontend

const express = require('express');
const cors = require('cors');
const path = require('path');

// Import MUSICA bindings
const {
    MICM,
    SolverType,
    Species,
    ReactionComponent,
    Phase,
    Arrhenius,
    Photolysis,
    Emission,
    UserDefined,
    Mechanism
} = require('../index.js');

const app = express();
const PORT = process.env.PORT || 3001;

// Middleware
app.use(cors());
app.use(express.json({ limit: '10mb' })); // Support large mechanism configurations

// Available preset mechanisms
const PRESET_MECHANISMS = {
    ts1: {
        id: 'ts1',
        name: 'TS1',
        description: '209 species tropospheric chemistry mechanism',
        config_path: path.join(__dirname, '../configs/v0/ts1')
    },
    chapman: {
        id: 'chapman',
        name: 'Chapman',
        description: 'Stratospheric ozone chemistry',
        config_path: path.join(__dirname, '../configs/v1/chapman')
    },
    analytical: {
        id: 'analytical',
        name: 'Analytical',
        description: 'Simple analytical test case',
        config_path: path.join(__dirname, '../configs/v0/analytical')
    }
};

// ============================================================
// API ENDPOINTS
// ============================================================

// GET /api/health - Health check
app.get('/api/health', (req, res) => {
    res.json({
        status: 'ok',
        service: 'MUSICA API',
        version: '1.0.0',
        timestamp: new Date().toISOString()
    });
});

// GET /api/mechanisms - List available preset mechanisms
app.get('/api/mechanisms', (req, res) => {
    try {
        const mechanisms = Object.values(PRESET_MECHANISMS).map(m => ({
            id: m.id,
            name: m.name,
            description: m.description
        }));
        res.json({ success: true, mechanisms });
    } catch (error) {
        res.status(500).json({
            success: false,
            error: error.message
        });
    }
});

// POST /api/mechanism/create - Create custom mechanism from configuration
app.post('/api/mechanism/create', (req, res) => {
    try {
        const { name, species, phases, reactions } = req.body;

        if (!species || !Array.isArray(species)) {
            return res.status(400).json({
                success: false,
                error: 'Species array is required'
            });
        }

        // Create Species objects
        const speciesObjs = species.map(s => new Species({
            name: s.name,
            molecular_weight_kg_mol: s.molecular_weight_kg_mol,
            other_properties: s.other_properties || {}
        }));

        // Create Phase objects
        const phaseObjs = phases ? phases.map(p => new Phase({
            name: p.name,
            species: p.species
        })) : [];

        // Create Reaction objects
        const reactionObjs = [];
        if (reactions && Array.isArray(reactions)) {
            for (const r of reactions) {
                if (r.type === 'ARRHENIUS') {
                    reactionObjs.push(new Arrhenius({
                        name: r.name,
                        A: r.A || 1.0,
                        B: r.B || 0.0,
                        C: r.C || 0.0,
                        D: r.D || 300.0,
                        E: r.E || 0.0,
                        reactants: r.reactants || [],
                        products: r.products || [],
                        gas_phase: r.gas_phase
                    }));
                } else if (r.type === 'PHOTOLYSIS') {
                    reactionObjs.push(new Photolysis({
                        name: r.name,
                        reactants: r.reactants || [],
                        products: r.products || [],
                        gas_phase: r.gas_phase,
                        scaling_factor: r.scaling_factor || 1.0
                    }));
                }
                // Add more reaction types as needed
            }
        }

        // Create Mechanism
        const mechanism = new Mechanism({
            name: name || 'Custom Mechanism',
            species: speciesObjs,
            phases: phaseObjs,
            reactions: reactionObjs
        });

        // Serialize for response
        const mechanismData = mechanism.serialize();

        res.json({
            success: true,
            message: 'Mechanism created successfully',
            mechanism: mechanismData
        });
    } catch (error) {
        console.error('Error creating mechanism:', error);
        res.status(400).json({
            success: false,
            error: error.message,
            stack: process.env.NODE_ENV === 'development' ? error.stack : undefined
        });
    }
});

// POST /api/simulate - Run atmospheric chemistry simulation
app.post('/api/simulate', async (req, res) => {
    try {
        const {
            mechanismId,
            mechanism,
            conditions,
            initialConcentrations,
            userDefinedRateParameters,
            timeStep,
            numSteps,
            solverType
        } = req.body;

        // Validate required parameters
        if (!mechanismId && !mechanism) {
            return res.status(400).json({
                success: false,
                error: 'Either mechanismId or mechanism configuration is required'
            });
        }

        if (!conditions || !conditions.temperature || !conditions.pressure) {
            return res.status(400).json({
                success: false,
                error: 'Temperature and pressure conditions are required'
            });
        }

        if (!initialConcentrations) {
            return res.status(400).json({
                success: false,
                error: 'Initial concentrations are required'
            });
        }

        // Create MICM solver
        let micm;
        if (mechanismId) {
            // Use preset mechanism
            const preset = PRESET_MECHANISMS[mechanismId];
            if (!preset) {
                return res.status(404).json({
                    success: false,
                    error: `Mechanism '${mechanismId}' not found`
                });
            }

            micm = new MICM({
                config_path: preset.config_path,
                solver_type: solverType || SolverType.rosenbrock_standard_order
            });
        } else {
            // Use custom mechanism (mechanism object)
            // This would require reconstructing the Mechanism from JSON
            // For now, we'll return an error
            return res.status(501).json({
                success: false,
                error: 'Custom mechanism simulation not yet implemented. Use mechanismId for now.'
            });
        }

        // Create state
        const state = micm.createState(1);

        // Set conditions
        state.setConditions({
            temperatures: conditions.temperature,
            pressures: conditions.pressure
        });

        // Set initial concentrations
        state.setConcentrations(initialConcentrations);

        // Set user-defined rate parameters (photolysis rates, etc.)
        if (userDefinedRateParameters) {
            state.setUserDefinedRateParameters(userDefinedRateParameters);
        }

        // Run simulation
        const results = [];
        const steps = numSteps || 10;
        const dt = timeStep || 60.0; // Default 60 seconds

        for (let i = 0; i < steps; i++) {
            // Solve one time step
            micm.solve(state, dt);

            // Get concentrations
            const concentrations = state.getConcentrations();

            // Store results
            results.push({
                time: i * dt,
                concentrations
            });
        }

        res.json({
            success: true,
            simulation: {
                mechanismId: mechanismId || 'custom',
                timeStep: dt,
                numSteps: steps,
                results
            }
        });

    } catch (error) {
        console.error('Simulation error:', error);
        res.status(500).json({
            success: false,
            error: error.message,
            stack: process.env.NODE_ENV === 'development' ? error.stack : undefined
        });
    }
});

// POST /api/mechanism/validate - Validate mechanism configuration
app.post('/api/mechanism/validate', (req, res) => {
    try {
        const { mechanism } = req.body;

        const errors = [];
        const warnings = [];

        // Validate species
        if (!mechanism.species || mechanism.species.length === 0) {
            errors.push('Mechanism must have at least one species');
        }

        // Validate phases
        if (!mechanism.phases || mechanism.phases.length === 0) {
            warnings.push('Mechanism has no phases defined');
        }

        // Validate reactions
        if (!mechanism.reactions || mechanism.reactions.length === 0) {
            warnings.push('Mechanism has no reactions defined');
        }

        // Check for duplicate species names
        if (mechanism.species) {
            const names = mechanism.species.map(s => s.name);
            const duplicates = names.filter((name, index) => names.indexOf(name) !== index);
            if (duplicates.length > 0) {
                errors.push(`Duplicate species names: ${duplicates.join(', ')}`);
            }
        }

        res.json({
            valid: errors.length === 0,
            errors,
            warnings
        });
    } catch (error) {
        res.status(400).json({
            success: false,
            error: error.message
        });
    }
});

// GET /api/species/:mechanismId - Get species list for a mechanism
app.get('/api/species/:mechanismId', (req, res) => {
    try {
        const { mechanismId } = req.params;
        const preset = PRESET_MECHANISMS[mechanismId];

        if (!preset) {
            return res.status(404).json({
                success: false,
                error: `Mechanism '${mechanismId}' not found`
            });
        }

        // Create MICM solver to get species info
        const micm = new MICM({
            config_path: preset.config_path,
            solver_type: SolverType.rosenbrock_standard_order
        });

        const state = micm.createState(1);
        const speciesOrdering = state.getSpeciesOrdering();

        res.json({
            success: true,
            mechanismId,
            species: Object.keys(speciesOrdering)
        });
    } catch (error) {
        res.status(500).json({
            success: false,
            error: error.message
        });
    }
});

// ============================================================
// ERROR HANDLING
// ============================================================

// 404 handler
app.use((req, res) => {
    res.status(404).json({
        success: false,
        error: 'Endpoint not found',
        path: req.path
    });
});

// Global error handler
app.use((err, req, res, next) => {
    console.error('Unhandled error:', err);
    res.status(500).json({
        success: false,
        error: 'Internal server error',
        message: err.message,
        stack: process.env.NODE_ENV === 'development' ? err.stack : undefined
    });
});

// ============================================================
// START SERVER
// ============================================================

app.listen(PORT, () => {
    console.log('============================================================');
    console.log('  MUSICA API Server');
    console.log('============================================================');
    console.log(`  Status: Running`);
    console.log(`  Port: ${PORT}`);
    console.log(`  URL: http://localhost:${PORT}`);
    console.log('');
    console.log('  Available endpoints:');
    console.log('    GET  /api/health');
    console.log('    GET  /api/mechanisms');
    console.log('    GET  /api/species/:mechanismId');
    console.log('    POST /api/mechanism/create');
    console.log('    POST /api/mechanism/validate');
    console.log('    POST /api/simulate');
    console.log('============================================================');
    console.log('');
});

// Graceful shutdown
process.on('SIGTERM', () => {
    console.log('SIGTERM received. Shutting down gracefully...');
    process.exit(0);
});

process.on('SIGINT', () => {
    console.log('\nSIGINT received. Shutting down gracefully...');
    process.exit(0);
});
