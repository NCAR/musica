# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

"""
    State

Chemical state containing species concentrations, environmental conditions,
and user-defined rate parameters for one or more grid cells.

Wraps the C++ `musica::State` object and provides high-level accessors
with proper vector-ordering logic.
"""
mutable struct State
    _ptr::StatePtr
    _micm_ref::Any  # prevent GC of parent MICM before this State
    _species_ordering::Dict{String, Int}
    _rate_parameter_ordering::Dict{String, Int}
    _number_of_grid_cells::Int
    _vector_size::Int

    function State(solver_ptr, number_of_grid_cells::Int, vector_size::Int, micm_ref)
        number_of_grid_cells >= 1 || error("number_of_grid_cells must be >= 1")

        state_ptr = cpp_create_state(solver_ptr, Int64(number_of_grid_cells))

        # Build species ordering dict
        names = cpp_state_species_ordering_names(state_ptr)
        indices = cpp_state_species_ordering_indices(state_ptr)
        species_ordering = Dict{String, Int}()
        for i in 1:length(names)
            species_ordering[names[i]] = Int(indices[i])
        end

        # Build rate parameter ordering dict
        rnames = cpp_state_rate_param_ordering_names(state_ptr)
        rindices = cpp_state_rate_param_ordering_indices(state_ptr)
        rate_ordering = Dict{String, Int}()
        for i in 1:length(rnames)
            rate_ordering[rnames[i]] = Int(rindices[i])
        end

        obj = new(state_ptr, micm_ref, species_ordering, rate_ordering, number_of_grid_cells, vector_size)
        finalizer(obj) do s
            cpp_delete_state(s._ptr)
        end
        return obj
    end
end

"""
    set_concentrations!(state::State, concentrations::Dict{String})

Set species concentrations. Values can be scalars (for single grid cell)
or vectors matching the number of grid cells.
"""
function set_concentrations!(state::State, concentrations::Dict{String, <:Any})
    n_species = length(state._species_ordering)
    for (name, value) in concentrations
        haskey(state._species_ordering, name) || error("Species $name not found in the mechanism.")
        i_species = state._species_ordering[name]
        values = value isa Real ? [Float64(value)] : Float64.(value)
        length(values) == state._number_of_grid_cells || error(
            "Concentration list for $name must have length $(state._number_of_grid_cells).")
        for i_cell in 1:state._number_of_grid_cells
            idx = flat_index(i_cell, i_species, n_species, state._vector_size)
            cpp_state_set_concentration!(state._ptr, Int64(idx), values[i_cell])
        end
    end
end

"""
    get_concentrations(state::State) -> Dict{String, Vector{Float64}}

Get species concentrations for all grid cells.
"""
function get_concentrations(state::State)
    concentrations = Dict{String, Vector{Float64}}()
    n_species = length(state._species_ordering)
    for (species, i_species) in state._species_ordering
        vals = Vector{Float64}(undef, state._number_of_grid_cells)
        for i_cell in 1:state._number_of_grid_cells
            idx = flat_index(i_cell, i_species, n_species, state._vector_size)
            vals[i_cell] = cpp_state_get_concentration(state._ptr, Int64(idx))
        end
        concentrations[species] = vals
    end
    return concentrations
end

"""
    set_user_defined_rate_parameters!(state::State, params::Dict{String})

Set user-defined rate parameters. Values can be scalars (for single grid cell)
or vectors matching the number of grid cells.
"""
function set_user_defined_rate_parameters!(state::State, params::Dict{String, <:Any})
    n_params = length(state._rate_parameter_ordering)
    for (name, value) in params
        haskey(state._rate_parameter_ordering, name) || error(
            "User-defined rate parameter $name not found in the mechanism.")
        i_param = state._rate_parameter_ordering[name]
        values = value isa Real ? [Float64(value)] : Float64.(value)
        length(values) == state._number_of_grid_cells || error(
            "User-defined rate parameter list for $name must have length $(state._number_of_grid_cells).")
        for i_cell in 1:state._number_of_grid_cells
            idx = flat_index(i_cell, i_param, n_params, state._vector_size)
            cpp_state_set_rate_param!(state._ptr, Int64(idx), values[i_cell])
        end
    end
