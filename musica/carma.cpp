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
      [](py::dict params_dict)
      {
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
        if (params_dict.contains("idx_wave"))
          params.idx_wave = params_dict["idx_wave"].cast<int>();
        if (params_dict.contains("dtime"))
          params.dtime = params_dict["dtime"].cast<double>();
        if (params_dict.contains("nstep"))
          params.nstep = static_cast<int>(params_dict["nstep"].cast<float>());
        if (params_dict.contains("deltaz"))
          params.deltaz = params_dict["deltaz"].cast<double>();
        if (params_dict.contains("zmin"))
          params.zmin = params_dict["zmin"].cast<double>();

        // Handle groups configuration
        if (params_dict.contains("groups"))
        {
          auto groups_py = params_dict["groups"];
          if (!groups_py.is_none() && py::isinstance<py::list>(groups_py))
          {
            auto groups_list = groups_py.cast<py::list>();
            for (auto group_py : groups_list)
            {
              auto group_dict = group_py.cast<py::dict>();
              musica::CARMAGroupConfig group;

              if (group_dict.contains("id"))
                group.id = group_dict["id"].cast<int>();
              if (group_dict.contains("name"))
                group.name = group_dict["name"].cast<std::string>();
              if (group_dict.contains("shortname"))
                group.shortname = group_dict["shortname"].cast<std::string>();
              if (group_dict.contains("rmin"))
                group.rmin = group_dict["rmin"].cast<double>();
              if (group_dict.contains("rmrat"))
                group.rmrat = group_dict["rmrat"].cast<double>();
              if (group_dict.contains("ishape"))
                group.ishape = static_cast<musica::ParticleShape>(group_dict["ishape"].cast<int>());
              if (group_dict.contains("eshape"))
                group.eshape = group_dict["eshape"].cast<double>();
              if (group_dict.contains("is_ice"))
                group.is_ice = group_dict["is_ice"].cast<bool>();
              if (group_dict.contains("is_fractal"))
                group.is_fractal = group_dict["is_fractal"].cast<bool>();
              if (group_dict.contains("do_mie"))
                group.do_mie = group_dict["do_mie"].cast<bool>();
              if (group_dict.contains("do_wetdep"))
                group.do_wetdep = group_dict["do_wetdep"].cast<bool>();
              if (group_dict.contains("do_drydep"))
                group.do_drydep = group_dict["do_drydep"].cast<bool>();
              if (group_dict.contains("do_vtran"))
                group.do_vtran = group_dict["do_vtran"].cast<bool>();
              if (group_dict.contains("solfac"))
                group.solfac = group_dict["solfac"].cast<double>();
              if (group_dict.contains("scavcoef"))
                group.scavcoef = group_dict["scavcoef"].cast<double>();
              if (group_dict.contains("rmon"))
                group.rmon = group_dict["rmon"].cast<double>();
              if (group_dict.contains("df"))
                group.df = group_dict["df"].cast<std::vector<double>>();
              if (group_dict.contains("falpha"))
                group.falpha = group_dict["falpha"].cast<double>();

              params.groups.push_back(group);
            }
          }
        }

        // Handle elements configuration
        if (params_dict.contains("elements"))
        {
          auto elements_py = params_dict["elements"];
          if (!elements_py.is_none() && py::isinstance<py::list>(elements_py))
          {
            auto elements_list = elements_py.cast<py::list>();
            for (auto element_py : elements_list)
            {
              auto element_dict = element_py.cast<py::dict>();
              musica::CARMAElementConfig element;

              if (element_dict.contains("id"))
                element.id = element_dict["id"].cast<int>();
              if (element_dict.contains("igroup"))
                element.igroup = element_dict["igroup"].cast<int>();
              if (element_dict.contains("name"))
                element.name = element_dict["name"].cast<std::string>();
              if (element_dict.contains("shortname"))
                element.shortname = element_dict["shortname"].cast<std::string>();
              if (element_dict.contains("rho"))
                element.rho = element_dict["rho"].cast<double>();
              if (element_dict.contains("itype"))
                element.itype = static_cast<musica::ParticleType>(element_dict["itype"].cast<int>());
              if (element_dict.contains("icomposition"))
                element.icomposition = static_cast<musica::ParticleComposition>(element_dict["icomposition"].cast<int>());
              if (element_dict.contains("isolute"))
                element.isolute = element_dict["isolute"].cast<int>();
              if (element_dict.contains("rhobin"))
                element.rhobin = element_dict["rhobin"].cast<std::vector<double>>();
              if (element_dict.contains("arat"))
                element.arat = element_dict["arat"].cast<std::vector<double>>();
              if (element_dict.contains("kappa"))
                element.kappa = element_dict["kappa"].cast<double>();
              if (element_dict.contains("is_shell"))
                element.isShell = element_dict["is_shell"].cast<bool>();

              params.elements.push_back(element);
            }
          }
        }

        if (params_dict.contains("extinction_coefficient"))
        {
          auto extinction_coeff_py = params_dict["extinction_coefficient"];
          if (!extinction_coeff_py.is_none())
          {
            // Convert 3D Python list to flat array
            auto extinction_3d = extinction_coeff_py.cast<std::vector<std::vector<std::vector<double>>>>();
            size_t total_size = params.nwave * params.nbin * params.ngroup;
            params.extinction_coefficient.resize(total_size);

            // Copy data using proper indexing: index = i + j*nwave + k*nwave*nbin
            for (int k = 0; k < params.ngroup; ++k)
            {
              for (int j = 0; j < params.nbin; ++j)
              {
                for (int i = 0; i < params.nwave; ++i)
                {
                  size_t idx = i + j * params.nwave + k * params.nwave * params.nbin;
                  params.extinction_coefficient[idx] = extinction_3d[i][j][k];
                }
              }
            }
          }
        }

        try
        {
          auto carma_instance = new musica::CARMA(params);
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
      "_run_carma",
      [](std::uintptr_t carma_ptr)
      {
        musica::CARMA* carma_instance = reinterpret_cast<musica::CARMA*>(carma_ptr);

        try
        {
          musica::CARMAOutput output = carma_instance->Run();

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
}