// MUSICA - Main entry point
// Loads the modular JavaScript API

const { MICM, SolverType } = require('./javascript/micm/micm');
const State = require('./javascript/micm/state');
const Conditions = require('./javascript/micm/conditions');
const { GAS_CONSTANT, AVOGADRO, BOLTZMANN } = require('./javascript/micm/utils');

// Load mechanism configuration classes
const addon = require('./build/Release/musica.node');
const Species = addon.Species;
const ReactionComponent = addon.ReactionComponent;
const Phase = addon.Phase;
const Arrhenius = addon.Arrhenius;
const Photolysis = addon.Photolysis;
const Emission = addon.Emission;
const UserDefined = addon.UserDefined;
const Mechanism = addon.Mechanism;

module.exports = {
    // MICM solver classes
    MICM,
    State,
    Conditions,
    SolverType,

    // Mechanism configuration classes
    Species,
    ReactionComponent,
    Phase,
    Arrhenius,
    Photolysis,
    Emission,
    UserDefined,
    Mechanism,

    // Constants
    GAS_CONSTANT,
    AVOGADRO,
    BOLTZMANN,
};

// Support for default ESM import: import musica from 'musica'
module.exports.default = module.exports;
