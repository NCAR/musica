// Utility functions for MUSICA JavaScript bindings

/**
 * Check if a value is a scalar number (not NaN, not boolean)
 * @param {*} x - Value to check
 * @returns {boolean} True if x is a scalar number
 */
function isScalarNumber(x) {
    return (
        (typeof x === 'number' || x instanceof Number) &&
        !Number.isNaN(x) &&
        !(typeof x === 'boolean')
    );
}

// Physical and chemical constants
const AVOGADRO = 6.02214076e23;  // mol^-1
const BOLTZMANN = 1.380649e-23;  // J K^-1
const GAS_CONSTANT = AVOGADRO * BOLTZMANN;  // J K^-1 mol^-1 (8.31446261815324)

module.exports = {
    isScalarNumber,
    AVOGADRO,
    BOLTZMANN,
    GAS_CONSTANT,
};
