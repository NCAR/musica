// ========== MECHANISM ==========
class Mechanism {
    constructor({ name, version, species, phases, reactions }) {
        this.name = name;
        this.version = version || "1.0.0";
        this.species = species || [];
        this.phases = phases || [];
        this.reactions = reactions || [];
    }
    getJSON() {
        // Fixed: Use map() instead of for...in (for...in iterates keys, not values)
        return {
            name: this.name,
            version: this.version,
            species: this.species.map(s => s.getJSON()),
            phases: this.phases.map(p => p.getJSON()),
            reactions: this.reactions.map(r => r.getJSON())
        };
    }
}
module.exports = { Mechanism };
