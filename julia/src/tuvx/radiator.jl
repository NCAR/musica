# Copyright (C) 2023-2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0

const RadiatorPtr = CxxWrap.CxxWrapCore.CxxPtr{CppRadiator}

"""
    RadiatorView <: AbstractMatrix{Float64}

A zero-copy view into a 2D value array of a [`Radiator`](@ref) (optical
depths, single scattering albedos, or asymmetry factors).

The array is stored height-fastest in memory, which is exactly Julia's
column-major layout for a `(num_height_sections, num_wavelength_sections)`
matrix, so no transpose is needed. Element `view[h, w]` is the value at
height section `h` and wavelength section `w`.

The view shares memory with the TUV-x radiator, so a write to the view
changes the radiator itself. The view also holds a reference to its radiator.
The reference stops the garbage collector from freeing the radiator while the
view is alive.

Get a new view after you add the radiator to a [`RadiatorMap`](@ref). The map
takes over the memory of the radiator, and an older view then points to
memory that is free.
"""
struct RadiatorView <: AbstractMatrix{Float64}
    _data::Matrix{Float64}
    _radiator::Any  # prevents GC of the parent Radiator while this view is alive
end

Base.size(view::RadiatorView) = size(view._data)
Base.IndexStyle(::Type{RadiatorView}) = IndexLinear()
Base.getindex(view::RadiatorView, i::Int) = view._data[i]
Base.getindex(view::RadiatorView, i::Int, j::Int) = view._data[i, j]
Base.setindex!(view::RadiatorView, value, i::Int) = (view._data[i] = value)
Base.setindex!(view::RadiatorView, value, i::Int, j::Int) = (view._data[i, j] = value)

"""
    Radiator

An optically active species for TUV-x radiative transfer calculations, such
as an aerosol layer.

# Constructor

    Radiator(; name, height_grid, wavelength_grid, optical_depths=nothing,
               single_scattering_albedos=nothing, asymmetry_factors=nothing)

- `name::AbstractString`: The name of the radiator
- `height_grid::Grid`: The height grid on which the radiator is defined
- `wavelength_grid::Grid`: The wavelength grid on which the radiator is defined
- `optical_depths::AbstractMatrix{<:Real}`: Optical depths, shape `(num_height_sections(height_grid), num_sections(wavelength_grid))`
- `single_scattering_albedos::AbstractMatrix{<:Real}`: Single scattering albedos, same shape as `optical_depths`
- `asymmetry_factors::AbstractMatrix{<:Real}`: Asymmetry factors, same shape as `optical_depths`

The number of streams is currently fixed at 1 in TUV-x, so asymmetry factors
are exposed as a 2D array.

# Example

```julia
height_grid = Grid(name = "height", units = "km", edges = [0.0, 2.0, 4.0])
wavelength_grid = Grid(name = "wavelength", units = "nm", edges = [200.0, 300.0])
radiator = Radiator(name = "aerosol", height_grid = height_grid, wavelength_grid = wavelength_grid)
```
"""
mutable struct Radiator
    _ptr::RadiatorPtr
    _owner::Any  # the RadiatorMap that owns the TUV-x radiator, or nothing

    function Radiator(ptr::RadiatorPtr, owner)
        obj = new(ptr, owner)
        finalizer(obj) do r
            cpp_delete_radiator(r._ptr)
        end
        return obj
    end
end

function Radiator(;
    name::AbstractString,
    height_grid::Grid,
    wavelength_grid::Grid,
    optical_depths::Union{AbstractMatrix{<:Real},Nothing} = nothing,
    single_scattering_albedos::Union{AbstractMatrix{<:Real},Nothing} = nothing,
    asymmetry_factors::Union{AbstractMatrix{<:Real},Nothing} = nothing,
)
    radiator = Radiator(
        cpp_create_radiator(String(name), height_grid._ptr, wavelength_grid._ptr),
        nothing,
    )

    optical_depths !== nothing && set_optical_depths!(radiator, optical_depths)
    single_scattering_albedos !== nothing &&
        set_single_scattering_albedos!(radiator, single_scattering_albedos)
    asymmetry_factors !== nothing && set_asymmetry_factors!(radiator, asymmetry_factors)

    return radiator
end

"""
    get_name(radiator::Radiator) -> String

Get the name of the radiator.
"""
get_name(radiator::Radiator) = String(cpp_radiator_name(radiator._ptr))

