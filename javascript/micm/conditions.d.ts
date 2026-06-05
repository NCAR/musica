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
    constructor({ temperature, pressure, air_density }?: {
        temperature?: number | null | undefined;
        pressure?: number | null | undefined;
        air_density?: number | null | undefined;
    } | undefined);
    temperature: number | null;
    pressure: number | null;
    air_density: number | null;
}
