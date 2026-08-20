// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the Python bindings for the TUV-x RadiationFieldUpdater class in the
// musica library.
#include "../common.hpp"

#include <musica/tuvx/radiation_field_updater.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace
{
  using Array = py::array_t<double, py::array::c_style | py::array::forcecast>;

  // Validates a field array's shape and returns a pointer to its data.
  //
  // The array's numpy shape is (num_wavelength_bins, num_vertical_interfaces), matching the
  // axis order Radiator's Python properties already use (the other physical axis trailing).
  // With that shape and C-style (row-major) storage, the flat memory is interface-fastest,
  // which is exactly the layout the underlying Fortran bridge expects -- no transpose needed.
  double* GetValidatedPointer(
      const Array& array,
      std::size_t num_wavelength_bins,
      std::size_t num_vertical_interfaces,
      const char* name)
  {
    py::buffer_info buf = array.request();
    if (buf.ndim != 2)
      throw py::value_error(std::string(name) + " must be a two-dimensional array");
    if (static_cast<std::size_t>(buf.shape[0]) != num_wavelength_bins ||
        static_cast<std::size_t>(buf.shape[1]) != num_vertical_interfaces)
      throw py::value_error(std::string(name) + " must have shape (num_wavelength_bins, num_vertical_interfaces)");
    return static_cast<double*>(buf.ptr);
  }
}  // namespace

void bind_tuvx_radiation_field_updater(py::module_& radiation_field_updater)
{
  py::class_<musica::RadiationFieldUpdater>(radiation_field_updater, "_RadiationFieldUpdater")
      .def(
          "update",
          [](musica::RadiationFieldUpdater& self,
             Array direct_actinic_flux,
             Array upward_actinic_flux,
             Array downward_actinic_flux,
             std::size_t num_vertical_interfaces,
             std::size_t num_wavelength_bins,
             std::optional<Array> direct_irradiance,
             std::optional<Array> upward_irradiance,
             std::optional<Array> downward_irradiance)
          {
            double* direct_flux_ptr = GetValidatedPointer(
                direct_actinic_flux, num_wavelength_bins, num_vertical_interfaces, "direct_actinic_flux");
            double* upward_flux_ptr = GetValidatedPointer(
                upward_actinic_flux, num_wavelength_bins, num_vertical_interfaces, "upward_actinic_flux");
            double* downward_flux_ptr = GetValidatedPointer(
                downward_actinic_flux, num_wavelength_bins, num_vertical_interfaces, "downward_actinic_flux");
            double* direct_irradiance_ptr =
                direct_irradiance.has_value()
                    ? GetValidatedPointer(
                          *direct_irradiance, num_wavelength_bins, num_vertical_interfaces, "direct_irradiance")
                    : nullptr;
            double* upward_irradiance_ptr =
                upward_irradiance.has_value()
                    ? GetValidatedPointer(
                          *upward_irradiance, num_wavelength_bins, num_vertical_interfaces, "upward_irradiance")
                    : nullptr;
            double* downward_irradiance_ptr =
                downward_irradiance.has_value()
                    ? GetValidatedPointer(
                          *downward_irradiance, num_wavelength_bins, num_vertical_interfaces, "downward_irradiance")
                    : nullptr;

            musica::Error error;
            self.Update(
                direct_flux_ptr,
                upward_flux_ptr,
                downward_flux_ptr,
                direct_irradiance_ptr,
                upward_irradiance_ptr,
                downward_irradiance_ptr,
                num_vertical_interfaces,
                num_wavelength_bins,
                &error);
            handle_error(error, "Error updating radiation field");
          },
          py::arg("direct_actinic_flux"),
          py::arg("upward_actinic_flux"),
          py::arg("downward_actinic_flux"),
          py::arg("num_vertical_interfaces"),
          py::arg("num_wavelength_bins"),
          py::arg("direct_irradiance") = py::none(),
          py::arg("upward_irradiance") = py::none(),
          py::arg("downward_irradiance") = py::none(),
          "Set the radiation field TUV-x will use for the next run. Each array has shape "
          "(num_wavelength_bins, num_vertical_interfaces).");
}
