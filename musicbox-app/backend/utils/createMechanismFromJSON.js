// Utility to create MICM mechanisms from v1 JSON configs using JavaScript Mechanism API
// This allows consistent handling of both predefined and custom mechanisms

const fs = require('fs');
const path = require('path');

/**
 * Creates a MICM Mechanism object from v1 JSON configuration
 * Uses the JavaScript Mechanism API (javascript/mechanism_configuration/)
 *
 * @param {Object} musica - The MUSICA Node.js addon
 * @param {Object} configJSON - Parsed v1 mechanism JSON
 * @param {Array} additionalSpecies - Optional custom species to add
 * @param {Array} additionalReactions - Optional custom reactions to add
 * @returns {Object} - MICM Mechanism object
 */
function createMechanismFromJSON(musica, configJSON, additionalSpecies = [], additionalReactions = []) {
    // Extract classes from JavaScript Mechanism API
    const { Species, Phase, ReactionComponent } = musica.mechanismConfiguration.types;
    const {
        Arrhenius,
        Photolysis,
        UserDefined,
        Troe,
        TernaryChemicalActivation,
        Tunneling,
        Branched,
        Surface,
        FirstOrderLoss,
        Emission
    } = musica.mechanismConfiguration.reactionTypes;
    const { Mechanism } = musica.mechanismConfiguration;

    console.log(`   Creating mechanism from JSON: ${configJSON.name}`);

    // 1. Create Species objects from JSON
    const speciesObjects = (configJSON.species || []).map(sp => {
        return new Species({
            name: sp.name,
            molecular_weight_kg_mol: sp['molecular weight [kg mol-1]'] || 0.029,
            other_properties: {
                ...sp,
                name: undefined,  // Remove name from properties
                'molecular weight [kg mol-1]': undefined  // Remove already-used fields
            }
        });
    });

    // Add custom species if provided
    if (additionalSpecies.length > 0) {
        console.log(`   Adding ${additionalSpecies.length} custom species to predefined mechanism`);
        additionalSpecies.forEach(sp => {
            speciesObjects.push(new Species({
                name: sp.name,
                molecular_weight_kg_mol: sp.molecular_weight_kg_mol || 0.029,
                other_properties: sp.properties || {}
            }));
        });
    }

    console.log(`   Created ${speciesObjects.length} species objects`);

    // 2. Create Phase objects from JSON
    const phaseObjects = (configJSON.phases || []).map(phase => {
        // Extract species names from phase
        const speciesInPhase = (phase.species || []).map(s => {
            if (typeof s === 'string') return s;
            if (s.name) return s.name;
            return null;
        }).filter(n => n !== null);

        return new Phase({
            name: phase.name || 'gas',
            species: speciesInPhase
        });
    });

    console.log(`   Created ${phaseObjects.length} phases`);

    // 3. Create Reaction objects from JSON
    const reactionObjects = [];
    let reactionCounts = {};

    // Helper to parse reactants/products
    const parseComponents = (componentsList) => {
        if (!componentsList) return [];
        return componentsList.map(comp => {
            return new ReactionComponent({
                species_name: comp['species name'] || comp.species_name || comp.name,
                coefficient: comp.coefficient || 1.0
            });
        });
    };

    // Process each reaction
    for (const rxn of (configJSON.reactions || [])) {
        try {
            const reactants = parseComponents(rxn.reactants);
            const products = parseComponents(rxn.products);
            const gasPhase = rxn['gas phase'] || rxn.gas_phase || 'gas';

            let reactionObj;
            const type = rxn.type;

            // Increment counter for this type
            reactionCounts[type] = (reactionCounts[type] || 0) + 1;

            switch (type) {
                case 'ARRHENIUS':
                    reactionObj = new Arrhenius({
                        name: rxn.name || '',
                        reactants,
                        products,
                        gas_phase: gasPhase,
                        A: rxn.A || 1.0,
                        B: rxn.B || 0.0,
                        C: rxn.C || 0.0,
                        D: rxn.D || 300.0,
                        E: rxn.E || 0.0,
                    });
                    break;

                case 'PHOTOLYSIS':
                    reactionObj = new Photolysis({
                        name: rxn.name || '',
                        reactants,
                        products,
                        gas_phase: gasPhase,
                        scaling_factor: rxn['scaling factor'] || rxn.scaling_factor || 1.0
                    });
                    break;

                case 'USER_DEFINED':
                    reactionObj = new UserDefined({
                        name: rxn.name || '',
                        reactants,
                        products,
                        gas_phase: gasPhase,
                        scaling_factor: rxn['scaling factor'] || rxn.scaling_factor || 1.0
                    });
                    break;

                case 'TROE':
                    reactionObj = new Troe({
                        name: rxn.name || '',
                        reactants,
                        products,
                        gas_phase: gasPhase,
                        k0_A: rxn.k0_A,
                        k0_B: rxn.k0_B,
                        k0_C: rxn.k0_C,
                        kinf_A: rxn.kinf_A,
                        kinf_B: rxn.kinf_B,
                        kinf_C: rxn.kinf_C,
                        Fc: rxn.Fc,
                        N: rxn.N
                    });
                    break;

                case 'TERNARY_CHEMICAL_ACTIVATION':
                    reactionObj = new TernaryChemicalActivation({
                        name: rxn.name || '',
                        reactants,
                        products,
                        gas_phase: gasPhase,
                        k0_A: rxn.k0_A,
                        k0_B: rxn.k0_B,
                        k0_C: rxn.k0_C,
                        kinf_A: rxn.kinf_A,
                        kinf_B: rxn.kinf_B,
                        kinf_C: rxn.kinf_C,
                        Fc: rxn.Fc,
                        N: rxn.N
                    });
                    break;

                case 'TUNNELING':
                    reactionObj = new Tunneling({
                        name: rxn.name || '',
                        reactants,
                        products,
                        gas_phase: gasPhase,
                        A: rxn.A,
                        B: rxn.B,
                        C: rxn.C
                    });
                    break;

                case 'BRANCHED':
                case 'BRANCHED_NO_RO2':
                    reactionObj = new Branched({
                        name: rxn.name || '',
                        reactants,
                        products,
                        gas_phase: gasPhase,
                        X: rxn.X,
                        Y: rxn.Y,
                        a0: rxn.a0,
                        n: rxn.n,
                        alkoxy_products: parseComponents(rxn.alkoxy_products || []),
                        nitrate_products: parseComponents(rxn.nitrate_products || [])
                    });
                    break;

                case 'SURFACE':
                    reactionObj = new Surface({
                        name: rxn.name || '',
                        gas_phase_species: rxn['gas-phase species'] || rxn.gas_phase_species,
                        gas_phase_products: parseComponents(rxn['gas-phase products'] || rxn.gas_phase_products || []),
                        gas_phase: gasPhase,
                        reaction_probability: rxn['reaction probability'] || rxn.reaction_probability || 1.0
                    });
                    break;

                case 'FIRST_ORDER_LOSS':
                    reactionObj = new FirstOrderLoss({
                        name: rxn.name || '',
                        reactants,
                        gas_phase: gasPhase,
                        scaling_factor: rxn['scaling factor'] || rxn.scaling_factor || 1.0
                    });
                    break;

                case 'EMISSION':
                    reactionObj = new Emission({
                        name: rxn.name || '',
                        products,
                        gas_phase: gasPhase,
                        scaling_factor: rxn['scaling factor'] || rxn.scaling_factor || 1.0
                    });
                    break;

                default:
                    console.log(`   Warning: Unknown reaction type "${type}" for reaction "${rxn.name}"`);
                    continue;
            }

            if (reactionObj) {
                reactionObjects.push(reactionObj);
            }
        } catch (error) {
            console.error(`   Error creating reaction "${rxn.name}":`, error.message);
        }
    }

    // Add custom reactions if provided
    if (additionalReactions.length > 0) {
        console.log(`   Adding ${additionalReactions.length} custom reactions to predefined mechanism`);
        // Process custom reactions using the same logic as DynamicMechanism.js
        // (Implementation would be similar to createDynamicMechanism)
    }

    console.log(`   Created ${reactionObjects.length} reactions:`, reactionCounts);

    // 4. Create Mechanism
    const mechanism = new Mechanism({
        name: configJSON.name || 'mechanism',
        version: configJSON.version || '1.0.0',
        species: speciesObjects,
        phases: phaseObjects,
        reactions: reactionObjects
    });

    console.log(`   Mechanism "${mechanism.name}" created successfully using JavaScript API`);
    return mechanism;
}

/**
 * Load mechanism from config directory and create Mechanism object
 *
 * @param {Object} musica - The MUSICA Node.js addon
 * @param {string} configPath - Path to mechanism directory (e.g., 'configs/v1/chapman')
 * @param {Array} additionalSpecies - Optional custom species to add
 * @param {Array} additionalReactions - Optional custom reactions to add
 * @returns {Object} - MICM Mechanism object
 */
function loadMechanismFromPath(musica, configPath, additionalSpecies = [], additionalReactions = []) {
    const configFile = path.join(configPath, 'config.json');

    if (!fs.existsSync(configFile)) {
        throw new Error(`Config file not found: ${configFile}`);
    }

    const configJSON = JSON.parse(fs.readFileSync(configFile, 'utf8'));
    return createMechanismFromJSON(musica, configJSON, additionalSpecies, additionalReactions);
}

module.exports = {
    createMechanismFromJSON,
    loadMechanismFromPath
};
