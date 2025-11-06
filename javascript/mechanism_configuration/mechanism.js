// ========== MECHANISM ==========
class Mechanism {
    constructor({ name = "Custom Mechanism", species = [], phases = [], reactions = [] }) {
        this.name = name;
        this.species = species;
        this.phases = phases;
        this.reactions = reactions;
    }

    getJSON() {
        return {
            "version": "1.0.0",
            "name": this.name,
            "species": this.species.map(s => s.getJSON()),
            "phases": this.phases.map(p => p.getJSON()),
            "reactions": this.reactions.map(r => r.getJSON())
        };
    }
}

module.exports = { Mechanism };