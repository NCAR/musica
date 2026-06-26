# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

"""
    SolverState

Enum representing the final state of the solver after a solve call.
"""
@enum SolverState begin
    NotYetCalled = 0
    Running = 1
    Converged = 2
    ConvergenceExceededMaxSteps = 3
    StepSizeTooSmall = 4
    RepeatedlySingularMatrix = 5
    NaNDetected = 6
    InfDetected = 7
    AcceptingUnconvergedIntegration = 8
end

"""
    SolverStats

Statistics from a solver run.
"""
struct SolverStats
    function_calls::Int
    jacobian_updates::Int
    number_of_steps::Int
    accepted::Int
    rejected::Int
    decompositions::Int
    solves::Int
    final_time::Float64
end

"""
    SolverResult

Combined result containing both solver state and statistics.
"""
struct SolverResult
    state::SolverState
    stats::SolverStats
end

function SolverResult(cpp_result::CppSolverResult)
    cpp_stats = get_solver_stats(cpp_result)
    stats = SolverStats(
        get_function_calls(cpp_stats),
        get_jacobian_updates(cpp_stats),
        get_number_of_steps(cpp_stats),
        get_accepted(cpp_stats),
        get_rejected(cpp_stats),
        get_decompositions(cpp_stats),
        get_solves(cpp_stats),
        get_final_time(cpp_stats),
    )
    state = SolverState(get_solver_state(cpp_result))
    return SolverResult(state, stats)
end

function Base.show(io::IO, r::SolverResult)
    print(
        io,
        "SolverResult(state=$(r.state), steps=$(r.stats.number_of_steps), final_time=$(r.stats.final_time))",
    )
end
