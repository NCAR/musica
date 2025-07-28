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

              if (group_dict.contains("name"))
                group.name = group_dict["name"].cast<std::string>();
              if (group_dict.contains("shortname"))
                group.shortname = group_dict["shortname"].cast<std::string>();
              if (group_dict.contains("rmin"))
                group.rmin = group_dict["rmin"].cast<double>();
              if (group_dict.contains("rmrat"))
                group.rmrat = group_dict["rmrat"].cast<double>();
              if (group_dict.contains("rmassmin"))
                group.rmassmin = group_dict["rmassmin"].cast<double>();
              if (group_dict.contains("ishape"))
                group.ishape = static_cast<musica::ParticleShape>(group_dict["ishape"].cast<int>());
              if (group_dict.contains("eshape"))
                group.eshape = group_dict["eshape"].cast<double>();
              if (group_dict.contains("swelling_approach"))
                if (group_dict["swelling_approach"].contains("algorithm") &&
                    group_dict["swelling_approach"].contains("composition"))
                {
                  group.swelling_approach.algorithm = static_cast<musica::ParticleSwellingAlgorithm>(
                      group_dict["swelling_approach"]["algorithm"].cast<int>());
                  group.swelling_approach.composition = static_cast<musica::ParticleSwellingComposition>(
                      group_dict["swelling_approach"]["composition"].cast<int>());
                }
              if (group_dict.contains("fall_velocity_routine"))
                group.fall_velocity_routine = static_cast<musica::FallVelocityAlgorithm>(
                    group_dict["fall_velocity_routine"].cast<int>());
              if (group_dict.contains("mie_calculation_algorithm"))
                group.mie_calculation_algorithm = static_cast<musica::MieCalculationAlgorithm>(
                    group_dict["mie_calculation_algorithm"].cast<int>());
              if (group_dict.contains("optics_algorithm"))
                group.optics_algorithm = static_cast<musica::OpticsAlgorithm>(group_dict["optics_algorithm"].cast<int>());
              if (group_dict.contains("is_ice"))
                group.is_ice = group_dict["is_ice"].cast<bool>();
              if (group_dict.contains("is_fractal"))
                group.is_fractal = group_dict["is_fractal"].cast<bool>();
              if (group_dict.contains("is_cloud"))
                group.is_cloud = group_dict["is_cloud"].cast<bool>();
              if (group_dict.contains("is_sulfate"))
                group.is_sulfate = group_dict["is_sulfate"].cast<bool>();
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
              if (group_dict.contains("dpc_threshold"))
                group.dpc_threshold = group_dict["dpc_threshold"].cast<double>();
              if (group_dict.contains("rmon"))
                group.rmon = group_dict["rmon"].cast<double>();
              if (group_dict.contains("df"))
                group.df = group_dict["df"].cast<std::vector<double>>();
              if (group_dict.contains("falpha"))
                group.falpha = group_dict["falpha"].cast<double>();
              if (group_dict.contains("neutral_volfrc"))
                group.neutral_volfrc = group_dict["neutral_volfrc"].cast<double>();

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

              if (element_dict.contains("igroup"))
                element.igroup = element_dict["igroup"].cast<int>();
              if (element_dict.contains("isolute"))
                element.isolute = element_dict["isolute"].cast<int>();
              if (element_dict.contains("name"))
                element.name = element_dict["name"].cast<std::string>();
              if (element_dict.contains("shortname"))
                element.shortname = element_dict["shortname"].cast<std::string>();
              if (element_dict.contains("itype"))
                element.itype = static_cast<musica::ParticleType>(element_dict["itype"].cast<int>());
              if (element_dict.contains("icomposition"))
                element.icomposition = static_cast<musica::ParticleComposition>(element_dict["icomposition"].cast<int>());
              if (element_dict.contains("is_shell"))
                element.isShell = element_dict["is_shell"].cast<bool>();
              if (element_dict.contains("rho"))
                element.rho = element_dict["rho"].cast<double>();
              if (element_dict.contains("rhobin"))
                element.rhobin = element_dict["rhobin"].cast<std::vector<double>>();
              if (element_dict.contains("arat"))
                element.arat = element_dict["arat"].cast<std::vector<double>>();
              if (element_dict.contains("kappa"))
                element.kappa = element_dict["kappa"].cast<double>();
              if (element_dict.contains("refidx"))
              {
                auto refidx_py = element_dict["refidx"];
                if (!refidx_py.is_none() && py::isinstance<py::list>(refidx_py))
                {
                  auto refidx_outer_list = refidx_py.cast<py::list>();
                  for (auto refidx_row_py : refidx_outer_list)
                  {
                    std::vector<musica::CARMAComplex> refidx_row;
                    if (!refidx_row_py.is_none() && py::isinstance<py::list>(refidx_row_py))
                    {
                      auto refidx_inner_list = refidx_row_py.cast<py::list>();
                      for (auto refidx_item : refidx_inner_list)
                      {
                        auto refidx_dict = refidx_item.cast<py::dict>();
                        musica::CARMAComplex refidx_value;
                        if (refidx_dict.contains("real"))
                          refidx_value.real = refidx_dict["real"].cast<double>();
                        if (refidx_dict.contains("imaginary"))
                          refidx_value.imaginary = refidx_dict["imaginary"].cast<double>();
                        refidx_row.push_back(refidx_value);
                      }
                    }
                    element.refidx.push_back(refidx_row);
                  }
                }
              }
              params.elements.push_back(element);
            }
          }
        }

        // Handle solutes configuration
        if (params_dict.contains("solutes"))
        {
          auto solutes_py = params_dict["solutes"];
          if (!solutes_py.is_none() && py::isinstance<py::list>(solutes_py))
          {
            auto solutes_list = solutes_py.cast<py::list>();
            for (auto solute_py : solutes_list)
            {
              auto solute_dict = solute_py.cast<py::dict>();
              musica::CARMASoluteConfig solute;

              if (solute_dict.contains("name"))
                solute.name = solute_dict["name"].cast<std::string>();
              if (solute_dict.contains("shortname"))
                solute.shortname = solute_dict["shortname"].cast<std::string>();
              if (solute_dict.contains("ions"))
                solute.ions = solute_dict["ions"].cast<int>();
              if (solute_dict.contains("wtmol"))
                solute.wtmol = solute_dict["wtmol"].cast<double>();
              if (solute_dict.contains("rho"))
                solute.rho = solute_dict["rho"].cast<double>();

              params.solutes.push_back(solute);
            }
          }
        }

        // Handle gases configuration
        if (params_dict.contains("gases"))
        {
          auto gases_py = params_dict["gases"];
          if (!gases_py.is_none() && py::isinstance<py::list>(gases_py))
          {
            auto gases_list = gases_py.cast<py::list>();
            for (auto gas_py : gases_list)
            {
              auto gas_dict = gas_py.cast<py::dict>();
              musica::CARMAGasConfig gas;

              if (gas_dict.contains("name"))
                gas.name = gas_dict["name"].cast<std::string>();
              if (gas_dict.contains("shortname"))
                gas.shortname = gas_dict["shortname"].cast<std::string>();
              if (gas_dict.contains("wtmol"))
                gas.wtmol = gas_dict["wtmol"].cast<double>();
              if (gas_dict.contains("ivaprtn"))
                gas.ivaprtn = static_cast<musica::VaporizationAlgorithm>(gas_dict["ivaprtn"].cast<int>());
              if (gas_dict.contains("icomposition"))
                gas.icomposition = static_cast<musica::GasComposition>(gas_dict["icomposition"].cast<int>());
              if (gas_dict.contains("dgc_threshold"))
                gas.dgc_threshold = gas_dict["dgc_threshold"].cast<double>();
              if (gas_dict.contains("ds_threshold"))
                gas.ds_threshold = gas_dict["ds_threshold"].cast<double>();
              if (gas_dict.contains("refidx"))
              {
                auto refidx_py = gas_dict["refidx"];
                if (!refidx_py.is_none() && py::isinstance<py::list>(refidx_py))
                {
                  auto refidx_outer_list = refidx_py.cast<py::list>();
                  for (auto refidx_row_py : refidx_outer_list)
                  {
                    std::vector<musica::CARMAComplex> refidx_row;
                    if (!refidx_row_py.is_none() && py::isinstance<py::list>(refidx_row_py))
                    {
                      auto refidx_inner_list = refidx_row_py.cast<py::list>();
                      for (auto refidx_item : refidx_inner_list)
                      {
                        auto refidx_dict = refidx_item.cast<py::dict>();
                        musica::CARMAComplex refidx_value;
                        if (refidx_dict.contains("real"))
                          refidx_value.real = refidx_dict["real"].cast<double>();
                        if (refidx_dict.contains("imaginary"))
                          refidx_value.imaginary = refidx_dict["imaginary"].cast<double>();
                        refidx_row.push_back(refidx_value);
                      }
                    }
                    gas.refidx.push_back(refidx_row);
                  }
                }
              }

              params.gases.push_back(gas);
            }
          }
        }

        // Handle wavelength bins
        if (params_dict.contains("wavelength_bins"))
        {
          auto wavelength_bins_py = params_dict["wavelength_bins"];
          if (!wavelength_bins_py.is_none() && py::isinstance<py::list>(wavelength_bins_py))
          {
            auto wavelength_bins_list = wavelength_bins_py.cast<py::list>();
            for (auto bin_py : wavelength_bins_list)
            {
              auto bin_dict = bin_py.cast<py::dict>();
              musica::CARMAWavelengthBin bin;

              if (bin_dict.contains("center"))
                bin.center = bin_dict["center"].cast<double>();
              if (bin_dict.contains("width"))
                bin.width = bin_dict["width"].cast<double>();
              if (bin_dict.contains("do_emission"))
                bin.do_emission = bin_dict["do_emission"].cast<bool>();

              params.wavelength_bins.push_back(bin);
            }
          }
        }
        if (params_dict.contains("number_of_refractive_indices"))
        {
          params.number_of_refractive_indices = params_dict["number_of_refractive_indices"].cast<int>();
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

          // Grid and coordinate arrays
          result["lat"] = output.lat;
          result["lon"] = output.lon;
          result["vertical_center"] = output.vertical_center;
          result["vertical_levels"] = output.vertical_levels;

          // Atmospheric state variables
          result["pressure"] = output.pressure;
          result["temperature"] = output.temperature;
          result["air_density"] = output.air_density;

          // Fundamental CARMA data for Python calculations
          // Particle state arrays (3D: nz x nbin x nelem)
          result["particle_concentration"] = output.particle_concentration;
          result["mass_mixing_ratio"] = output.mass_mixing_ratio;

          // Particle properties (3D: nz x nbin x ngroup)
          result["wet_radius"] = output.wet_radius;
          result["wet_density"] = output.wet_density;
          result["fall_velocity"] = output.fall_velocity;
          result["nucleation_rate"] = output.nucleation_rate;
          result["deposition_velocity"] = output.deposition_velocity;

          // Group configuration arrays (2D: nbin x ngroup)
          result["dry_radius"] = output.dry_radius;
          result["mass_per_bin"] = output.mass_per_bin;
          result["radius_ratio"] = output.radius_ratio;
          result["aspect_ratio"] = output.aspect_ratio;

          // Group mapping and properties (1D arrays)
          result["group_particle_number_concentration"] = output.group_particle_number_concentration;
          result["constituent_type"] = output.constituent_type;
          result["max_prognostic_bin"] = output.max_prognostic_bin;

          return result;
        }
        catch (const std::exception& e)
        {
          throw py::value_error("Error running CARMA: " + std::string(e.what()));
        }
      },
      "Run CARMA with specified parameters");
}