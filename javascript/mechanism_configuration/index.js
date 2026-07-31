// mechanism_configuration/index.js
//
// Re-export everything from mechanism_configuration.js (not just the runtime
// values types/reactionTypes/Mechanism) so that consumers can also reach the
// exported type aliases — e.g. SpeciesParams, MechanismParams, the per-reaction
// *Params types, and the Reaction union — through the `mechanismConfiguration`
// namespace. The value surface is unchanged (mechanism_configuration.js only
// exports those same three values); this additionally forwards the types.
export * from './mechanism_configuration.js';
