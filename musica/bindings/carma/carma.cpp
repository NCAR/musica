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
  try
  {
    if (obj.is_none())
      return {};
    std::vector<double> result;
    for (const auto& item : obj)
    {
      result.push_back(item.cast<double>());
    }
    return result;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error converting sequence to vector<double>: " << e.what() << std::endl;
    std::cerr << "Object type: " << py::str(obj.get_type()).cast<std::string>() << std::endl;
    std::cerr << "Object repr: " << py::repr(obj).cast<std::string>() << std::endl;
    throw;
  }
};

auto array_2d_to_vector_double = [](const py::object& obj) -> std::tuple<std::vector<double>, int, int>
{
  try
  {
    if (obj.is_none())
      return {};
    if (py::isinstance<py::array>(obj))
    {
      auto arr = obj.cast<py::array>();
      if (arr.ndim() != 2)
        throw std::invalid_argument("Expected 2D array for 2D double vector");
      auto shape = arr.shape();
      return { arr.cast<std::vector<double>>(), static_cast<int>(shape[0]), static_cast<int>(shape[1]) };
    }
    if (py::isinstance<py::list>(obj))
    {
      auto list = obj.cast<py::list>();
      if (list.size() == 0 || !py::isinstance<py::list>(list[0]))
        throw std::invalid_argument("Expected list of lists for 2D double vector");
      int rows = static_cast<int>(list.size());
      int cols = static_cast<int>(list[0].cast<py::list>().size());
      std::vector<double> result;
      for (const auto& row : list)
      {
        auto row_list = row.cast<py::list>();
        if (row_list.size() != cols)
          throw std::invalid_argument("All rows must have the same number of elements");
        for (const auto& item : row_list)
          result.push_back(item.cast<double>());
      }
      return { result, rows, cols };
    }
    throw std::invalid_argument("Expected a 2D array or list of lists for 2D double vector");
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error converting sequence to vector<double>: " << e.what() << std::endl;
    std::cerr << "Object type: " << py::str(obj.get_type()).cast<std::string>() << std::endl;
    std::cerr << "Object repr: " << py::repr(obj).cast<std::string>() << std::endl;
    throw;
  }
};

auto to_surface_properties = [](const py::object& obj) -> musica::CARMASurfaceProperties
{
  if (obj.is_none())
    return {};
  if (py::isinstance<py::dict>(obj))
  {
    auto dict = obj.cast<py::dict>();
    musica::CARMASurfaceProperties props;
    if (dict.contains("surface_friction_velocity"))
      props.surface_friction_velocity = dict["surface_friction_velocity"].cast<double>();
    if (dict.contains("aerodynamic_resistance"))
      props.aerodynamic_resistance = dict["aerodynamic_resistance"].cast<double>();
    if (dict.contains("area_fraction"))
      props.area_fraction = dict["area_fraction"].cast<double>();
    return props;
  }
  if (py::isinstance<py::object>(obj))
  {
    musica::CARMASurfaceProperties props;
    props.surface_friction_velocity = obj.attr("surface_friction_velocity").cast<double>();
    props.aerodynamic_resistance = obj.attr("aerodynamic_resistance").cast<double>();
    props.area_fraction = obj.attr("area_fraction").cast<double>();
    return props;
  }
  throw std::invalid_argument("Expected a dictionary for surface properties");
};

