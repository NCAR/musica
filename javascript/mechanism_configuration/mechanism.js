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
        Object.assign(obj, this.reactions.getJSON());
        return obj;
    }
}
module.exports = { Mechanism };
