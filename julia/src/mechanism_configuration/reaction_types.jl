# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

# Reaction types for building a version 1 mechanism configuration in code.
#
# These mirror the JavaScript and Python interfaces. Each reaction is a plain data
# object with a `to_dict` method that produces the spaced-key schema expected by the
# MICM parser. The `LambdaRateConstant` reaction is intentionally not provided: it
# relies on a C++ lambda string / host-language callback that has no Julia analog.

# Convert a vector of `ReactionComponent`s to a vector of dictionaries.
_components(v) = [to_dict(c) for c in v]

# Add the optional `name` and `gas phase` keys, omitting them when `nothing`.
function _add_common!(d::Dict{String,Any}, name, gas_phase)
    name === nothing || (d["name"] = name)
    gas_phase === nothing || (d["gas phase"] = gas_phase)
    return d
end

"""
    Arrhenius(; A=1.0, B=0.0, C=0.0, D=300.0, E=0.0, Ea=nothing,
                reactants, products, name=nothing, gas_phase=nothing,
                other_properties=Dict())

Arrhenius-type rate constant: `k = A * exp(C/T) * (T/D)^B * (1 + E*P)`.
`C` and `Ea` are mutually exclusive; if `Ea` is given it is emitted in place of `C`.
"""
struct Arrhenius
    A::Float64
    B::Float64
    C::Float64
    D::Float64
    E::Float64
    Ea::Union{Float64,Nothing}
    reactants::Vector{ReactionComponent}
    products::Vector{ReactionComponent}
    name::Union{String,Nothing}
    gas_phase::Union{String,Nothing}
    other_properties::Dict{String,Any}
end

function Arrhenius(;
    A::Real = 1.0,
    B::Real = 0.0,
    C::Real = 0.0,
    D::Real = 300.0,
    E::Real = 0.0,
    Ea::Union{Real,Nothing} = nothing,
    reactants::AbstractVector = ReactionComponent[],
    products::AbstractVector = ReactionComponent[],
    name::Union{AbstractString,Nothing} = nothing,
    gas_phase::Union{AbstractString,Nothing} = nothing,
    other_properties::AbstractDict = Dict{String,Any}(),
)
    return Arrhenius(
        A,
        B,
        C,
        D,
        E,
        Ea === nothing ? nothing : Float64(Ea),
        collect(ReactionComponent, reactants),
        collect(ReactionComponent, products),
        name === nothing ? nothing : String(name),
        gas_phase === nothing ? nothing : String(gas_phase),
        Dict{String,Any}(other_properties),
    )
end

function to_dict(r::Arrhenius)
    d = Dict{String,Any}(
        "type" => "ARRHENIUS",
        "A" => r.A,
        "B" => r.B,
        "D" => r.D,
        "E" => r.E,
    )
    r.Ea === nothing ? (d["C"] = r.C) : (d["Ea"] = r.Ea)
    _add_common!(d, r.name, r.gas_phase)
    d["reactants"] = _components(r.reactants)
    d["products"] = _components(r.products)
    _merge_other_properties!(d, r.other_properties)
    return d
end

"""
    Branched(; X=nothing, Y=nothing, a0=nothing, n=nothing,
               reactants, nitrate_products, alkoxy_products,
               name=nothing, gas_phase=nothing, other_properties=Dict())

Branched (Wennberg NO + RO2) reaction.
"""
struct Branched
    X::Union{Float64,Nothing}
    Y::Union{Float64,Nothing}
    a0::Union{Float64,Nothing}
    n::Union{Float64,Nothing}
    reactants::Vector{ReactionComponent}
    nitrate_products::Vector{ReactionComponent}
    alkoxy_products::Vector{ReactionComponent}
    name::Union{String,Nothing}
    gas_phase::Union{String,Nothing}
    other_properties::Dict{String,Any}
end

function Branched(;
    X::Union{Real,Nothing} = nothing,
    Y::Union{Real,Nothing} = nothing,
    a0::Union{Real,Nothing} = nothing,
    n::Union{Real,Nothing} = nothing,
    reactants::AbstractVector = ReactionComponent[],
    nitrate_products::AbstractVector = ReactionComponent[],
    alkoxy_products::AbstractVector = ReactionComponent[],
    name::Union{AbstractString,Nothing} = nothing,
    gas_phase::Union{AbstractString,Nothing} = nothing,
    other_properties::AbstractDict = Dict{String,Any}(),
)
    f(x) = x === nothing ? nothing : Float64(x)
    return Branched(
        f(X),
        f(Y),
        f(a0),
        f(n),
        collect(ReactionComponent, reactants),
        collect(ReactionComponent, nitrate_products),
        collect(ReactionComponent, alkoxy_products),
        name === nothing ? nothing : String(name),
        gas_phase === nothing ? nothing : String(gas_phase),
        Dict{String,Any}(other_properties),
    )