void bind_carma(py::module_& carma)
{
  carma.def("_get_carma_version", []() { return musica::CARMA::GetVersion(); }, "Get the version of the CARMA instance");

  py::enum_<musica::ParticleType>(carma, "ParticleType")
      .value("INVOLATILE", musica::ParticleType::INVOLATILE)
      .value("VOLATILE", musica::ParticleType::VOLATILE)
      .value("COREMASS", musica::ParticleType::COREMASS)
      .value("VOLCORE", musica::ParticleType::VOLCORE)
      .value("CORE2MOM", musica::ParticleType::CORE2MOM)
      .export_values();

  py::enum_<musica::ParticleComposition>(carma, "ParticleComposition")
      .value("ALUMINUM", musica::ParticleComposition::ALUMINUM)
      .value("H2SO4", musica::ParticleComposition::H2SO4)
      .value("DUST", musica::ParticleComposition::DUST)
      .value("ICE", musica::ParticleComposition::ICE)
      .value("H2O", musica::ParticleComposition::H2O)
      .value("BLACKCARBON", musica::ParticleComposition::BLACKCARBON)
      .value("ORGANICCARBON", musica::ParticleComposition::ORGANICCARBON)
      .value("OTHER", musica::ParticleComposition::OTHER)
      .export_values();

  py::enum_<musica::SulfateNucleationMethod>(carma, "SulfateNucleationMethod")
      .value("NONE", musica::SulfateNucleationMethod::NONE)
      .value("ZHAO_TURCO", musica::SulfateNucleationMethod::ZHAO_TURCO)
      .value("VEHKAMAKI", musica::SulfateNucleationMethod::VEHKAMAKI)
      .export_values();

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
              {
                auto swell_py = group_dict["swelling_approach"];
                if (py::isinstance<py::dict>(swell_py))
                {
                  auto swell_dict = swell_py.cast<py::dict>();
                  if (swell_dict.contains("algorithm"))
                    group.swelling_approach.algorithm =
                        static_cast<musica::ParticleSwellingAlgorithm>(swell_dict["algorithm"].cast<int>());
                  if (swell_dict.contains("composition"))
                    group.swelling_approach.composition =
                        static_cast<musica::ParticleSwellingComposition>(swell_dict["composition"].cast<int>());
                }
                else
                {
                  throw py::type_error(
                      "Expected 'swelling_approach' to be a dictionary with 'algorithm' and 'composition' keys");
                }
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
              {
                group.df = to_vector_double(group_dict["df"]);
              }
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
                element.rhobin = to_vector_double(element_dict["rhobin"]);
              if (element_dict.contains("arat"))
                element.arat = to_vector_double(element_dict["arat"]);
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
            if (initialization_dict.contains("sulfnucl_method"))
              params.initialization.sulfnucl_method =
                  static_cast<musica::SulfateNucleationMethod>(initialization_dict["sulfnucl_method"].cast<int>());
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
      "_get_dimensions",
      [](std::uintptr_t carma_ptr)
      {
        musica::CARMA* carma_instance = reinterpret_cast<musica::CARMA*>(carma_ptr);
        auto params = carma_instance->GetParameters();
        py::dict result;
        result["number_of_bins"] = params.nbin;
        result["number_of_vertical_levels"] = params.nz;
        result["number_of_wavelength_bins"] = params.wavelength_bins.size();
        result["number_of_refractive_indices"] = params.number_of_refractive_indices;
        result["number_of_groups"] = params.groups.size();
        result["number_of_elements"] = params.elements.size();
        result["number_of_solutes"] = params.solutes.size();
        result["number_of_gases"] = params.gases.size();
        return result;
      },
      "Get the dimensions of the CARMA instance");

  carma.def(
      "_get_group_properties",
      [](std::uintptr_t carma_ptr, int group_index)
      {
        musica::CARMA* carma_instance = reinterpret_cast<musica::CARMA*>(carma_ptr);
        musica::CARMAGroupProperties group_props = carma_instance->GetGroupProperties(group_index);
        py::dict result;

        result["bin_radius"] = group_props.bin_radius;
        result["bin_radius_lower_bound"] = group_props.bin_radius_lower_bound;
        result["bin_radius_upper_bound"] = group_props.bin_radius_upper_bound;
        result["bin_width"] = group_props.bin_width;
        result["bin_mass"] = group_props.bin_mass;
        result["bin_width_mass"] = group_props.bin_width_mass;
        result["bin_volume"] = group_props.bin_volume;
        result["projected_area_ratio"] = group_props.projected_area_ratio;
        result["radius_ratio"] = group_props.radius_ratio;
        result["porosity_ratio"] = group_props.porosity_ratio;
        result["extinction_coefficient"] = group_props.extinction_coefficient;
        result["single_scattering_albedo"] = group_props.single_scattering_albedo;
        result["asymmetry_factor"] = group_props.asymmetry_factor;
        result["element_index_of_core_mass_elements"] = group_props.element_index_of_core_mass_elements;
        result["number_of_monomers_per_bin"] = group_props.number_of_monomers_per_bin;
        result["particle_number_element_for_group"] = group_props.particle_number_element_for_group;
        result["number_of_core_mass_elements_for_group"] = group_props.number_of_core_mass_elements_for_group;
        result["last_prognostic_bin"] = group_props.last_prognostic_bin;

        return result;
      },
      "Get properties of a specific CARMA group");

  carma.def(
      "_get_element_properties",
      [](std::uintptr_t carma_ptr, int element_index)
      {
        musica::CARMA* carma_instance = reinterpret_cast<musica::CARMA*>(carma_ptr);
        musica::CARMAElementProperties element_props = carma_instance->GetElementProperties(element_index);
        py::dict result;

        result["group_index"] = element_props.group_index;
        result["solute_index"] = element_props.solute_index;
        result["type"] = element_props.type;
        result["composition"] = element_props.composition;
        result["is_shell"] = element_props.is_shell;
        result["hygroscopicity_parameter"] = element_props.kappa;
        result["mass_density"] = element_props.rho;
        result["refractive_indices"] = element_props.refidx;
        result["number_of_refractive_indices"] = element_props.number_of_refractive_indices;

        return result;
      },
      "Get properties of a specific CARMA element");

  carma.def(
      "_create_carma_state",
      [](std::uintptr_t carma_ptr, py::kwargs kwargs)
      {
        // Helper lambdas for robust flexible casting
        musica::CARMAStateParameters params;
        params.time = kwargs.contains("time") ? kwargs["time"].cast<double>() : 0.0;
        params.time_step = kwargs.contains("time_step") ? kwargs["time_step"].cast<double>() : 0.0;
        params.longitude = kwargs.contains("longitude") ? kwargs["longitude"].cast<double>() : 0.0;
        params.latitude = kwargs.contains("latitude") ? kwargs["latitude"].cast<double>() : 0.0;
        params.coordinates = kwargs.contains("coordinates")
                                 ? static_cast<musica::CarmaCoordinates>(kwargs["coordinates"].cast<int>())
                                 : musica::CarmaCoordinates::CARTESIAN;
        params.vertical_center = to_vector_double(kwargs["vertical_center"]);
        params.vertical_levels = to_vector_double(kwargs["vertical_levels"]);
        params.temperature = to_vector_double(kwargs["temperature"]);
        params.original_temperature = to_vector_double(kwargs["original_temperature"]);
        params.pressure = to_vector_double(kwargs["pressure"]);
        params.pressure_levels = to_vector_double(kwargs["pressure_levels"]);
        if (kwargs.contains("relative_humidity") && !kwargs["relative_humidity"].is_none())
        {
          params.relative_humidity = to_vector_double(kwargs["relative_humidity"]);
        }
        if (kwargs.contains("specific_humidity") && !kwargs["specific_humidity"].is_none())
        {
          params.specific_humidity = to_vector_double(kwargs["specific_humidity"]);
        }
        if (kwargs.contains("radiative_intensity") && !kwargs["radiative_intensity"].is_none())
        {
          auto array_2d = array_2d_to_vector_double(kwargs["radiative_intensity"]);
          params.radiative_intensity = std::get<0>(array_2d);
          params.radiative_intensity_dim_1_size = std::get<1>(array_2d);
          params.radiative_intensity_dim_2_size = std::get<2>(array_2d);
        }

        musica::CARMA* carma_instance = reinterpret_cast<musica::CARMA*>(carma_ptr);
        try
        {
          auto carma_state = new musica::CARMAState(*carma_instance, params);
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
      [](std::uintptr_t carma_state_ptr, int bin_index, int element_index, py::object value, double surface_mass)
      {
        auto carma_state = reinterpret_cast<musica::CARMAState*>(carma_state_ptr);
        carma_state->SetBin(bin_index, element_index, to_vector_double(value), surface_mass);
      },
      "Set values for a specific bin and element in the CARMA state");

  carma.def(
      "_set_detrain",
      [](std::uintptr_t carma_state_ptr, int bin_index, int element_index, py::object value)
      {
        auto carma_state = reinterpret_cast<musica::CARMAState*>(carma_state_ptr);
        carma_state->SetDetrain(bin_index, element_index, to_vector_double(value));
      },
      "Set the mass of the detrained condensate for the bin");

  carma.def(
      "_set_gas",
      [](std::uintptr_t carma_state_ptr,
         int gas_index,
         py::object value,
         py::object old_mmr,
         py::object gas_saturation_wrt_ice,
         py::object gas_saturation_wrt_liquid)
      {
        auto carma_state = reinterpret_cast<musica::CARMAState*>(carma_state_ptr);
        carma_state->SetGas(
            gas_index,
            to_vector_double(value),
            to_vector_double(old_mmr),
            to_vector_double(gas_saturation_wrt_ice),
            to_vector_double(gas_saturation_wrt_liquid));
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
        // check if stats.z_substeps is all -1s, if so, set the result to None
        if (std::all_of(stats.z_substeps.begin(), stats.z_substeps.end(), [](int val) { return val == -1; }))
        {
          result["z_substeps"] = py::none();
        }
        else
        {
          result["z_substeps"] = stats.z_substeps;
        }
        result["xc"] = stats.xc;
        result["yc"] = stats.yc;
        return result;
      },
      "Get the step statistics for the current CARMAState");

  carma.def(
      "_get_bin",
      [](std::uintptr_t carma_state_ptr, int bin_index, int element_index)
      {
        auto carma_state = reinterpret_cast<musica::CARMAState*>(carma_state_ptr);
        musica::CarmaBinValues values = carma_state->GetBinValues(bin_index, element_index);
        py::dict result;
        result["mass_mixing_ratio"] = values.mass_mixing_ratio;
        result["number_mixing_ratio"] = values.number_mixing_ratio;
        result["number_density"] = values.number_density;
        result["nucleation_rate"] = values.nucleation_rate;
        result["wet_particle_radius"] = values.wet_particle_radius;
        result["wet_particle_density"] = values.wet_particle_density;
        result["dry_particle_density"] = values.dry_particle_density;
        result["particle_mass_on_surface"] = values.particle_mass_on_surface;
        result["sedimentation_flux"] = values.sedimentation_flux;
        result["fall_velocity"] = values.fall_velocity;
        result["deposition_velocity"] = values.deposition_velocity;
        result["delta_particle_temperature"] = values.delta_particle_temperature;
        result["kappa"] = values.kappa;
        result["total_mass_mixing_ratio"] = values.total_mass_mixing_ratio;
        return result;
      },
      "Get the values for a specific bin and element in the CARMA state");

  carma.def(
      "_get_detrain",
      [](std::uintptr_t carma_state_ptr, int bin_index, int element_index)
      {
        auto carma_state = reinterpret_cast<musica::CARMAState*>(carma_state_ptr);
        musica::CarmaDetrainValues values = carma_state->GetDetrain(bin_index, element_index);
        py::dict result;
        result["mass_mixing_ratio"] = values.mass_mixing_ratio;
        result["number_mixing_ratio"] = values.number_mixing_ratio;
        result["number_density"] = values.number_density;
        result["wet_particle_radius"] = values.wet_particle_radius;
        result["wet_particle_density"] = values.wet_particle_density;
        return result;
      },
      "Get the detrained condensate values for a specific bin and element in the CARMA state");

  carma.def(
      "_get_gas",
      [](std::uintptr_t carma_state_ptr, int gas_index)
      {
        auto carma_state = reinterpret_cast<musica::CARMAState*>(carma_state_ptr);
        musica::CarmaGasValues values = carma_state->GetGas(gas_index);
        py::dict result;
        result["mass_mixing_ratio"] = values.mass_mixing_ratio;
        result["gas_saturation_wrt_ice"] = values.gas_saturation_wrt_ice;
        result["gas_saturation_wrt_liquid"] = values.gas_saturation_wrt_liquid;
        result["gas_vapor_pressure_wrt_ice"] = values.gas_vapor_pressure_wrt_ice;
        result["gas_vapor_pressure_wrt_liquid"] = values.gas_vapor_pressure_wrt_liquid;
        result["weight_pct_aerosol_composition"] = values.weight_pct_aerosol_composition;
        return result;
      },
      "Get the gas values for a specific element in the CARMA state");

  carma.def(
      "_get_environmental_values",
      [](std::uintptr_t carma_state_ptr)
      {
        auto carma_state = reinterpret_cast<musica::CARMAState*>(carma_state_ptr);
        musica::CarmaEnvironmentalValues values = carma_state->GetEnvironmentalValues();
        py::dict result;
        result["temperature"] = values.temperature;
        result["pressure"] = values.pressure;
        result["air_density"] = values.air_density;
        if (std::all_of(values.latent_heat.begin(), values.latent_heat.end(), [](double val) { return val == -1; }))
        {
          result["latent_heat"] = py::none();
        }
        else
        {
          result["latent_heat"] = values.latent_heat;
        }
        return result;
      },
      "Get the state values for the current CARMAState");

  carma.def(
      "_set_temperature",
      [](std::uintptr_t carma_state_ptr, py::object temperature)
      {
        auto carma_state = reinterpret_cast<musica::CARMAState*>(carma_state_ptr);
        carma_state->SetTemperature(to_vector_double(temperature));
      },
      "Set the temperature profile [K] for the CARMA state");

  carma.def(
      "_set_air_density",
      [](std::uintptr_t carma_state_ptr, py::object air_density)
      {
        auto carma_state = reinterpret_cast<musica::CARMAState*>(carma_state_ptr);
        carma_state->SetAirDensity(to_vector_double(air_density));
      },
      "Set the air density profile [kg/m³] for the CARMA state");

  carma.def(
      "_step",
      [](std::uintptr_t carma_state_ptr, py::kwargs kwargs)
      {
        auto carma_state = reinterpret_cast<musica::CARMAState*>(carma_state_ptr);
        musica::CARMAStateStepConfig step_config;

        if (kwargs.contains("cloud_fraction"))
          step_config.cloud_fraction = to_vector_double(kwargs["cloud_fraction"]);
        if (kwargs.contains("critical_relative_humidity"))
          step_config.critical_relative_humidity = to_vector_double(kwargs["critical_relative_humidity"]);
        if (kwargs.contains("land"))
          step_config.land = to_surface_properties(kwargs["land"]);
        if (kwargs.contains("ocean"))
          step_config.ocean = to_surface_properties(kwargs["ocean"]);
        if (kwargs.contains("ice"))
          step_config.ice = to_surface_properties(kwargs["ice"]);

        try
        {
          carma_state->Step(step_config);
        }
        catch (const std::exception& e)
        {
          throw py::value_error("Error stepping CARMA state: " + std::string(e.what()));
        }
      },
      "Step the CARMA state with specified parameters");
}