// Utility to create MICM mechanisms dynamically from frontend species/reactions
// Follows the same pattern as loadV1Mechanism.js using the Mechanism API

/**
 * Creates a MICM mechanism object from species and reactions data
 * @param {Object} musica - The MUSICA Node.js addon
 * @param {Array} species - Array of species objects {name, molecular_weight_kg_mol}
 * @param {Array} reactions - Array of reaction objects {type, reactants, products, ...}
 * @returns {Object} - MICM Mechanism object
 */
function createDynamicMechanism(musica, species, reactions) {
    // Extract classes from new namespace structure (teammate's architecture)
    const { Species, Phase, ReactionComponent } = musica.mechanismConfiguration.types;
    const { Arrhenius, Photolysis, UserDefined } = musica.mechanismConfiguration.reactionTypes;
    const { Mechanism } = musica.mechanismConfiguration;

    console.log(`   Creating dynamic mechanism with ${species.length} species and ${reactions.length} reactions`);

    // Create Species objects
    const speciesObjects = species.map(sp => {
        return new Species({
            name: sp.name,
            molecular_weight_kg_mol: sp.molecular_weight_kg_mol || sp.molecularWeight || 0.029,
            other_properties: sp.properties || {}
        });
    });

    console.log(`   Created ${speciesObjects.length} species objects`);

    // Create Phase object (gas phase)
    const speciesNames = species.map(sp => sp.name);
    const phaseObjects = [new Phase({
        name: 'gas',
        species: speciesNames
    })];

    console.log(`   Created gas phase with species: ${speciesNames.join(', ')}`);

    // Create Reaction objects
    const reactionObjects = [];
    let arrheniusCount = 0;
    let photolysisCount = 0;
    let userDefinedCount = 0;
    let skippedCount = 0;

    for (const rxn of reactions) {
        try {
            console.log(`   DEBUG: Processing reaction:`, JSON.stringify(rxn, null, 2));

            // Parse reactants - create ReactionComponent objects
            const parseReactionComponents = (componentsList, componentType) => {
                console.log(`   DEBUG: ${componentType} input:`, JSON.stringify(componentsList));
                if (!componentsList || componentsList.length === 0) return [];

                return componentsList.map(item => {
                    let speciesName;
                    let coefficient = 1.0;

                    if (typeof item === 'string') {
                        // Simple format: "O3"
                        speciesName = item;
                    } else if (item.name) {
                        // Object format: {name: "O3", coefficient: 2}
                        speciesName = item.name;
                        coefficient = item.coefficient || 1.0;
                    } else if (item.species) {
                        // Alternative format: {species: "O3", coefficient: 2}
                        speciesName = item.species;
                        coefficient = item.coefficient || 1.0;
                    } else if (item['species name']) {
                        // v1 format: {"species name": "O3", coefficient: 2}
                        speciesName = item['species name'];
                        coefficient = item.coefficient || 1.0;
                    }

                    // Validate species exists
                    if (!speciesNames.includes(speciesName)) {
                        console.log(`   Warning: ${componentType} species "${speciesName}" not found in mechanism`);
                        return null;
                    }

                    return new ReactionComponent({
                        species_name: speciesName,
                        coefficient: coefficient
                    });
                }).filter(c => c !== null);
            };

            const reactants = parseReactionComponents(rxn.reactants, 'Reactant');
            const products = parseReactionComponents(rxn.products, 'Product');

            // Skip reaction if any species are missing
            if (reactants.length !== (rxn.reactants?.length || 0) ||
                products.length !== (rxn.products?.length || 0)) {
                console.log(`   Skipping reaction "${rxn.name}" - missing species`);
                skippedCount++;
                continue;
            }

            let reactionObj;

            switch (rxn.type) {
                case 'ARRHENIUS':
                    reactionObj = new Arrhenius({
                        name: rxn.name || '',
                        reactants,
                        products,
                        gas_phase: 'gas',
                        A: rxn.A || rxn.rateConstant?.A || 1.0,
                        B: rxn.B || rxn.rateConstant?.B || 0.0,
                        C: rxn.C || rxn.rateConstant?.C || 0.0,
                        D: rxn.D || rxn.rateConstant?.D || 300.0,
                        E: rxn.E || rxn.rateConstant?.E || 0.0,
                    });
                    arrheniusCount++;
                    break;

                case 'PHOTOLYSIS':
                    {
                        // Reaction name should be simple like Chapman's "jO2" style
                        // The PHOTO. prefix is added automatically by MICM for runtime parameters
                        const reactionName = `j${reactants[0].species_name}`;
                        const scalingFactor = rxn.scalingFactor || rxn.scaling_factor || rxn['scaling factor'] || 1.0;

                        console.log(`   Creating PHOTOLYSIS reaction "${reactionName}" with scaling factor: ${scalingFactor}`);
                        reactionObj = new Photolysis({
                            name: reactionName,
                            reactants,
                            products,
                            gas_phase: 'gas',
                            scaling_factor: scalingFactor
                        });
                        photolysisCount++;
                    }
                    break;

                case 'USER_DEFINED':
                    {
                        // Reaction name should be simple descriptive name (like Python tutorial: "complex_rxn")
                        // Runtime parameter will be USER.{reactionName}
                        const reactionName = reactants[0].species_name;
                        const scalingFactor = rxn.scalingFactor || rxn.scaling_factor || rxn['scaling factor'] || 1.0;

                        console.log(`   Creating USER_DEFINED reaction "${reactionName}" (runtime param: USER.${reactionName}) with scaling factor: ${scalingFactor}`);
                        reactionObj = new UserDefined({
                            name: reactionName,
                            reactants,
                            products,
                            gas_phase: 'gas',
                            scaling_factor: scalingFactor
                        });
                        userDefinedCount++;
                    }
                    break;

                default:
                    console.log(`   Skipping reaction "${rxn.name}" - unknown type: ${rxn.type}`);
                    skippedCount++;
                    continue;
            }

            if (reactionObj) {
                reactionObjects.push(reactionObj);
            }
        } catch (error) {
            console.error(`   Error creating reaction "${rxn.name}":`, error.message);
            skippedCount++;
        }
    }

    console.log(`   Created reactions: ${arrheniusCount} Arrhenius, ${photolysisCount} Photolysis, ${userDefinedCount} UserDefined`);
    if (skippedCount > 0) {
        console.log(`   Skipped ${skippedCount} reactions due to errors`);
    }

    // Create Mechanism
    const mechanism = new Mechanism({
        name: 'custom_mechanism',
        species: speciesObjects,
        phases: phaseObjects,
        reactions: reactionObjects
    });

    console.log(`   Dynamic mechanism created successfully`);
    return mechanism;
}

export { createDynamicMechanism };