end

"""
    get_user_defined_rate_parameters(state::State) -> Dict{String, Vector{Float64}}

Get user-defined rate parameters for all grid cells.
"""
function get_user_defined_rate_parameters(state::State)
    result = Dict{String, Vector{Float64}}()
    n_params = length(state._rate_parameter_ordering)
    for (param, i_param) in state._rate_parameter_ordering
        vals = Vector{Float64}(undef, state._number_of_grid_cells)
        for i_cell in 1:state._number_of_grid_cells
            idx = flat_index(i_cell, i_param, n_params, state._vector_size)
            vals[i_cell] = cpp_state_get_rate_param(state._ptr, Int64(idx))
        end
        result[param] = vals
    end
    return result
end

"""
    set_conditions!(state::State; temperatures, pressures, air_densities)

Set environmental conditions for each grid cell. For a single grid cell,
scalar values are accepted. If `air_densities` is not provided, it is
calculated from the Ideal Gas Law.
"""
function set_conditions!(state::State;
    temperatures::Union{Real, AbstractVector{<:Real}, Nothing}=nothing,
    pressures::Union{Real, AbstractVector{<:Real}, Nothing}=nothing,
    air_densities::Union{Real, AbstractVector{<:Real}, Nothing}=nothing,
)
    n = state._number_of_grid_cells

    function expand(param, name)
        if param === nothing
            return nothing
        elseif param isa Real
            n == 1 || error("$name must be a vector of length $n for multi-cell states.")
            return [Float64(param)]
        else
            length(param) == n || error("$name must have length $n.")
            return Float64.(param)
        end
    end

    temps = expand(temperatures, "temperatures")
    press = expand(pressures, "pressures")
    dens = expand(air_densities, "air_densities")

    for i_cell in 1:n
        idx = Int64(i_cell - 1)  # 0-based for C++
        cur_temp = temps !== nothing ? temps[i_cell] : cpp_state_get_condition_temperature(state._ptr, idx)
        cur_pres = press !== nothing ? press[i_cell] : cpp_state_get_condition_pressure(state._ptr, idx)
        if dens !== nothing
            cur_dens = dens[i_cell]
        elseif cur_temp > 0.0 && cur_pres > 0.0
            cur_dens = cur_pres / (GAS_CONSTANT * cur_temp)
        else
            cur_dens = 0.0
        end
        cpp_state_set_condition!(state._ptr, idx, cur_temp, cur_pres, cur_dens)
    end
end

"""
    get_conditions(state::State) -> Dict{String, Vector{Float64}}

Get environmental conditions for all grid cells.
Returns a dictionary with keys "temperature", "pressure", "air_density".
"""
function get_conditions(state::State)
    temps = Vector{Float64}(undef, state._number_of_grid_cells)
    press = Vector{Float64}(undef, state._number_of_grid_cells)
    dens = Vector{Float64}(undef, state._number_of_grid_cells)
    for i_cell in 1:state._number_of_grid_cells
        idx = Int64(i_cell - 1)
        temps[i_cell] = cpp_state_get_condition_temperature(state._ptr, idx)
        press[i_cell] = cpp_state_get_condition_pressure(state._ptr, idx)
        dens[i_cell] = cpp_state_get_condition_air_density(state._ptr, idx)
    end
    return Dict{String, Vector{Float64}}(
        "temperature" => temps,
        "pressure" => press,
        "air_density" => dens,
    )
end

"""
    get_species_ordering(state::State) -> Dict{String, Int}

Get the mapping of species names to their indices.
"""
get_species_ordering(state::State) = state._species_ordering

"""
    get_user_defined_rate_parameters_ordering(state::State) -> Dict{String, Int}

Get the mapping of user-defined rate parameter names to their indices.
"""
get_user_defined_rate_parameters_ordering(state::State) = state._rate_parameter_ordering
