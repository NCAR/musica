# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

"""
    Musica.MechanismConfiguration

Build version 1 mechanism configurations in code and serialize them to JSON or YAML
strings that can be passed to `MICM(config_string = ...)`.

This mirrors the Python (`musica.mechanism_configuration`) and JavaScript
(`musica.mechanismConfiguration`) interfaces.

# Example
```julia
using Musica
using Musica.MechanismConfiguration

A  = Species(name = "A", molecular_weight = 0.029)
B  = Species(name = "B", molecular_weight = 0.029)
AB = Species(name = "AB", molecular_weight = 0.058)

gas = Phase(name = "gas", species = [A, B, AB])

loss = UserDefined(
    name = "AB loss",
    gas_phase = "gas",
    reactants = [ReactionComponent(species_name = "AB")],
    products = [ReactionComponent(species_name = "A"), ReactionComponent(species_name = "B")],
)

mech = Mechanism(name = "ABBA", version = "1.0.0",
                 species = [A, B, AB], phases = [gas], reactions = [loss])

micm = MICM(config_string = to_json_string(mech))
```
"""
module MechanismConfiguration

using JSON
using YAML

include("types.jl")
include("reaction_types.jl")
include("mechanism.jl")

# Core data types
export Species, PhaseSpecies, Phase, ReactionComponent

# Reaction types
export Arrhenius,
    Branched,
    Emission,
    FirstOrderLoss,
    Photolysis,
    Surface,
    TaylorSeries,
    Troe,
    TernaryChemicalActivation,
    Tunneling,
    UserDefined

# Mechanism and serialization
export Mechanism
export to_dict, to_json_string, to_yaml_string, to_string

end # module MechanismConfiguration
