# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

module Musica

using CxxWrap

# Determine library path at compile time (must be top-level for @wrapmodule)
const _lib_path = let
    env_lib = get(ENV, "MUSICA_JULIA_LIB", nothing)
    if env_lib !== nothing && isfile(env_lib)
        env_lib
    else
        try
            @eval using Musica_jll
            Musica_jll.libmusica_julia
        catch
            error("MUSICA_JULIA_LIB environment variable must be set to the path of libmusica_julia.so " *
                  "(or install Musica_jll)")
        end
    end
end

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

# Version
export get_version

# Constants
export AVOGADRO, BOLTZMANN, GAS_CONSTANT

# Solver types
export SolverType
export Rosenbrock, RosenbrockStandardOrder, BackwardEuler, BackwardEulerStandardOrder, CudaRosenbrock

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
export solver_type

end # module Musica
