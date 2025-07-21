#include "binding_common.hpp"

#include <musica/carma/carma.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <iostream>

namespace py = pybind11;

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

          // Convert CARMAOutput to Python dictionary
          py::dict result;

          // Dimensions
          result["nz"] = output.nz;
          result["ny"] = output.ny;
          result["nx"] = output.nx;
          result["nelem"] = output.nelem;
          result["ngroup"] = output.ngroup;
          result["nbin"] = output.nbin;
          result["ngas"] = output.ngas;
          result["nstep"] = output.nstep;

          // Grid and coordinate arrays
          result["lat"] = output.lat;
          result["lon"] = output.lon;
          result["vertical_center"] = output.vertical_center;
          result["vertical_levels"] = output.vertical_levels;

          // Atmospheric state variables
          result["pressure"] = output.pressure;
          result["temperature"] = output.temperature;
          result["air_density"] = output.air_density;
          result["radiative_heating"] = output.radiative_heating;
          result["delta_temperature"] = output.delta_temperature;

          // Gas variables
          result["gas_mmr"] = output.gas_mmr;
          result["gas_saturation_liquid"] = output.gas_saturation_liquid;
          result["gas_saturation_ice"] = output.gas_saturation_ice;
          result["gas_vapor_pressure_ice"] = output.gas_vapor_pressure_ice;
          result["gas_vapor_pressure_liquid"] = output.gas_vapor_pressure_liquid;
          result["gas_weight_percent"] = output.gas_weight_percent;

          // Group-integrated variables
          result["number_density"] = output.number_density;
          result["surface_area"] = output.surface_area;
          result["mass_density"] = output.mass_density;
          result["effective_radius"] = output.effective_radius;
          result["effective_radius_wet"] = output.effective_radius_wet;
          result["mean_radius"] = output.mean_radius;
          result["nucleation_rate"] = output.nucleation_rate;
          result["mass_mixing_ratio"] = output.mass_mixing_ratio;
          result["projected_area"] = output.projected_area;
          result["aspect_ratio"] = output.aspect_ratio;
          result["vertical_mass_flux"] = output.vertical_mass_flux;
          result["extinction"] = output.extinction;
          result["optical_depth"] = output.optical_depth;

          // Bin-resolved variables
          result["bin_wet_radius"] = output.bin_wet_radius;
          result["bin_number_density"] = output.bin_number_density;
          result["bin_density"] = output.bin_density;
          result["bin_mass_mixing_ratio"] = output.bin_mass_mixing_ratio;
          result["bin_deposition_velocity"] = output.bin_deposition_velocity;

          // Group properties
          result["group_radius"] = output.group_radius;
          result["group_mass"] = output.group_mass;
          result["group_volume"] = output.group_volume;
          result["group_radius_ratio"] = output.group_radius_ratio;
          result["group_aspect_ratio"] = output.group_aspect_ratio;
          result["group_fractal_dimension"] = output.group_fractal_dimension;

          // Names
          result["element_names"] = output.element_names;
          result["group_names"] = output.group_names;
          result["gas_names"] = output.gas_names;

          return result;
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