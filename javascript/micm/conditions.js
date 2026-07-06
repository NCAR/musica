import { GAS_CONSTANT } from './utils.js';

/**
 * Environmental conditions for a single grid cell.
 *
 * If `air_density` is not provided and both `temperature` and `pressure` are
 * given, air density is computed from the Ideal Gas Law: ``P / (R * T)``.
 */
export class Conditions {
  /**
   * @param {Object} [options]
   * @param {number|null} [options.temperature=null] - Temperature in Kelvin
   * @param {number|null} [options.pressure=null] - Pressure in Pascals
   * @param {number|null} [options.air_density=null] - Air number density in mol m⁻³;
   *   computed from the Ideal Gas Law when `null` and both temperature and pressure are set
   */
  constructor({ temperature = null, pressure = null, air_density = null } = {}) {
    this.temperature = temperature;
    this.pressure = pressure;
    if (air_density !== null) {
      this.air_density = air_density;
    } else if (temperature !== null && pressure !== null) {
      this.air_density = 1.0 / ((GAS_CONSTANT * temperature) / pressure);
    } else {
      this.air_density = null;
    }
  }
}