end

function to_dict(r::Branched)
    d = Dict{String,Any}("type" => "BRANCHED_NO_RO2")
    r.X === nothing || (d["X"] = r.X)
    r.Y === nothing || (d["Y"] = r.Y)
    r.a0 === nothing || (d["a0"] = r.a0)
    r.n === nothing || (d["n"] = r.n)
    _add_common!(d, r.name, r.gas_phase)
    d["reactants"] = _components(r.reactants)
    d["nitrate products"] = _components(r.nitrate_products)
    d["alkoxy products"] = _components(r.alkoxy_products)
    _merge_other_properties!(d, r.other_properties)
    return d
end

"""
    Emission(; scaling_factor=1.0, products, name=nothing, gas_phase=nothing,
               other_properties=Dict())

Emission reaction (zeroth-order production), scaled by a user-defined rate parameter.
"""
struct Emission
    scaling_factor::Float64
    products::Vector{ReactionComponent}
    name::Union{String,Nothing}
    gas_phase::Union{String,Nothing}
    other_properties::Dict{String,Any}
end

function Emission(;
    scaling_factor::Real = 1.0,
    products::AbstractVector = ReactionComponent[],
    name::Union{AbstractString,Nothing} = nothing,
    gas_phase::Union{AbstractString,Nothing} = nothing,
    other_properties::AbstractDict = Dict{String,Any}(),
)
    return Emission(
        Float64(scaling_factor),
        collect(ReactionComponent, products),
        name === nothing ? nothing : String(name),
        gas_phase === nothing ? nothing : String(gas_phase),
        Dict{String,Any}(other_properties),
    )
end

function to_dict(r::Emission)
    d = Dict{String,Any}("type" => "EMISSION", "scaling factor" => r.scaling_factor)
    _add_common!(d, r.name, r.gas_phase)
    d["products"] = _components(r.products)
    _merge_other_properties!(d, r.other_properties)
    return d
end

"""
    FirstOrderLoss(; scaling_factor=1.0, reactants, name=nothing, gas_phase=nothing,
                     other_properties=Dict())

First-order loss reaction, scaled by a user-defined rate parameter.
"""
struct FirstOrderLoss
    scaling_factor::Float64
    reactants::Vector{ReactionComponent}
    name::Union{String,Nothing}
    gas_phase::Union{String,Nothing}
    other_properties::Dict{String,Any}
end

function FirstOrderLoss(;
    scaling_factor::Real = 1.0,
    reactants::AbstractVector = ReactionComponent[],
    name::Union{AbstractString,Nothing} = nothing,
    gas_phase::Union{AbstractString,Nothing} = nothing,
    other_properties::AbstractDict = Dict{String,Any}(),
)
    return FirstOrderLoss(
        Float64(scaling_factor),
        collect(ReactionComponent, reactants),
        name === nothing ? nothing : String(name),
        gas_phase === nothing ? nothing : String(gas_phase),
        Dict{String,Any}(other_properties),
    )
end

function to_dict(r::FirstOrderLoss)
    d = Dict{String,Any}("type" => "FIRST_ORDER_LOSS", "scaling factor" => r.scaling_factor)
    _add_common!(d, r.name, r.gas_phase)
    d["reactants"] = _components(r.reactants)
    _merge_other_properties!(d, r.other_properties)
    return d
end

"""
    Photolysis(; scaling_factor=1.0, reactants, products, name=nothing,
                 gas_phase=nothing, other_properties=Dict())

Photolysis reaction, scaled by a user-defined photolysis rate parameter.
"""
struct Photolysis
    scaling_factor::Float64
    reactants::Vector{ReactionComponent}
    products::Vector{ReactionComponent}
    name::Union{String,Nothing}
    gas_phase::Union{String,Nothing}
    other_properties::Dict{String,Any}
end

