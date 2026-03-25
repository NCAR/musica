# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

"""
    Conditions

Environmental conditions for a grid cell.

If `air_density` is not provided and both `temperature` and `pressure` are given,
air density is calculated from the Ideal Gas Law.

# Fields
- `temperature::Float64`: Temperature in Kelvin
- `pressure::Float64`: Pressure in Pascals
- `air_density::Float64`: Air density in mol m⁻³
"""
mutable struct Conditions
    temperature::Float64
    pressure::Float64
    air_density::Float64
end

function Conditions(; temperature::Real=0.0, pressure::Real=0.0, air_density::Union{Real, Nothing}=nothing)
    if air_density === nothing
        if temperature > 0.0 && pressure > 0.0
            air_density = pressure / (GAS_CONSTANT * temperature)
        else
            air_density = 0.0
        end
    end
    return Conditions(Float64(temperature), Float64(pressure), Float64(air_density))
end
