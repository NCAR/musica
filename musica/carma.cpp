#include "binding_common.hpp"

#include <musica/carma/carma.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <format>
#include <iostream>

namespace py = pybind11;

void print_2d_vector(const std::vector<std::vector<double>>& vec)
{
  for (const auto& row : vec)
  {
    for (const auto& value : row)
    {
      std::cout << value << " ";
    }
  }
  std::cout << std::endl;
}

void print_3d_vector(const std::vector<std::vector<std::vector<double>>>& vec)
{
  for (const auto& matrix : vec)
  {
    for (const auto& row : matrix)
    {
      for (const auto& value : row)
      {
        std::cout << value << " ";
      }
      std::cout << std::endl;
    }
    std::cout << "-----" << std::endl;  // Separator for matrices
  }
}

void bind_carma(py::module_& carma)
{
  carma.def("_get_carma_version", []() { return musica::CARMA::GetVersion(); }, "Get the version of the CARMA instance");

  carma.def(
      "_create_carma",
      []()
      {
        try
        {
          auto carma_instance = new musica::CARMA();
          return reinterpret_cast<std::uintptr_t>(carma_instance);
        }
        catch (const std::exception& e)
        {
          throw py::value_error("Error creating CARMA instance: " + std::string(e.what()));
        }
      },
      "Create a CARMA instance");

  carma.def(
      "_delete_carma",
      [](std::uintptr_t carma_ptr)
      {
        musica::CARMA* carma_instance = reinterpret_cast<musica::CARMA*>(carma_ptr);
        delete carma_instance;
      },
      "Delete a CARMA instance");

  carma.def(
      "_run_carma_with_parameters",
      [](std::uintptr_t carma_ptr, py::dict params_dict)
      {
        musica::CARMA* carma_instance = reinterpret_cast<musica::CARMA*>(carma_ptr);

        // Convert Python dict to CARMAParameters
        musica::CARMAParameters params;

        if (params_dict.contains("max_bins"))
          params.max_bins = params_dict["max_bins"].cast<int>();
        if (params_dict.contains("max_groups"))
          params.max_groups = params_dict["max_groups"].cast<int>();
        if (params_dict.contains("nz"))
          params.nz = params_dict["nz"].cast<int>();
        if (params_dict.contains("ny"))
          params.ny = params_dict["ny"].cast<int>();
        if (params_dict.contains("nx"))
          params.nx = params_dict["nx"].cast<int>();
        if (params_dict.contains("nelem"))
          params.nelem = params_dict["nelem"].cast<int>();
        if (params_dict.contains("ngroup"))
          params.ngroup = params_dict["ngroup"].cast<int>();
        if (params_dict.contains("nbin"))
          params.nbin = params_dict["nbin"].cast<int>();
        if (params_dict.contains("nsolute"))
          params.nsolute = params_dict["nsolute"].cast<int>();
        if (params_dict.contains("ngas"))
          params.ngas = params_dict["ngas"].cast<int>();
        if (params_dict.contains("nwave"))
          params.nwave = params_dict["nwave"].cast<int>();
        if (params_dict.contains("dtime"))
          params.dtime = params_dict["dtime"].cast<double>();
        if (params_dict.contains("nstep"))
          params.nstep = params_dict["nstep"].cast<int>();
        if (params_dict.contains("deltaz"))
          params.deltaz = params_dict["deltaz"].cast<double>();
        if (params_dict.contains("zmin"))
          params.zmin = params_dict["zmin"].cast<double>();

        try
        {
          musica::CARMAOutput output = carma_instance->Run(params);

          std::cout << std::format(
                           "nz={}, ny={}, nx={}, nelem={}, ngroup={}, nbin={}, ngas={}, nstep={}",
                           output.nz,
                           output.ny,
                           output.nx,
                           output.nelem,
                           output.ngroup,
                           output.nbin,
                           output.ngas,
                           output.nstep)
                    << std::endl;

          std::cout << "lat: ";
          for (const auto& value : output.lat)
          {
            std::cout << value << " ";
          }
          std::cout << std::endl;

          std::cout << "lon: ";
          for (const auto& value : output.lon)
          {
            std::cout << value << " ";
          }
          std::cout << std::endl;

          std::cout << "Vertical center: ";
          for (const auto& value : output.vertical_center)
          {
            std::cout << value << " ";
          }
          std::cout << std::endl;

          std::cout << "Vertical levels: ";
          for (const auto& value : output.vertical_levels)
          {
            std::cout << value << " ";
          }
          std::cout << std::endl;

          std::cout << "Pressure: ";
          for (const auto& value : output.pressure)
          {
            std::cout << value << " ";
          }
          std::cout << std::endl;

          std::cout << "Temperature: ";
          for (const auto& value : output.temperature)
          {
            std::cout << value << " ";
          }
          std::cout << std::endl;

          std::cout << "Air density: ";
          for (const auto& value : output.air_density)
          {
            std::cout << value << " ";
          }
          std::cout << std::endl;

          std::cout << "Radiative heating: ";
          for (const auto& value : output.radiative_heating)
          {
            std::cout << value << " ";
          }
          std::cout << std::endl;

          std::cout << "Delta temperature: ";
          for (const auto& value : output.delta_temperature)
          {
            std::cout << value << " ";
          }
          std::cout << std::endl;

          std::cout << "Gas MMR: ";
          print_2d_vector(output.gas_mmr);
          std::cout << "Gas saturation liquid: ";
          print_2d_vector(output.gas_saturation_liquid);
          std::cout << "Gas saturation ice: ";
          print_2d_vector(output.gas_saturation_ice);
          std::cout << "Gas vapor pressure ice: ";
          print_2d_vector(output.gas_vapor_pressure_ice);
          std::cout << "Gas vapor pressure liquid: ";
          print_2d_vector(output.gas_vapor_pressure_liquid);
          std::cout << "Gas weight percent: ";
          print_2d_vector(output.gas_weight_percent);
          std::cout << "Number density: ";
          print_2d_vector(output.number_density);
          std::cout << "Surface area: ";
          print_2d_vector(output.surface_area);
          std::cout << "Mass density: ";
          print_2d_vector(output.mass_density);
          std::cout << "Effective radius: ";
          print_2d_vector(output.effective_radius);
          std::cout << "Effective radius wet: ";
          print_2d_vector(output.effective_radius_wet);
          std::cout << "Mean radius: ";
          print_2d_vector(output.mean_radius);
          std::cout << "Nucleation rate: ";
          print_2d_vector(output.nucleation_rate);
          std::cout << "Mass mixing ratio: ";
          print_2d_vector(output.mass_mixing_ratio);
          std::cout << "Projected area: ";
          print_2d_vector(output.projected_area);
          std::cout << "Aspect ratio: ";
          print_2d_vector(output.aspect_ratio);
          std::cout << "Vertical mass flux: ";
          print_2d_vector(output.vertical_mass_flux);
          std::cout << "Extinction: ";
          print_2d_vector(output.extinction);
          std::cout << "Optical depth: ";
          print_2d_vector(output.optical_depth);
          std::cout << "Bin wet radius: ";
          print_3d_vector(output.bin_wet_radius);
          std::cout << "Bin number density: ";
          print_3d_vector(output.bin_number_density);
          std::cout << "Bin density: ";
          print_3d_vector(output.bin_density);
          std::cout << "Bin mass mixing ratio: ";
          print_3d_vector(output.bin_mass_mixing_ratio);
          std::cout << "Bin deposition velocity: ";
          print_3d_vector(output.bin_deposition_velocity);
          std::cout << "Group radius: ";
          print_2d_vector(output.group_radius);
          std::cout << "Group mass: ";
          print_2d_vector(output.group_mass);
          std::cout << "Group volume: ";
          print_2d_vector(output.group_volume);
          std::cout << "Group radius ratio: ";
          print_2d_vector(output.group_radius_ratio);
          std::cout << "Group aspect ratio: ";
          print_2d_vector(output.group_aspect_ratio);
          std::cout << "Group fractal dimension: ";
          print_2d_vector(output.group_fractal_dimension);
        }
        catch (const std::exception& e)
        {
          throw py::value_error("Error running CARMA: " + std::string(e.what()));
        }
      },
      "Run CARMA with specified parameters");

  // Expose test configuration factory functions
  carma.def(
      "_get_aluminum_test_params",
      []()
      {
        auto params = musica::CARMATestConfigs::CreateAluminumTestParams();
        py::dict result;
        result["max_bins"] = params.max_bins;
        result["max_groups"] = params.max_groups;
        result["nz"] = params.nz;
        result["ny"] = params.ny;
        result["nx"] = params.nx;
        result["nelem"] = params.nelem;
        result["ngroup"] = params.ngroup;
        result["nbin"] = params.nbin;
        result["nsolute"] = params.nsolute;
        result["ngas"] = params.ngas;
        result["nwave"] = params.nwave;
        result["dtime"] = params.dtime;
        result["nstep"] = params.nstep;
        result["deltaz"] = params.deltaz;
        result["zmin"] = params.zmin;
        return result;
      },
      "Get aluminum test parameters");

  carma.def(
      "_get_fractal_optics_test_params",
      []()
      {
        auto params = musica::CARMATestConfigs::CreateFractalOpticsTestParams();
        py::dict result;
        result["max_bins"] = params.max_bins;
        result["max_groups"] = params.max_groups;
        result["nz"] = params.nz;
        result["ny"] = params.ny;
        result["nx"] = params.nx;
        result["nelem"] = params.nelem;
        result["ngroup"] = params.ngroup;
        result["nbin"] = params.nbin;
        result["nsolute"] = params.nsolute;
        result["ngas"] = params.ngas;
        result["nwave"] = params.nwave;
        result["dtime"] = params.dtime;
        result["nstep"] = params.nstep;
        result["deltaz"] = params.deltaz;
        result["zmin"] = params.zmin;
        return result;
      },
      "Get fractal optics test parameters");

  carma.def(
      "_get_sulfate_test_params",
      []()
      {
        auto params = musica::CARMATestConfigs::CreateSulfateTestParams();
        py::dict result;
        result["max_bins"] = params.max_bins;
        result["max_groups"] = params.max_groups;
        result["nz"] = params.nz;
        result["ny"] = params.ny;
        result["nx"] = params.nx;
        result["nelem"] = params.nelem;
        result["ngroup"] = params.ngroup;
        result["nbin"] = params.nbin;
        result["nsolute"] = params.nsolute;
        result["ngas"] = params.ngas;
        result["nwave"] = params.nwave;
        result["dtime"] = params.dtime;
        result["nstep"] = params.nstep;
        result["deltaz"] = params.deltaz;
        result["zmin"] = params.zmin;
        return result;
      },
      "Get sulfate test parameters");
}