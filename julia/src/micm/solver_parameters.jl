# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

"""
    RosenbrockSolverParameters

Parameters for configuring Rosenbrock solvers.

# Fields
- `relative_tolerance::Float64`: Relative tolerance (default: 1e-6)
- `absolute_tolerances::Union{Vector{Float64}, Nothing}`: Per-species absolute tolerances, or nothing for MICM defaults
- `h_min::Float64`: Minimum step size in seconds (default: 0.0, solver chooses)
- `h_max::Float64`: Maximum step size in seconds (default: 0.0, solver chooses)
- `h_start::Float64`: Initial step size in seconds (default: 0.0, solver chooses)
- `max_number_of_steps::Int`: Maximum number of internal steps (default: 1000)
"""
struct RosenbrockSolverParameters
    relative_tolerance::Float64
    absolute_tolerances::Union{Vector{Float64}, Nothing}
    h_min::Float64
    h_max::Float64
    h_start::Float64
    max_number_of_steps::Int
end

function RosenbrockSolverParameters(;
    relative_tolerance::Real=1e-6,
    absolute_tolerances::Union{Vector{Float64}, Nothing}=nothing,
    h_min::Real=0.0,
    h_max::Real=0.0,
    h_start::Real=0.0,
    max_number_of_steps::Int=1000,
)
    return RosenbrockSolverParameters(
        Float64(relative_tolerance), absolute_tolerances,
        Float64(h_min), Float64(h_max), Float64(h_start),
        max_number_of_steps,
    )
end

function to_cpp_rosenbrock(params::RosenbrockSolverParameters)
    cpp = CppRosenbrockParams()
    set_relative_tolerance!(cpp, params.relative_tolerance)
    set_h_min!(cpp, params.h_min)
    set_h_max!(cpp, params.h_max)
    set_h_start!(cpp, params.h_start)
    set_max_number_of_steps!(cpp, params.max_number_of_steps)
    if params.absolute_tolerances !== nothing
        set_absolute_tolerances!(cpp, params.absolute_tolerances)
    end
    return cpp
end

function from_cpp_rosenbrock(cpp::CppRosenbrockParams)
    abs_tols = get_absolute_tolerances(cpp)
    return RosenbrockSolverParameters(
        relative_tolerance=get_relative_tolerance(cpp),
        absolute_tolerances=isempty(abs_tols) ? nothing : Vector{Float64}(abs_tols),
        h_min=get_h_min(cpp),
        h_max=get_h_max(cpp),
        h_start=get_h_start(cpp),
        max_number_of_steps=Int(get_max_number_of_steps(cpp)),
    )
end

"""
    BackwardEulerSolverParameters

Parameters for configuring Backward Euler solvers.

# Fields
- `relative_tolerance::Float64`: Relative tolerance (default: 1e-6)
- `absolute_tolerances::Union{Vector{Float64}, Nothing}`: Per-species absolute tolerances, or nothing for MICM defaults
- `max_number_of_steps::Int`: Maximum number of internal steps (default: 11)
- `time_step_reductions::Vector{Float64}`: Factors for time step reduction after failed solves (must have 5 elements)
"""
struct BackwardEulerSolverParameters
    relative_tolerance::Float64
    absolute_tolerances::Union{Vector{Float64}, Nothing}
    max_number_of_steps::Int
    time_step_reductions::Vector{Float64}
end

function BackwardEulerSolverParameters(;
    relative_tolerance::Real=1e-6,
    absolute_tolerances::Union{Vector{Float64}, Nothing}=nothing,
    max_number_of_steps::Int=11,
    time_step_reductions::Vector{Float64}=[0.5, 0.5, 0.5, 0.5, 0.1],
)
    length(time_step_reductions) == 5 || error("time_step_reductions must have exactly 5 elements, got $(length(time_step_reductions))")
    return BackwardEulerSolverParameters(
        Float64(relative_tolerance), absolute_tolerances,
        max_number_of_steps, time_step_reductions,
    )
end

function to_cpp_backward_euler(params::BackwardEulerSolverParameters)
    cpp = CppBackwardEulerParams()
    set_relative_tolerance!(cpp, params.relative_tolerance)
    set_max_number_of_steps!(cpp, params.max_number_of_steps)
    set_time_step_reductions!(cpp, params.time_step_reductions)
    if params.absolute_tolerances !== nothing
        set_absolute_tolerances!(cpp, params.absolute_tolerances)
    end
    return cpp
end

function from_cpp_backward_euler(cpp::CppBackwardEulerParams)
    abs_tols = get_absolute_tolerances(cpp)
    return BackwardEulerSolverParameters(
        relative_tolerance=get_relative_tolerance(cpp),
        absolute_tolerances=isempty(abs_tols) ? nothing : Vector{Float64}(abs_tols),
        max_number_of_steps=Int(get_max_number_of_steps(cpp)),
        time_step_reductions=Vector{Float64}(get_time_step_reductions(cpp)),
    )
end
