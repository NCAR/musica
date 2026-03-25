# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

module Musica

using CxxWrap
using Libdl

const lib_ext = Libdl.dlext
const lib_prefix = Sys.iswindows() ? "" : "lib"
const libmusica_julia =
    joinpath(@__DIR__, "..", "deps", "lib", "$(lib_prefix)musica_julia.$lib_ext")

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

# Version
export get_version

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
export solver_type

end # module Musica
