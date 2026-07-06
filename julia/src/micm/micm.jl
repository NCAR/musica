# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

"""
    MICM

Wrapper around the C++ MICM chemical kinetics solver.

# Constructor

    MICM(; config_path=nothing, config_string=nothing,
           solver_type=RosenbrockStandardOrder, solver_parameters=nothing)

Provide exactly one of `config_path` or `config_string`.

- `config_path::String`: Path to a configuration file or directory
- `config_string::String`: A JSON or YAML string holding a valid mechanism configuration
- `solver_type::SolverType`: Type of solver to use
- `solver_parameters`: Optional `RosenbrockSolverParameters` or `BackwardEulerSolverParameters`
"""
mutable struct MICM
    _ptr::MICMPtr
    _solver_type::SolverType
    _vector_size::Int

    function MICM(;
        config_path::Union{AbstractString,Nothing} = nothing,
        config_string::Union{AbstractString,Nothing} = nothing,
        solver_type::SolverType = RosenbrockStandardOrder,
        solver_parameters::Union{
            RosenbrockSolverParameters,
            BackwardEulerSolverParameters,
            Nothing,
        } = nothing,
    )
        if (config_path === nothing) == (config_string === nothing)
            error("Provide exactly one of `config_path` or `config_string`")
        end

        vs = Int(cpp_get_vector_size(Int(solver_type)))
        vs > 0 || error("Invalid vector size: $vs")

        ptr = if config_path !== nothing
            cpp_create_solver(String(config_path), Int(solver_type))
        else
            cpp_create_solver_from_string(String(config_string), Int(solver_type))
        end

        obj = new(ptr, solver_type, vs)
        finalizer(obj) do m
            cpp_delete_solver(m._ptr)
        end

        if solver_parameters !== nothing
            set_solver_parameters!(obj, solver_parameters)
        end
        return obj
    end
end

"""
    solver_type(micm::MICM) -> SolverType

Get the type of solver used.
"""
solver_type(micm::MICM) = micm._solver_type

"""
    create_state(micm::MICM; number_of_grid_cells=1) -> State

Create a new state object for this solver.
"""
function create_state(micm::MICM; number_of_grid_cells::Int = 1)
    return State(micm._ptr, number_of_grid_cells, micm._vector_size, micm)
end

"""
    solve!(micm::MICM, state::State, time_step::Real) -> SolverResult

Solve the chemical system for the given state and time step (in seconds).
"""
function solve!(micm::MICM, state::State, time_step::Real)
    cpp_result = cpp_micm_solve(micm._ptr, state._ptr, Float64(time_step))
    return SolverResult(cpp_result)
end

"""
    get_species_property(micm::MICM, species_name, property_name, ::Type{T})

Get a property of a chemical species, returned as type `T`.

`T` selects which property kind to read and the return type:
`Float64`, `Int`, `Bool`, or `String`.

# Examples
```julia
mw   = get_species_property(micm, "O3", "molecular weight [kg mol-1]", Float64)
name = get_species_property(micm, "O3", "__long name", String)
gas  = get_species_property(micm, "O3", "__is gas", Bool)
n    = get_species_property(micm, "O3", "__atoms", Int)
```
"""
function get_species_property(
    micm::MICM,
    species_name::AbstractString,
    property_name::AbstractString,
    ::Type{Float64},
)
    return cpp_get_species_property_double(
        micm._ptr,
        String(species_name),
        String(property_name),
    )
end

function get_species_property(
    micm::MICM,
    species_name::AbstractString,
    property_name::AbstractString,
    ::Type{Int},
)
    return Int(
        cpp_get_species_property_int(
            micm._ptr,
            String(species_name),
            String(property_name),
        ),
    )
end

function get_species_property(
    micm::MICM,
    species_name::AbstractString,
    property_name::AbstractString,
    ::Type{Bool},
)
    return cpp_get_species_property_bool(
        micm._ptr,
        String(species_name),
        String(property_name),
    )
end

function get_species_property(
    micm::MICM,
    species_name::AbstractString,
    property_name::AbstractString,
    ::Type{String},
)
    return String(
        cpp_get_species_property_string(
            micm._ptr,
            String(species_name),
            String(property_name),
        ),
    )
end

"""
    set_solver_parameters!(micm::MICM, params::RosenbrockSolverParameters)
    set_solver_parameters!(micm::MICM, params::BackwardEulerSolverParameters)

Set solver-specific parameters. Parameter type must match the solver type.
"""
function set_solver_parameters!(micm::MICM, params::RosenbrockSolverParameters)
    cpp_params = to_cpp_rosenbrock(params)
    cpp_set_rosenbrock_params(micm._ptr, cpp_params)
end

function set_solver_parameters!(micm::MICM, params::BackwardEulerSolverParameters)
    cpp_params = to_cpp_backward_euler(params)
    cpp_set_backward_euler_params(micm._ptr, cpp_params)
end

"""
    get_solver_parameters(micm::MICM)

Get the current solver parameters. Returns `RosenbrockSolverParameters` or
`BackwardEulerSolverParameters` depending on the solver type.
"""
function get_solver_parameters(micm::MICM)
    st = micm._solver_type
    if st == Rosenbrock || st == RosenbrockStandardOrder || st == CudaRosenbrock
        return from_cpp_rosenbrock(cpp_get_rosenbrock_params(micm._ptr))
    elseif st == BackwardEuler || st == BackwardEulerStandardOrder
        return from_cpp_backward_euler(cpp_get_backward_euler_params(micm._ptr))
    else
        error("Unknown solver type: $st")
    end
end
