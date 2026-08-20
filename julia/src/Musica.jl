# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

module Musica

using CxxWrap
using Musica_jll

# Library path is provided by Musica_jll (either the registered JLL or the
# local stub at julia/Musica_jll/ which points to the CMake build output).
const _lib_path = Musica_jll.libmusica_julia

# @wrapmodule must be at module top level so types are defined during precompilation
@wrapmodule(() -> _lib_path)

function __init__()
    @initcxx
end

# Type aliases for CxxWrap pointer types (used in MICM and State structs)
const MICMPtr = CxxWrap.CxxWrapCore.CxxPtr{CppMICM}
const StatePtr = CxxWrap.CxxWrapCore.CxxPtr{CppState}

# Include MICM submodule files (order matters for dependencies)
include("micm/constants.jl")
include("micm/solver.jl")
include("micm/solver_result.jl")
include("micm/conditions.jl")
include("micm/solver_parameters.jl")
include("micm/utils.jl")
include("micm/state.jl")
include("micm/micm.jl")

# Mechanism configuration (pure-Julia; builds config strings for MICM)
include("mechanism_configuration/mechanism_configuration.jl")
export MechanismConfiguration

"""
    tuvx_available() -> Bool

Whether the bindings were built with TUV-x support. TUV-x is an optional Fortran
component, so its functions (e.g. [`get_tuvx_version`](@ref)) and types (e.g.
`Grid`) are only defined when this returns `true`.
"""
tuvx_available() = isdefined(@__MODULE__, :get_tuvx_version)
export tuvx_available

# TUV-x submodule files. These reference types that @wrapmodule only defines
# when the library was built with TUV-x, so include them conditionally. Each
# file exports its own names; include splices them in at module top level.
if tuvx_available()
    include("tuvx/grid.jl")
    include("tuvx/grid_map.jl")
    include("tuvx/profile.jl")
    include("tuvx/profile_map.jl")
    include("tuvx/radiator.jl")
    include("tuvx/radiator_map.jl")
    include("tuvx/tuvx.jl")
    include("tuvx/v54.jl")
    include("tuvx/vts1.jl")
    export V54, VTS1
end

# Version
export get_musica_version
export get_micm_version
export get_tuvx_version

# Constants
export AVOGADRO, BOLTZMANN, GAS_CONSTANT

# Solver types
export SolverType
export Rosenbrock,
    RosenbrockStandardOrder, BackwardEuler, BackwardEulerStandardOrder, CudaRosenbrock

# Solver results
export SolverState, SolverStats, SolverResult
export NotYetCalled, Running, Converged, ConvergenceExceededMaxSteps
export StepSizeTooSmall, RepeatedlySingularMatrix, NaNDetected, InfDetected
export AcceptingUnconvergedIntegration

# Conditions
export Conditions

# Solver parameters
export RosenbrockSolverParameters, BackwardEulerSolverParameters

# MICM and State types
export MICM, State

# Functions
export create_state, solve!
export set_concentrations!, get_concentrations
export set_conditions!, get_conditions
export set_user_defined_rate_parameters!, get_user_defined_rate_parameters
export get_species_ordering, get_user_defined_rate_parameters_ordering
export set_solver_parameters!, get_solver_parameters
export get_species_property
export solver_type

end # module Musica
