// Conditions class for MICM

const { GAS_CONSTANT } = require('./utils');

/**
 * Represents environmental conditions for a grid cell
 */
class Conditions {
    /**
     * Create a Conditions object
     * @param {Object} options - Condition parameters
     * @param {number} options.temperature - Temperature in Kelvin
     * @param {number} options.pressure - Pressure in Pascals
     * @param {number} options.air_density - Air density in mol m^-3 (calculated from ideal gas law if not provided)
     */
    constructor({
        temperature = null,
        pressure = null,
        air_density = null,
    } = {}) {
        this.temperature = temperature;
        this.pressure = pressure;
        if (air_density !== null) {
            this.air_density = air_density;
        } else if (temperature !== null && pressure !== null) {
            // Calculate air density from ideal gas law: density = P / (R * T)
            this.air_density = pressure / (GAS_CONSTANT * temperature);
        } else {
            this.air_density = null;
        }
    }
}

module.exports = Conditions;