function Photolysis(;
    scaling_factor::Real = 1.0,
    reactants::AbstractVector = ReactionComponent[],
    products::AbstractVector = ReactionComponent[],
    name::Union{AbstractString,Nothing} = nothing,
    gas_phase::Union{AbstractString,Nothing} = nothing,
    other_properties::AbstractDict = Dict{String,Any}(),
)
    return Photolysis(
        Float64(scaling_factor),
        collect(ReactionComponent, reactants),
        collect(ReactionComponent, products),
        name === nothing ? nothing : String(name),
        gas_phase === nothing ? nothing : String(gas_phase),
        Dict{String,Any}(other_properties),
    )
end

function to_dict(r::Photolysis)
    d = Dict{String,Any}("type" => "PHOTOLYSIS", "scaling factor" => r.scaling_factor)
    _add_common!(d, r.name, r.gas_phase)
    d["reactants"] = _components(r.reactants)
    d["products"] = _components(r.products)
    _merge_other_properties!(d, r.other_properties)
    return d
end

"""
    Surface(; reaction_probability=1.0, gas_phase_species, gas_phase_products,
              name=nothing, gas_phase=nothing, other_properties=Dict())

Surface (heterogeneous) reaction. `gas_phase_species` is a single `ReactionComponent`;
only its species name is written to the configuration.
"""
struct Surface
    reaction_probability::Float64
    gas_phase_species::ReactionComponent
    gas_phase_products::Vector{ReactionComponent}
    name::Union{String,Nothing}
    gas_phase::Union{String,Nothing}
    other_properties::Dict{String,Any}
end

function Surface(;
    reaction_probability::Real = 1.0,
    gas_phase_species::ReactionComponent,
    gas_phase_products::AbstractVector = ReactionComponent[],
    name::Union{AbstractString,Nothing} = nothing,
    gas_phase::Union{AbstractString,Nothing} = nothing,
    other_properties::AbstractDict = Dict{String,Any}(),
)
    return Surface(
        Float64(reaction_probability),
        gas_phase_species,
        collect(ReactionComponent, gas_phase_products),
        name === nothing ? nothing : String(name),
        gas_phase === nothing ? nothing : String(gas_phase),
        Dict{String,Any}(other_properties),
    )
end

function to_dict(r::Surface)
    d = Dict{String,Any}(
        "type" => "SURFACE",
        "reaction probability" => r.reaction_probability,
    )
    _add_common!(d, r.name, r.gas_phase)
    d["gas-phase species"] = r.gas_phase_species.species_name
    d["gas-phase products"] = _components(r.gas_phase_products)
    _merge_other_properties!(d, r.other_properties)
    return d
end

"""
    TaylorSeries(; A=1.0, B=0.0, C=0.0, D=300.0, E=0.0, Ea=nothing,
                   taylor_coefficients=[1.0], reactants, products,
                   name=nothing, gas_phase=nothing, other_properties=Dict())

Taylor-series rate constant. `C` and `Ea` are mutually exclusive.
"""
struct TaylorSeries
    A::Float64
    B::Float64
    C::Float64
    D::Float64
    E::Float64
    Ea::Union{Float64,Nothing}
    taylor_coefficients::Vector{Float64}
    reactants::Vector{ReactionComponent}
    products::Vector{ReactionComponent}
    name::Union{String,Nothing}
    gas_phase::Union{String,Nothing}
    other_properties::Dict{String,Any}
end

function TaylorSeries(;
    A::Real = 1.0,
    B::Real = 0.0,
    C::Real = 0.0,
    D::Real = 300.0,
    E::Real = 0.0,
    Ea::Union{Real,Nothing} = nothing,
    taylor_coefficients::AbstractVector = [1.0],
    reactants::AbstractVector = ReactionComponent[],
    products::AbstractVector = ReactionComponent[],
    name::Union{AbstractString,Nothing} = nothing,
    gas_phase::Union{AbstractString,Nothing} = nothing,
    other_properties::AbstractDict = Dict{String,Any}(),
)
    return TaylorSeries(
        A,
        B,
        C,
        D,
        E,
        Ea === nothing ? nothing : Float64(Ea),
        collect(Float64, taylor_coefficients),
        collect(ReactionComponent, reactants),
        collect(ReactionComponent, products),
        name === nothing ? nothing : String(name),
        gas_phase === nothing ? nothing : String(gas_phase),
        Dict{String,Any}(other_properties),
    )
