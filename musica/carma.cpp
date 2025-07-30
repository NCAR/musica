#include "binding_common.hpp"

#include <musica/carma/carma.hpp>
#include <musica/carma/carma_state.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <iostream>

namespace py = pybind11;

auto to_vector_double = [](const py::object& obj) -> std::vector<double>
{
  if (obj.is_none())
    return {};
  if (py::isinstance<py::array>(obj))
  {
    auto arr = obj.cast<py::array>();
    if (arr.size() == 0)
      return {};
    // Accept both 0d, 1d arrays
    if (arr.ndim() == 0)
      return { arr.cast<double>() };
    if (arr.ndim() == 1)
      return py::cast<std::vector<double>>(obj);
    throw std::invalid_argument("Expected 1D array for 1D double vector");
  }
  if (py::isinstance<py::sequence>(obj))
  {
    auto seq = obj.cast<py::sequence>();
    if (seq.size() == 0)
      return {};
    return py::cast<std::vector<double>>(obj);
  }
  return {};
};

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
        if (params_dict.contains("nbin"))
          params.nbin = params_dict["nbin"].cast<int>();
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
                group.fall_velocity_routine =
                    static_cast<musica::FallVelocityAlgorithm>(group_dict["fall_velocity_routine"].cast<int>());
              if (group_dict.contains("mie_calculation_algorithm"))
                group.mie_calculation_algorithm =
                    static_cast<musica::MieCalculationAlgorithm>(group_dict["mie_calculation_algorithm"].cast<int>());
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

        // Handle coagulation configuration
        if (params_dict.contains("coagulations"))
        {
          auto coagulations_py = params_dict["coagulations"];
          if (!coagulations_py.is_none() && py::isinstance<py::list>(coagulations_py))
          {
            auto coagulations_list = coagulations_py.cast<py::list>();
            for (auto coagulation_py : coagulations_list)
            {
              auto coagulation_dict = coagulation_py.cast<py::dict>();
              musica::CARMACoagulationConfig coagulation;

              if (coagulation_dict.contains("igroup1"))
                coagulation.igroup1 = coagulation_dict["igroup1"].cast<int>();
              if (coagulation_dict.contains("igroup2"))
                coagulation.igroup2 = coagulation_dict["igroup2"].cast<int>();
              if (coagulation_dict.contains("igroup3"))
                coagulation.igroup3 = coagulation_dict["igroup3"].cast<int>();
              if (coagulation_dict.contains("algorithm"))
                coagulation.algorithm =
                    static_cast<musica::ParticleCollectionAlgorithm>(coagulation_dict["algorithm"].cast<int>());
              if (coagulation_dict.contains("ck0"))
                coagulation.ck0 = coagulation_dict["ck0"].cast<double>();
              if (coagulation_dict.contains("grav_e_coll0"))
                coagulation.grav_e_coll0 = coagulation_dict["grav_e_coll0"].cast<double>();
              if (coagulation_dict.contains("use_ccd"))
                coagulation.use_ccd = coagulation_dict["use_ccd"].cast<bool>();

              params.coagulations.push_back(coagulation);
            }
          }
        }

        // Handle growth configuration
        if (params_dict.contains("growths"))
        {
          auto growths_py = params_dict["growths"];
          if (!growths_py.is_none() && py::isinstance<py::list>(growths_py))
          {
            auto growths_list = growths_py.cast<py::list>();
            for (auto growth_py : growths_list)
            {
              auto growth_dict = growth_py.cast<py::dict>();
              musica::CARMAGrowthConfig growth;

              if (growth_dict.contains("ielem"))
                growth.ielem = growth_dict["ielem"].cast<int>();
              if (growth_dict.contains("igas"))
                growth.igas = growth_dict["igas"].cast<int>();

              params.growths.push_back(growth);
            }
          }
        }

        // Handle nucleation configuration
        if (params_dict.contains("nucleations"))
        {
          auto nucleations_py = params_dict["nucleations"];
          if (!nucleations_py.is_none() && py::isinstance<py::list>(nucleations_py))
          {
            auto nucleations_list = nucleations_py.cast<py::list>();
            for (auto nucleation_py : nucleations_list)
            {
              auto nucleation_dict = nucleation_py.cast<py::dict>();
              musica::CARMANucleationConfig nucleation;

              if (nucleation_dict.contains("ielemfrom"))
                nucleation.ielemfrom = nucleation_dict["ielemfrom"].cast<int>();
              if (nucleation_dict.contains("ielemto"))
                nucleation.ielemto = nucleation_dict["ielemto"].cast<int>();
              if (nucleation_dict.contains("algorithm"))
                nucleation.algorithm =
                    static_cast<musica::ParticleNucleationAlgorithm>(nucleation_dict["algorithm"].cast<int>());
              if (nucleation_dict.contains("rlh_nuc"))
                nucleation.rlh_nuc = nucleation_dict["rlh_nuc"].cast<double>();
              if (nucleation_dict.contains("igas"))
                nucleation.igas = nucleation_dict["igas"].cast<int>();
              if (nucleation_dict.contains("ievp2elem"))
                nucleation.ievp2elem = nucleation_dict["ievp2elem"].cast<int>();

              params.nucleations.push_back(nucleation);
            }
          }
        }

        // Handle initialization configuration
        if (params_dict.contains("initialization"))
        {
          auto initialization_py = params_dict["initialization"];
          if (!initialization_py.is_none() && py::isinstance<py::dict>(initialization_py))
          {
            auto initialization_dict = initialization_py.cast<py::dict>();

            if (initialization_dict.contains("do_cnst_rlh"))
              params.initialization.do_cnst_rlh = initialization_dict["do_cnst_rlh"].cast<bool>();
            if (initialization_dict.contains("do_detrain"))
              params.initialization.do_detrain = initialization_dict["do_detrain"].cast<bool>();
            if (initialization_dict.contains("do_fixedinit"))
              params.initialization.do_fixedinit = initialization_dict["do_fixedinit"].cast<bool>();
            if (initialization_dict.contains("do_incloud"))
              params.initialization.do_incloud = initialization_dict["do_incloud"].cast<bool>();
            if (initialization_dict.contains("do_explised"))
              params.initialization.do_explised = initialization_dict["do_explised"].cast<bool>();
            if (initialization_dict.contains("do_substep"))
              params.initialization.do_substep = initialization_dict["do_substep"].cast<bool>();
            if (initialization_dict.contains("do_thermo"))
              params.initialization.do_thermo = initialization_dict["do_thermo"].cast<bool>();
            if (initialization_dict.contains("do_vdiff"))
              params.initialization.do_vdiff = initialization_dict["do_vdiff"].cast<bool>();
            if (initialization_dict.contains("do_vtran"))
              params.initialization.do_vtran = initialization_dict["do_vtran"].cast<bool>();
            if (initialization_dict.contains("do_drydep"))
              params.initialization.do_drydep = initialization_dict["do_drydep"].cast<bool>();
            if (initialization_dict.contains("do_pheat"))
              params.initialization.do_pheat = initialization_dict["do_pheat"].cast<bool>();
            if (initialization_dict.contains("do_pheatatm"))
              params.initialization.do_pheatatm = initialization_dict["do_pheatatm"].cast<bool>();
            if (initialization_dict.contains("do_clearsky"))
              params.initialization.do_clearsky = initialization_dict["do_clearsky"].cast<bool>();
            if (initialization_dict.contains("do_partialinit"))
              params.initialization.do_partialinit = initialization_dict["do_partialinit"].cast<bool>();
            if (initialization_dict.contains("do_coremasscheck"))
              params.initialization.do_coremasscheck = initialization_dict["do_coremasscheck"].cast<bool>();
            if (initialization_dict.contains("vf_const"))
              params.initialization.vf_const = initialization_dict["vf_const"].cast<double>();
            if (initialization_dict.contains("minsubsteps"))
              params.initialization.minsubsteps = initialization_dict["minsubsteps"].cast<int>();
            if (initialization_dict.contains("maxsubsteps"))
              params.initialization.maxsubsteps = initialization_dict["maxsubsteps"].cast<int>();
            if (initialization_dict.contains("maxretries"))
              params.initialization.maxretries = initialization_dict["maxretries"].cast<int>();
            if (initialization_dict.contains("conmax"))
              params.initialization.conmax = initialization_dict["conmax"].cast<double>();
            if (initialization_dict.contains("dt_threshold"))
              params.initialization.dt_threshold = initialization_dict["dt_threshold"].cast<double>();
            if (initialization_dict.contains("cstick"))
              params.initialization.cstick = initialization_dict["cstick"].cast<double>();
            if (initialization_dict.contains("gsticki"))
              params.initialization.gsticki = initialization_dict["gsticki"].cast<double>();
            if (initialization_dict.contains("gstickl"))
              params.initialization.gstickl = initialization_dict["gstickl"].cast<double>();
            if (initialization_dict.contains("tstick"))
              params.initialization.tstick = initialization_dict["tstick"].cast<double>();
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

  carma.def(
      "_create_carma_state",
      [](std::uintptr_t carma_ptr, py::kwargs kwargs)
      {
        // Helper lambdas for robust flexible casting
        musica::CARMAStateParameters params;
        params.longitude = kwargs.contains("longitude") ? kwargs["longitude"].cast<double>() : 0.0;
        params.latitude = kwargs.contains("latitude") ? kwargs["latitude"].cast<double>() : 0.0;
        params.coordinates = kwargs.contains("coordinates")
                                 ? static_cast<musica::CarmaCoordinates>(kwargs["coordinates"].cast<int>())
                                 : musica::CarmaCoordinates::CARTESIAN;
        params.vertical_center = to_vector_double(kwargs["vertical_center"]);
        params.vertical_levels = to_vector_double(kwargs["vertical_levels"]);
        params.temperature = to_vector_double(kwargs["temperature"]);
        params.pressure = to_vector_double(kwargs["pressure"]);
        params.pressure_levels = to_vector_double(kwargs["pressure_levels"]);

        musica::CARMA* carma_instance = reinterpret_cast<musica::CARMA*>(carma_ptr);
        try
        {
          auto carma_state = new musica::CARMAState(carma_instance, params);
          return reinterpret_cast<std::uintptr_t>(carma_state);
        }
        catch (const std::exception& e)
        {
          throw py::value_error("Error creating CARMA instance: " + std::string(e.what()));
        }
      },
      py::arg("carma_pointer"),
      "Create a CARMA state for a specific column with named arguments");

  carma.def(
      "_delete_carma_state",
      [](std::uintptr_t carma_state_ptr)
      {
        auto carma_state = reinterpret_cast<musica::CARMAState*>(carma_state_ptr);
        delete carma_state;
      },
      "Delete a CARMA state instance");

  carma.def(
      "_set_bin",
      [](std::uintptr_t carma_state_ptr, int bin_index, int element_index, py::object value)
      {
        auto carma_state = reinterpret_cast<musica::CARMAState*>(carma_state_ptr);
        carma_state->SetBin(bin_index, element_index, to_vector_double(value));
      },
      "Set values for a specific bin and element in the CARMA state");
  
  carma.def(
      "_set_detrain",
      [](std::uintptr_t carma_state_ptr, int bin_index, int element_index, py::object value)
      {
        auto carma_state = reinterpret_cast<musica::CARMAState*>(carma_state_ptr);
        carma_state->SetBin(bin_index, element_index, to_vector_double(value));
      },
      "Set the mass of the detrained condensate for the bin");
  
  carma.def(
      "_set_gas",
      [](std::uintptr_t carma_state_ptr, int gas_index, py::object value, py::object old_mmr, py::object gas_saturation_wrt_ice, py::object gas_saturation_wrt_liquid)
      {
        auto carma_state = reinterpret_cast<musica::CARMAState*>(carma_state_ptr);
        carma_state->SetGas(gas_index, to_vector_double(value), to_vector_double(old_mmr), to_vector_double(gas_saturation_wrt_ice), to_vector_double(gas_saturation_wrt_liquid));
      },
      "Set the gas mass mixing ratio for a specific gas index in the CARMA state");
  
  carma.def(
      "_get_step_statistics",
      [](std::uintptr_t carma_state_ptr)
      {
        auto carma_state = reinterpret_cast<musica::CARMAState*>(carma_state_ptr);
        musica::CarmaStatistics stats = carma_state->GetStepStatistics();
        py::dict result;
        result["max_number_of_substeps"] = stats.max_number_of_substeps;
        result["max_number_of_retries"] = stats.max_number_of_retries;
        result["total_number_of_steps"] = stats.total_number_of_steps;
        result["total_number_of_substeps"] = stats.total_number_of_substeps;
        result["total_number_of_retries"] = stats.total_number_of_retries;
        result["z_substeps"] = stats.z_substeps;
        result["xc"] = stats.xc;
        result["yc"] = stats.yc;
        return result;
      },
      "Get the step statistics for the current CARMAState");
}