"""
    num_height_sections(radiator::Radiator) -> Int

Get the number of sections in the radiator's height grid.
"""
num_height_sections(radiator::Radiator) =
    Int(cpp_radiator_num_height_sections(radiator._ptr))

"""
    num_wavelength_sections(radiator::Radiator) -> Int

Get the number of sections in the radiator's wavelength grid.
"""
num_wavelength_sections(radiator::Radiator) =
    Int(cpp_radiator_num_wavelength_sections(radiator._ptr))

function _radiator_view(address::UInt64, radiator::Radiator)
    dims = (num_height_sections(radiator), num_wavelength_sections(radiator))
    data = unsafe_wrap(Array, Ptr{Float64}(address), dims; own = false)
    return RadiatorView(data, radiator)
end

"""
    optical_depths(radiator::Radiator) -> RadiatorView

Get a zero-copy view of the radiator's optical depths, of shape
`(num_height_sections, num_wavelength_sections)`.

A write to the view changes the radiator.
"""
optical_depths(radiator::Radiator) =
    _radiator_view(cpp_radiator_optical_depths_pointer(radiator._ptr), radiator)

"""
    single_scattering_albedos(radiator::Radiator) -> RadiatorView

Get a zero-copy view of the radiator's single scattering albedos, of shape
`(num_height_sections, num_wavelength_sections)`.

A write to the view changes the radiator.
"""
single_scattering_albedos(radiator::Radiator) =
    _radiator_view(cpp_radiator_single_scattering_albedos_pointer(radiator._ptr), radiator)

"""
    asymmetry_factors(radiator::Radiator) -> RadiatorView

Get a zero-copy view of the radiator's asymmetry factors, of shape
`(num_height_sections, num_wavelength_sections)`.

A write to the view changes the radiator.
"""
asymmetry_factors(radiator::Radiator) =
    _radiator_view(cpp_radiator_asymmetry_factors_pointer(radiator._ptr), radiator)

function _check_radiator_array_size(
    radiator::Radiator,
    values::AbstractMatrix{<:Real},
    label::AbstractString,
)
    expected = (num_height_sections(radiator), num_wavelength_sections(radiator))
    size(values) == expected || error(
        "$label must have size $expected (num_height_sections, num_wavelength_sections).",
    )
end

"""
    set_optical_depths!(radiator::Radiator, values::AbstractMatrix{<:Real}) -> Radiator

Copy `values` into the radiator's optical depths.
"""
function set_optical_depths!(radiator::Radiator, values::AbstractMatrix{<:Real})
    _check_radiator_array_size(radiator, values, "optical_depths")
    cpp_radiator_set_optical_depths!(radiator._ptr, vec(convert(Matrix{Float64}, values)))
    return radiator
end

"""
    set_single_scattering_albedos!(radiator::Radiator, values::AbstractMatrix{<:Real}) -> Radiator

Copy `values` into the radiator's single scattering albedos.
"""
function set_single_scattering_albedos!(radiator::Radiator, values::AbstractMatrix{<:Real})
    _check_radiator_array_size(radiator, values, "single_scattering_albedos")
    cpp_radiator_set_single_scattering_albedos!(
        radiator._ptr,
        vec(convert(Matrix{Float64}, values)),
    )
    return radiator
end

"""
    set_asymmetry_factors!(radiator::Radiator, values::AbstractMatrix{<:Real}) -> Radiator

Copy `values` into the radiator's asymmetry factors.
"""
function set_asymmetry_factors!(radiator::Radiator, values::AbstractMatrix{<:Real})
    _check_radiator_array_size(radiator, values, "asymmetry_factors")
    cpp_radiator_set_asymmetry_factors!(
        radiator._ptr,
        vec(convert(Matrix{Float64}, values)),
    )
    return radiator
end

function Base.show(io::IO, radiator::Radiator)
    print(
        io,
        "Radiator(name=\"$(get_name(radiator))\", ",
        "num_height_sections=$(num_height_sections(radiator)), ",
        "num_wavelength_sections=$(num_wavelength_sections(radiator)))",
    )
end

export Radiator, RadiatorView
export get_name, num_height_sections, num_wavelength_sections
export optical_depths, single_scattering_albedos, asymmetry_factors
export set_optical_depths!, set_single_scattering_albedos!, set_asymmetry_factors!