end

function to_dict(r::TaylorSeries)
    d = Dict{String,Any}(
        "type" => "TAYLOR_SERIES",
        "A" => r.A,
        "B" => r.B,
        "D" => r.D,
        "E" => r.E,
    )
    r.Ea === nothing ? (d["C"] = r.C) : (d["Ea"] = r.Ea)
    d["taylor coefficients"] = r.taylor_coefficients
    _add_common!(d, r.name, r.gas_phase)
    d["reactants"] = _components(r.reactants)
    d["products"] = _components(r.products)
    _merge_other_properties!(d, r.other_properties)
    return d
end

"""
    Troe(; k0_A=1.0, k0_B=0.0, k0_C=0.0, kinf_A=1.0, kinf_B=0.0, kinf_C=0.0,
           Fc=0.6, N=1.0, reactants, products, name=nothing, gas_phase=nothing,
           other_properties=Dict())

Troe (falloff) reaction.
"""
struct Troe
    k0_A::Float64
    k0_B::Float64
    k0_C::Float64
    kinf_A::Float64
    kinf_B::Float64
    kinf_C::Float64
    Fc::Float64
    N::Float64
    reactants::Vector{ReactionComponent}
    products::Vector{ReactionComponent}
    name::Union{String,Nothing}
    gas_phase::Union{String,Nothing}
    other_properties::Dict{String,Any}
end

function Troe(;
    k0_A::Real = 1.0,
    k0_B::Real = 0.0,
    k0_C::Real = 0.0,
    kinf_A::Real = 1.0,
    kinf_B::Real = 0.0,
    kinf_C::Real = 0.0,
    Fc::Real = 0.6,
    N::Real = 1.0,
    reactants::AbstractVector = ReactionComponent[],
    products::AbstractVector = ReactionComponent[],
    name::Union{AbstractString,Nothing} = nothing,
    gas_phase::Union{AbstractString,Nothing} = nothing,
    other_properties::AbstractDict = Dict{String,Any}(),
)
    return Troe(
        k0_A,
        k0_B,
        k0_C,
        kinf_A,
        kinf_B,
        kinf_C,
        Fc,
        N,
        collect(ReactionComponent, reactants),
        collect(ReactionComponent, products),
        name === nothing ? nothing : String(name),
        gas_phase === nothing ? nothing : String(gas_phase),
        Dict{String,Any}(other_properties),
    )
end

function to_dict(r::Troe)
    d = Dict{String,Any}(
        "type" => "TROE",
        "k0_A" => r.k0_A,
        "k0_B" => r.k0_B,
        "k0_C" => r.k0_C,
        "kinf_A" => r.kinf_A,
        "kinf_B" => r.kinf_B,
        "kinf_C" => r.kinf_C,
        "Fc" => r.Fc,
        "N" => r.N,
    )
    _add_common!(d, r.name, r.gas_phase)
    d["reactants"] = _components(r.reactants)
    d["products"] = _components(r.products)
    _merge_other_properties!(d, r.other_properties)
    return d
end

"""
    TernaryChemicalActivation(; k0_A=1.0, k0_B=0.0, k0_C=0.0, kinf_A=1.0,
                                kinf_B=0.0, kinf_C=0.0, Fc=0.6, N=1.0,
                                reactants, products, name=nothing,
                                gas_phase=nothing, other_properties=Dict())

Ternary chemical activation reaction.
"""
struct TernaryChemicalActivation
    k0_A::Float64
    k0_B::Float64
    k0_C::Float64
    kinf_A::Float64
    kinf_B::Float64
    kinf_C::Float64
    Fc::Float64
    N::Float64
    reactants::Vector{ReactionComponent}
    products::Vector{ReactionComponent}
    name::Union{String,Nothing}
    gas_phase::Union{String,Nothing}
    other_properties::Dict{String,Any}
end

