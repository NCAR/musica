// ========== MECHANISM ==========
// REVIEW: Is this how'd I'd need to do this?
class Mechanism {
    constructor({ species, phases, reactions }) {
        this.species = species;
        this.phases = phases;
        this.reactions = reactions;
    }
    getJSON() {
        let obj = {};
        for (const s in this.species) Object.assign(obj, s.getJSON());
        for (const p in this.phases) Object.assign(obj, p.getJSON());
        for (const r in this.reactions) Object.assign(obj, r.getJSON());
        return obj;
    }
}
module.exports = { Mechanism };
