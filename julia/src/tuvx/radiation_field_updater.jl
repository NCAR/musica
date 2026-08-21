# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

const RadiationFieldUpdaterPtr = CxxWrap.CxxWrapCore.CxxPtr{CppRadiationFieldUpdater}

"""
    RadiationFieldUpdater

Lets a host application push a radiation field into TUV-x's "from host" solver.

Obtained only via [`get_radiation_field_updater`](@ref); never constructed directly. The
returned updater is valid only while the `TUVX` instance that produced it stays alive.
"""
mutable struct RadiationFieldUpdater
    _ptr::RadiationFieldUpdaterPtr
    _num_vertical_interfaces::Int
    _num_wavelength_bins::Int

    function RadiationFieldUpdater(
        ptr::RadiationFieldUpdaterPtr,
        num_vertical_interfaces::Integer,
        num_wavelength_bins::Integer,
    )
        obj = new(ptr, Int(num_vertical_interfaces), Int(num_wavelength_bins))
        finalizer(obj) do u
            cpp_delete_radiation_field_updater(u._ptr)
        end
        return obj
    end
end

function _check_radiation_field_array_size(
    updater::RadiationFieldUpdater,
    values::AbstractMatrix{<:Real},
    label::AbstractString,
)
    expected = (updater._num_vertical_interfaces, updater._num_wavelength_bins)
    size(values) == expected || error(
        "$label must have size $expected (num_vertical_interfaces, num_wavelength_bins).",
    )
end

"""
    update!(updater::RadiationFieldUpdater,
            direct_actinic_flux::AbstractMatrix{<:Real},
            upward_actinic_flux::AbstractMatrix{<:Real},
            downward_actinic_flux::AbstractMatrix{<:Real};
            direct_irradiance = nothing,
            upward_irradiance = nothing,
            downward_irradiance = nothing) -> RadiationFieldUpdater

Set the radiation field TUV-x will use for the next [`run!`](@ref).

Each array has shape `(num_vertical_interfaces, num_wavelength_bins)`. With Julia's
column-major layout, this is the same convention `Radiator`'s zero-copy views already
use, and it needs no transpose to reach the layout the Fortran bridge expects. Interface
1 is the lowest altitude. All values are dimensionless and must NOT include the
extraterrestrial flux profile or the Earth-Sun distance factor -- TUV-x applies both
downstream.

- `direct_actinic_flux`, `upward_actinic_flux`, `downward_actinic_flux`: components of
  the actinic flux (required)
- `direct_irradiance`, `upward_irradiance`, `downward_irradiance`: components of the
  irradiance (optional, independently). Used only for dose rates; an omitted component
  is treated as all zeros for this call.

Call this before every [`run!`](@ref). TUV-x does not raise an error if a run is missed,
it silently reuses the last field set (or an all-zero field, if `update!` was never
called).
"""
function update!(
    updater::RadiationFieldUpdater,
    direct_actinic_flux::AbstractMatrix{<:Real},
    upward_actinic_flux::AbstractMatrix{<:Real},
    downward_actinic_flux::AbstractMatrix{<:Real};
    direct_irradiance::Union{AbstractMatrix{<:Real},Nothing} = nothing,
    upward_irradiance::Union{AbstractMatrix{<:Real},Nothing} = nothing,
    downward_irradiance::Union{AbstractMatrix{<:Real},Nothing} = nothing,
)
    _check_radiation_field_array_size(updater, direct_actinic_flux, "direct_actinic_flux")
    _check_radiation_field_array_size(updater, upward_actinic_flux, "upward_actinic_flux")
    _check_radiation_field_array_size(
        updater,
        downward_actinic_flux,
        "downward_actinic_flux",
    )
    direct_irradiance !== nothing &&
        _check_radiation_field_array_size(updater, direct_irradiance, "direct_irradiance")
    upward_irradiance !== nothing &&
        _check_radiation_field_array_size(updater, upward_irradiance, "upward_irradiance")
    downward_irradiance !== nothing && _check_radiation_field_array_size(
        updater,
        downward_irradiance,
        "downward_irradiance",
    )

    placeholder = zeros(Float64, 0)
    cpp_radiation_field_updater_update!(
        updater._ptr,
        vec(convert(Matrix{Float64}, direct_actinic_flux)),
        vec(convert(Matrix{Float64}, upward_actinic_flux)),
        vec(convert(Matrix{Float64}, downward_actinic_flux)),
        direct_irradiance !== nothing ? vec(convert(Matrix{Float64}, direct_irradiance)) :
        placeholder,
        upward_irradiance !== nothing ? vec(convert(Matrix{Float64}, upward_irradiance)) :
        placeholder,
        downward_irradiance !== nothing ?
        vec(convert(Matrix{Float64}, downward_irradiance)) : placeholder,
        direct_irradiance !== nothing,
        upward_irradiance !== nothing,
        downward_irradiance !== nothing,
        Int64(updater._num_vertical_interfaces),
        Int64(updater._num_wavelength_bins),
    )
    return updater
end

export RadiationFieldUpdater, update!