function TernaryChemicalActivation(;
    k0_A::Real = 1.0,
    k0_B::Real = 0.0,
    k0_C::Real = 0.0,
    kinf_A::Real = 1.0,
    kinf_B::Real = 0.0,
    kinf_C::Real = 0.0,
    Fc::Real = 0.6,
    N::Real = 1.0,
    reactants::AbstractVector = ReactionComponent[],
    products::AbstractVector = ReactionComponent[],
    name::Union{AbstractString,Nothing} = nothing,
    gas_phase::Union{AbstractString,Nothing} = nothing,
    other_properties::AbstractDict = Dict{String,Any}(),
)
    return TernaryChemicalActivation(
        k0_A,
        k0_B,
        k0_C,
        kinf_A,
        kinf_B,
        kinf_C,
        Fc,
        N,
        collect(ReactionComponent, reactants),
        collect(ReactionComponent, products),
        name === nothing ? nothing : String(name),
        gas_phase === nothing ? nothing : String(gas_phase),
        Dict{String,Any}(other_properties),
    )
end

function to_dict(r::TernaryChemicalActivation)
    d = Dict{String,Any}(
        "type" => "TERNARY_CHEMICAL_ACTIVATION",
        "k0_A" => r.k0_A,
        "k0_B" => r.k0_B,
        "k0_C" => r.k0_C,
        "kinf_A" => r.kinf_A,
        "kinf_B" => r.kinf_B,
        "kinf_C" => r.kinf_C,
        "Fc" => r.Fc,
        "N" => r.N,
    )
    _add_common!(d, r.name, r.gas_phase)
    d["reactants"] = _components(r.reactants)
    d["products"] = _components(r.products)
    _merge_other_properties!(d, r.other_properties)
    return d
end

"""
    Tunneling(; A=1.0, B=0.0, C=0.0, reactants, products, name=nothing,
                gas_phase=nothing, other_properties=Dict())

Wennberg tunneling reaction.
"""
struct Tunneling
    A::Float64
    B::Float64
    C::Float64
    reactants::Vector{ReactionComponent}
    products::Vector{ReactionComponent}
    name::Union{String,Nothing}
    gas_phase::Union{String,Nothing}
    other_properties::Dict{String,Any}
end

function Tunneling(;
    A::Real = 1.0,
    B::Real = 0.0,
    C::Real = 0.0,
    reactants::AbstractVector = ReactionComponent[],
    products::AbstractVector = ReactionComponent[],
    name::Union{AbstractString,Nothing} = nothing,
    gas_phase::Union{AbstractString,Nothing} = nothing,
    other_properties::AbstractDict = Dict{String,Any}(),
)
    return Tunneling(
        A,
        B,
        C,
        collect(ReactionComponent, reactants),
        collect(ReactionComponent, products),
        name === nothing ? nothing : String(name),
        gas_phase === nothing ? nothing : String(gas_phase),
        Dict{String,Any}(other_properties),
    )
end

function to_dict(r::Tunneling)
    d = Dict{String,Any}("type" => "TUNNELING", "A" => r.A, "B" => r.B, "C" => r.C)
    _add_common!(d, r.name, r.gas_phase)
    d["reactants"] = _components(r.reactants)
    d["products"] = _components(r.products)
    _merge_other_properties!(d, r.other_properties)
    return d
end

"""
    UserDefined(; scaling_factor=1.0, reactants, products, name=nothing,
                  gas_phase=nothing, other_properties=Dict())

User-defined reaction whose rate is supplied at runtime via a user-defined rate
parameter.
"""
struct UserDefined
    scaling_factor::Float64
    reactants::Vector{ReactionComponent}
    products::Vector{ReactionComponent}
    name::Union{String,Nothing}
    gas_phase::Union{String,Nothing}
    other_properties::Dict{String,Any}
end

function UserDefined(;
    scaling_factor::Real = 1.0,
    reactants::AbstractVector = ReactionComponent[],
    products::AbstractVector = ReactionComponent[],
    name::Union{AbstractString,Nothing} = nothing,
    gas_phase::Union{AbstractString,Nothing} = nothing,
    other_properties::AbstractDict = Dict{String,Any}(),
)
    return UserDefined(
        Float64(scaling_factor),
        collect(ReactionComponent, reactants),
        collect(ReactionComponent, products),
        name === nothing ? nothing : String(name),
        gas_phase === nothing ? nothing : String(gas_phase),
        Dict{String,Any}(other_properties),
    )
end

function to_dict(r::UserDefined)
    d = Dict{String,Any}("type" => "USER_DEFINED", "scaling factor" => r.scaling_factor)
    _add_common!(d, r.name, r.gas_phase)
    d["reactants"] = _components(r.reactants)
    d["products"] = _components(r.products)
    _merge_other_properties!(d, r.other_properties)
    return d
end
