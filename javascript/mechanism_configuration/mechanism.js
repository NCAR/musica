// ========== MECHANISM ==========
class Mechanism {
    constructor({ name, version, species, phases, reactions }) {
        this.name = name;
        this.version = version;
        this.species = species;
        this.phases = phases;
        this.reactions = reactions;
    }
    getJSON() {
        let obj = {};
        obj['name'] = this.name;
        obj['version'] = this.version;
        obj['species'] = this.species.map(s => s.getJSON());
        obj['phases'] = this.phases.map(p => p.getJSON());
        obj['reactions'] = this.reactions.map(r => r.getJSON());
        return obj;
    }
    // Allows for one-step conversion to string to pass to C++
    getString() {
        return JSON.stringify(this.getJSON());
    }
}
module.exports = { Mechanism };
