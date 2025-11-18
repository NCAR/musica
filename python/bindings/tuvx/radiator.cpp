// Copyright (C) 2023-2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file defines the Python bindings for the TUV-x Radiator class in the musica library.
#include "../common.hpp"

#include <musica/tuvx/grid.hpp>
#include <musica/tuvx/radiator.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void bind_tuvx_radiator(py::module_ &radiator)
{
  py::class_<musica::Radiator>(radiator, "_Radiator")
      .def(py::init(
          [](const py::kwargs &kwargs)
          {
            if (!kwargs.contains("name"))
              throw py::value_error("Missing required argument: name");
            if (!kwargs.contains("height_grid"))
              throw py::value_error("Missing required argument: height_grid");
            if (!kwargs.contains("wavelength_grid"))
              throw py::value_error("Missing required argument: wavelength_grid");
            if (!py::isinstance<py::str>(kwargs["name"]))
              throw py::value_error("Argument 'name' must be a string");
            if (!py::isinstance<musica::Grid>(kwargs["height_grid"].cast<py::object>()))
              throw py::value_error("Argument 'height_grid' must be a Grid object");
            if (!py::isinstance<musica::Grid>(kwargs["wavelength_grid"].cast<py::object>()))
              throw py::value_error("Argument 'wavelength_grid' must be a Grid object");

            std::string name = kwargs["name"].cast<std::string>();
            musica::Grid *height_grid = kwargs["height_grid"].cast<musica::Grid *>();
            musica::Grid *wavelength_grid = kwargs["wavelength_grid"].cast<musica::Grid *>();

            musica::Error error;
            auto radiator_instance = new musica::Radiator(name.c_str(), height_grid, wavelength_grid, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error creating radiator: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            return radiator_instance;
          }))
      .def("__del__", [](musica::Radiator &radiator) {})
      .def_property_readonly(
          "name",
          [](musica::Radiator &self)
          {
            musica::Error error;
            std::string name = self.GetName(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting radiator name: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            musica::DeleteError(&error);
            return name;
          },
          "The name of the radiator")
      .def_property_readonly(
          "number_of_height_sections",
          [](musica::Radiator &self)
          {
            musica::Error error;
            size_t num_height_sections = self.GetNumberOfHeightSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting number of height grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            musica::DeleteError(&error);
            return num_height_sections;
          },
          "The number of sections in the height grid")
      .def_property_readonly(
          "number_of_wavelength_sections",
          [](musica::Radiator &self)
          {
            musica::Error error;
            size_t num_wavelength_sections = self.GetNumberOfWavelengthSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message =
                  "Error getting number of wavelength grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            musica::DeleteError(&error);
            return num_wavelength_sections;
          },
          "The number of sections in the wavelength grid")
      .def_property(
          "optical_depths",
          // Getter - converts C++ array to 2D numpy array
          [](musica::Radiator &self)
          {
            musica::Error error;
            size_t num_height_sections = self.GetNumberOfHeightSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting number of height grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            size_t num_wavelength_sections = self.GetNumberOfWavelengthSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message =
                  "Error getting number of wavelength grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            auto result = py::array_t<double>({ num_wavelength_sections, num_height_sections });
            py::buffer_info buf = result.request();
            double *ptr = static_cast<double *>(buf.ptr);
            self.GetOpticalDepths(ptr, num_height_sections, num_wavelength_sections, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting optical depths: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            return result;
          },
          // Setter - converts 2D numpy array to C++ array
          [](musica::Radiator &self, py::array_t<double, py::array::c_style | py::array::forcecast> array)
          {
            py::buffer_info buf = array.request();
            musica::Error error;
            size_t num_height_sections = self.GetNumberOfHeightSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting number of height grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            size_t num_wavelength_sections = self.GetNumberOfWavelengthSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message =
                  "Error getting number of wavelength grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            if (buf.ndim != 2)
              throw py::value_error("Array must be two-dimensional");
            if (static_cast<size_t>(buf.shape[0]) != num_wavelength_sections ||
                static_cast<size_t>(buf.shape[1]) != num_height_sections)
              throw py::value_error("Array shape must be (num_wavelength_sections, num_height_sections)");
            double *ptr = static_cast<double *>(buf.ptr);
            self.SetOpticalDepths(ptr, num_height_sections, num_wavelength_sections, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error setting optical depths: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
          },
          "2D array of optical depths with shape (num_wavelength_sections, num_height_sections)")
      .def_property(
          "single_scattering_albedos",
          // Getter - converts C++ array to 2D numpy array
          [](musica::Radiator &self)
          {
            musica::Error error;
            size_t num_height_sections = self.GetNumberOfHeightSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting number of height grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            size_t num_wavelength_sections = self.GetNumberOfWavelengthSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message =
                  "Error getting number of wavelength grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            auto result = py::array_t<double>({ num_wavelength_sections, num_height_sections });
            py::buffer_info buf = result.request();
            double *ptr = static_cast<double *>(buf.ptr);
            self.GetSingleScatteringAlbedos(ptr, num_height_sections, num_wavelength_sections, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting single scattering albedos: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            return result;
          },
          // Setter - converts 2D numpy array to C++ array
          [](musica::Radiator &self, py::array_t<double, py::array::c_style | py::array::forcecast> array)
          {
            py::buffer_info buf = array.request();
            musica::Error error;
            size_t num_height_sections = self.GetNumberOfHeightSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting number of height grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            size_t num_wavelength_sections = self.GetNumberOfWavelengthSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message =
                  "Error getting number of wavelength grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            if (buf.ndim != 2)
              throw py::value_error("Array must be two-dimensional");
            if (static_cast<size_t>(buf.shape[0]) != num_wavelength_sections ||
                static_cast<size_t>(buf.shape[1]) != num_height_sections)
              throw py::value_error("Array shape must be (num_wavelength_sections, num_height_sections)");
            double *ptr = static_cast<double *>(buf.ptr);
            self.SetSingleScatteringAlbedos(ptr, num_height_sections, num_wavelength_sections, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error setting single scattering albedos: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
          },
          "2D array of single scattering albedos with shape (num_wavelength_sections, num_height_sections)")
      .def_property(
          "asymmetry_factors",
          // Getter - converts C++ array to 2D numpy array
          [](musica::Radiator &self)
          {
            musica::Error error;
            size_t num_height_sections = self.GetNumberOfHeightSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting number of height grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            size_t num_wavelength_sections = self.GetNumberOfWavelengthSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message =
                  "Error getting number of wavelength grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            auto result = py::array_t<double>({ num_wavelength_sections, num_height_sections });
            py::buffer_info buf = result.request();
            double *ptr = static_cast<double *>(buf.ptr);
            // The number of streams is currently fixed at 1 in TUV-x
            constexpr size_t num_streams = 1;
            self.GetAsymmetryFactors(ptr, num_height_sections, num_wavelength_sections, num_streams, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting asymmetry factors: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            return result;
          },
          // Setter - converts 2D numpy array to C++ array
          [](musica::Radiator &self, py::array_t<double, py::array::c_style | py::array::forcecast> array)
          {
            py::buffer_info buf = array.request();
            musica::Error error;
            size_t num_height_sections = self.GetNumberOfHeightSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error getting number of height grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            size_t num_wavelength_sections = self.GetNumberOfWavelengthSections(&error);
            if (!musica::IsSuccess(error))
            {
              std::string message =
                  "Error getting number of wavelength grid sections: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
            if (buf.ndim != 2)
              throw py::value_error("Array must be two-dimensional");
            if (static_cast<size_t>(buf.shape[0]) != num_wavelength_sections ||
                static_cast<size_t>(buf.shape[1]) != num_height_sections)
              throw py::value_error("Array shape must be (num_wavelength_sections, num_height_sections)");
            double *ptr = static_cast<double *>(buf.ptr);
            // The number of streams is currently fixed at 1 in TUV-x
            constexpr size_t num_streams = 1;
            self.SetAsymmetryFactors(ptr, num_height_sections, num_wavelength_sections, num_streams, &error);
            if (!musica::IsSuccess(error))
            {
              std::string message = "Error setting asymmetry factors: " + std::string(error.message_.value_);
              musica::DeleteError(&error);
              throw py::value_error(message);
            }
          },
          "2D array of asymmetry factors with shape (num_wavelength_sections, num_height_sections)");
}
