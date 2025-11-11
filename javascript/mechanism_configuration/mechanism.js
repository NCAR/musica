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
        for (const s in this.species) Object.assign(obj, s.getJSON());
        for (const p in this.phases) Object.assign(obj, p.getJSON());
        Object.assign(obj, this.reactions.getJSON());
        return obj;
    }
}
module.exports = { Mechanism };
