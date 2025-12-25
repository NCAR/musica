import { GAS_CONSTANT } from './utils.js';

export class Conditions {
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
            this.air_density = 1.0 / ((GAS_CONSTANT * temperature) / pressure);
        } else {
            this.air_density = null;
        }
    }
}