#include "../common.hpp"

#include <musica/carma/carma.hpp>
#include <musica/carma/carma_state.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>
#include <string>

namespace py = pybind11;

namespace pybind11::detail
{
  template<typename E>
  struct int_enum_caster
  {
    PYBIND11_TYPE_CASTER(E, const_name("int"));

    bool load(handle src, bool convert)
    {
      type_caster<int> caster;
      if (!caster.load(src, convert))
        return false;
      value = static_cast<E>(static_cast<int>(caster));
      return true;
    }

    static handle cast(E src, return_value_policy, handle)
    {
      return PyLong_FromLong(static_cast<long>(src));
    }
  };

  template<>
  struct type_caster<musica::ParticleShape> : int_enum_caster<musica::ParticleShape>
  {
  };
  template<>
  struct type_caster<musica::ParticleSwellingAlgorithm> : int_enum_caster<musica::ParticleSwellingAlgorithm>
  {
  };
  template<>
  struct type_caster<musica::ParticleSwellingComposition> : int_enum_caster<musica::ParticleSwellingComposition>
  {
  };
  template<>
  struct type_caster<musica::FallVelocityAlgorithm> : int_enum_caster<musica::FallVelocityAlgorithm>
  {
  };
  template<>
  struct type_caster<musica::MieCalculationAlgorithm> : int_enum_caster<musica::MieCalculationAlgorithm>
  {
  };
  template<>
  struct type_caster<musica::OpticsAlgorithm> : int_enum_caster<musica::OpticsAlgorithm>
  {
  };
  template<>
  struct type_caster<musica::VaporizationAlgorithm> : int_enum_caster<musica::VaporizationAlgorithm>
  {
  };
  template<>
  struct type_caster<musica::ParticleType> : int_enum_caster<musica::ParticleType>
  {
  };
  template<>
  struct type_caster<musica::GasComposition> : int_enum_caster<musica::GasComposition>
  {
  };
  template<>
  struct type_caster<musica::ParticleComposition> : int_enum_caster<musica::ParticleComposition>
  {
  };
  template<>
  struct type_caster<musica::ParticleCollectionAlgorithm> : int_enum_caster<musica::ParticleCollectionAlgorithm>
  {
  };
  template<>
  struct type_caster<musica::ParticleNucleationAlgorithm> : int_enum_caster<musica::ParticleNucleationAlgorithm>
  {
  };
  template<>
  struct type_caster<musica::SulfateNucleationMethod> : int_enum_caster<musica::SulfateNucleationMethod>
  {
  };
  template<>
  struct type_caster<musica::CarmaCoordinates> : int_enum_caster<musica::CarmaCoordinates>
  {
  };
}  // namespace pybind11::detail

void bind_carma(py::module_& carma)
{
  using namespace musica;

  carma.def("_get_carma_version", &CARMA::GetVersion, "Get the version of the CARMA instance");

  py::class_<CARMAComplex>(carma, "CARMAComplex")
      .def(py::init<>())
      .def(py::init<double, double>(), py::arg("real"), py::arg("imaginary"))
      .def_readwrite("real", &CARMAComplex::real)
      .def_readwrite("imaginary", &CARMAComplex::imaginary);

  py::class_<CARMAWavelengthBin>(carma, "CARMAWavelengthBin")
      .def(py::init<>())
      .def_readwrite("center", &CARMAWavelengthBin::center)
      .def_readwrite("width", &CARMAWavelengthBin::width)
      .def_readwrite("do_emission", &CARMAWavelengthBin::do_emission);

  py::class_<CARMASwellingApproach>(carma, "CARMASwellingApproach")
      .def(py::init<>())
      .def_readwrite("algorithm", &CARMASwellingApproach::algorithm)
      .def_readwrite("composition", &CARMASwellingApproach::composition);

  py::class_<CARMAGroupConfig>(carma, "CARMAGroupConfig")
      .def(py::init<>())
      .def_readwrite("name", &CARMAGroupConfig::name)
      .def_readwrite("shortname", &CARMAGroupConfig::shortname)
      .def_readwrite("rmin", &CARMAGroupConfig::rmin)
      .def_readwrite("rmrat", &CARMAGroupConfig::rmrat)
      .def_readwrite("rmassmin", &CARMAGroupConfig::rmassmin)
      .def_readwrite("ishape", &CARMAGroupConfig::ishape)
      .def_readwrite("eshape", &CARMAGroupConfig::eshape)
      .def_readwrite("swelling_approach", &CARMAGroupConfig::swelling_approach)
      .def_readwrite("fall_velocity_routine", &CARMAGroupConfig::fall_velocity_routine)
      .def_readwrite("mie_calculation_algorithm", &CARMAGroupConfig::mie_calculation_algorithm)
      .def_readwrite("optics_algorithm", &CARMAGroupConfig::optics_algorithm)
      .def_readwrite("is_ice", &CARMAGroupConfig::is_ice)
      .def_readwrite("is_fractal", &CARMAGroupConfig::is_fractal)
      .def_readwrite("is_cloud", &CARMAGroupConfig::is_cloud)
      .def_readwrite("is_sulfate", &CARMAGroupConfig::is_sulfate)
      .def_readwrite("do_wetdep", &CARMAGroupConfig::do_wetdep)
      .def_readwrite("do_drydep", &CARMAGroupConfig::do_drydep)
      .def_readwrite("do_vtran", &CARMAGroupConfig::do_vtran)
      .def_readwrite("solfac", &CARMAGroupConfig::solfac)
      .def_readwrite("scavcoef", &CARMAGroupConfig::scavcoef)
      .def_readwrite("dpc_threshold", &CARMAGroupConfig::dpc_threshold)
      .def_readwrite("rmon", &CARMAGroupConfig::rmon)
      .def_readwrite("df", &CARMAGroupConfig::df)
      .def_readwrite("falpha", &CARMAGroupConfig::falpha)
      .def_readwrite("neutral_volfrc", &CARMAGroupConfig::neutral_volfrc);

  py::class_<CARMAElementConfig>(carma, "CARMAElementConfig")
      .def(py::init<>())
      .def_readwrite("igroup", &CARMAElementConfig::igroup)
      .def_readwrite("isolute", &CARMAElementConfig::isolute)
      .def_readwrite("name", &CARMAElementConfig::name)
      .def_readwrite("shortname", &CARMAElementConfig::shortname)
      .def_readwrite("itype", &CARMAElementConfig::itype)
      .def_readwrite("icomposition", &CARMAElementConfig::icomposition)
      .def_readwrite("is_shell", &CARMAElementConfig::isShell)
      .def_readwrite("rho", &CARMAElementConfig::rho)
      .def_readwrite("rhobin", &CARMAElementConfig::rhobin)
      .def_readwrite("arat", &CARMAElementConfig::arat)
      .def_readwrite("kappa", &CARMAElementConfig::kappa)
      .def_readwrite("refidx", &CARMAElementConfig::refidx);

  py::class_<CARMASoluteConfig>(carma, "CARMASoluteConfig")
      .def(py::init<>())
      .def_readwrite("name", &CARMASoluteConfig::name)
      .def_readwrite("shortname", &CARMASoluteConfig::shortname)
      .def_readwrite("ions", &CARMASoluteConfig::ions)
      .def_readwrite("wtmol", &CARMASoluteConfig::wtmol)
      .def_readwrite("rho", &CARMASoluteConfig::rho);

  py::class_<CARMAGasConfig>(carma, "CARMAGasConfig")
      .def(py::init<>())
      .def_readwrite("name", &CARMAGasConfig::name)
      .def_readwrite("shortname", &CARMAGasConfig::shortname)
      .def_readwrite("wtmol", &CARMAGasConfig::wtmol)
      .def_readwrite("ivaprtn", &CARMAGasConfig::ivaprtn)
      .def_readwrite("icomposition", &CARMAGasConfig::icomposition)
      .def_readwrite("dgc_threshold", &CARMAGasConfig::dgc_threshold)
      .def_readwrite("ds_threshold", &CARMAGasConfig::ds_threshold)
      .def_readwrite("refidx", &CARMAGasConfig::refidx);

  py::class_<CARMACoagulationConfig>(carma, "CARMACoagulationConfig")
      .def(py::init<>())
      .def_readwrite("igroup1", &CARMACoagulationConfig::igroup1)
      .def_readwrite("igroup2", &CARMACoagulationConfig::igroup2)
      .def_readwrite("igroup3", &CARMACoagulationConfig::igroup3)
      .def_readwrite("algorithm", &CARMACoagulationConfig::algorithm)
      .def_readwrite("ck0", &CARMACoagulationConfig::ck0)
      .def_readwrite("grav_e_coll0", &CARMACoagulationConfig::grav_e_coll0)
      .def_readwrite("use_ccd", &CARMACoagulationConfig::use_ccd);

  py::class_<CARMAGrowthConfig>(carma, "CARMAGrowthConfig")
      .def(py::init<>())
      .def_readwrite("ielem", &CARMAGrowthConfig::ielem)
      .def_readwrite("igas", &CARMAGrowthConfig::igas);

  py::class_<CARMANucleationConfig>(carma, "CARMANucleationConfig")
      .def(py::init<>())
      .def_readwrite("ielemfrom", &CARMANucleationConfig::ielemfrom)
      .def_readwrite("ielemto", &CARMANucleationConfig::ielemto)
      .def_readwrite("algorithm", &CARMANucleationConfig::algorithm)
      .def_readwrite("rlh_nuc", &CARMANucleationConfig::rlh_nuc)
      .def_readwrite("igas", &CARMANucleationConfig::igas)
      .def_readwrite("ievp2elem", &CARMANucleationConfig::ievp2elem);

  py::class_<CARMAInitializationConfig>(carma, "CARMAInitializationConfig")
      .def(py::init<>())
      .def_readwrite("do_cnst_rlh", &CARMAInitializationConfig::do_cnst_rlh)
      .def_readwrite("do_detrain", &CARMAInitializationConfig::do_detrain)
      .def_readwrite("do_fixedinit", &CARMAInitializationConfig::do_fixedinit)
      .def_readwrite("do_incloud", &CARMAInitializationConfig::do_incloud)
      .def_readwrite("do_explised", &CARMAInitializationConfig::do_explised)
      .def_readwrite("do_substep", &CARMAInitializationConfig::do_substep)
      .def_readwrite("do_thermo", &CARMAInitializationConfig::do_thermo)
      .def_readwrite("do_vdiff", &CARMAInitializationConfig::do_vdiff)
      .def_readwrite("do_vtran", &CARMAInitializationConfig::do_vtran)
      .def_readwrite("do_drydep", &CARMAInitializationConfig::do_drydep)
      .def_readwrite("do_pheat", &CARMAInitializationConfig::do_pheat)
      .def_readwrite("do_pheatatm", &CARMAInitializationConfig::do_pheatatm)
      .def_readwrite("do_clearsky", &CARMAInitializationConfig::do_clearsky)
      .def_readwrite("do_partialinit", &CARMAInitializationConfig::do_partialinit)
      .def_readwrite("do_coremasscheck", &CARMAInitializationConfig::do_coremasscheck)
      .def_readwrite("sulfnucl_method", &CARMAInitializationConfig::sulfnucl_method)
      .def_readwrite("vf_const", &CARMAInitializationConfig::vf_const)
      .def_readwrite("minsubsteps", &CARMAInitializationConfig::minsubsteps)
      .def_readwrite("maxsubsteps", &CARMAInitializationConfig::maxsubsteps)
      .def_readwrite("maxretries", &CARMAInitializationConfig::maxretries)
      .def_readwrite("conmax", &CARMAInitializationConfig::conmax)
      .def_readwrite("dt_threshold", &CARMAInitializationConfig::dt_threshold)
      .def_readwrite("cstick", &CARMAInitializationConfig::cstick)
      .def_readwrite("gsticki", &CARMAInitializationConfig::gsticki)
      .def_readwrite("gstickl", &CARMAInitializationConfig::gstickl)
      .def_readwrite("tstick", &CARMAInitializationConfig::tstick);

  py::class_<CARMAParameters>(carma, "CARMAParameters")
      .def(py::init<>())
      .def_readwrite("nbin", &CARMAParameters::nbin)
      .def_readwrite("wavelength_bins", &CARMAParameters::wavelength_bins)
      .def_readwrite("number_of_refractive_indices", &CARMAParameters::number_of_refractive_indices)
      .def_readwrite("groups", &CARMAParameters::groups)
      .def_readwrite("elements", &CARMAParameters::elements)
      .def_readwrite("solutes", &CARMAParameters::solutes)
      .def_readwrite("gases", &CARMAParameters::gases)
      .def_readwrite("coagulations", &CARMAParameters::coagulations)
      .def_readwrite("growths", &CARMAParameters::growths)
      .def_readwrite("nucleations", &CARMAParameters::nucleations)
      .def_readwrite("initialization", &CARMAParameters::initialization);

  py::class_<CARMAGroupProperties>(carma, "CARMAGroupProperties")
      .def_readonly("bin_radius", &CARMAGroupProperties::bin_radius)
      .def_readonly("bin_radius_lower_bound", &CARMAGroupProperties::bin_radius_lower_bound)
      .def_readonly("bin_radius_upper_bound", &CARMAGroupProperties::bin_radius_upper_bound)
      .def_readonly("bin_width", &CARMAGroupProperties::bin_width)
      .def_readonly("bin_mass", &CARMAGroupProperties::bin_mass)
      .def_readonly("bin_width_mass", &CARMAGroupProperties::bin_width_mass)
      .def_readonly("bin_volume", &CARMAGroupProperties::bin_volume)
      .def_readonly("projected_area_ratio", &CARMAGroupProperties::projected_area_ratio)
      .def_readonly("radius_ratio", &CARMAGroupProperties::radius_ratio)
      .def_readonly("porosity_ratio", &CARMAGroupProperties::porosity_ratio)
      .def_readonly("extinction_coefficient", &CARMAGroupProperties::extinction_coefficient)
      .def_readonly("single_scattering_albedo", &CARMAGroupProperties::single_scattering_albedo)
      .def_readonly("asymmetry_factor", &CARMAGroupProperties::asymmetry_factor)
      .def_readonly("particle_number_element_for_group", &CARMAGroupProperties::particle_number_element_for_group)
      .def_readonly(
          "number_of_core_mass_elements_for_group", &CARMAGroupProperties::number_of_core_mass_elements_for_group)
      .def_readonly("element_index_of_core_mass_elements", &CARMAGroupProperties::element_index_of_core_mass_elements)
      .def_readonly("last_prognostic_bin", &CARMAGroupProperties::last_prognostic_bin)
      .def_readonly("number_of_monomers_per_bin", &CARMAGroupProperties::number_of_monomers_per_bin);

  py::class_<CARMAElementProperties>(carma, "CARMAElementProperties")
      .def_readonly("group_index", &CARMAElementProperties::group_index)
      .def_readonly("solute_index", &CARMAElementProperties::solute_index)
      .def_readonly("composition", &CARMAElementProperties::composition)
      .def_readonly("type", &CARMAElementProperties::type)
      .def_readonly("is_shell", &CARMAElementProperties::is_shell)
      .def_readonly("kappa", &CARMAElementProperties::kappa)
      .def_readonly("rho", &CARMAElementProperties::rho)
      .def_readonly("refidx", &CARMAElementProperties::refidx)
      .def_readonly("number_of_refractive_indices", &CARMAElementProperties::number_of_refractive_indices)
      .def_readonly("number_of_wavelengths", &CARMAElementProperties::number_of_wavelengths);

  py::class_<CARMA>(carma, "CARMA")
      .def(
          py::init(
              [](const CARMAParameters& params)
              {
                try
                {
                  return std::make_unique<CARMA>(params);
                }
                catch (const std::exception& e)
                {
                  throw py::value_error("Error creating CARMA instance: " + std::string(e.what()));
                }
              }),
          py::arg("parameters"))
      .def("get_group_properties", &CARMA::GetGroupProperties, py::arg("group_index"))
      .def("get_element_properties", &CARMA::GetElementProperties, py::arg("element_index"));

  py::class_<CARMAStateParameters>(carma, "CARMAStateParameters")
      .def(py::init<>())
      .def_readwrite("time", &CARMAStateParameters::time)
      .def_readwrite("time_step", &CARMAStateParameters::time_step)
      .def_readwrite("longitude", &CARMAStateParameters::longitude)
      .def_readwrite("latitude", &CARMAStateParameters::latitude)
      .def_readwrite("coordinates", &CARMAStateParameters::coordinates)
      .def_readwrite("vertical_center", &CARMAStateParameters::vertical_center)
      .def_readwrite("vertical_levels", &CARMAStateParameters::vertical_levels)
      .def_readwrite("temperature", &CARMAStateParameters::temperature)
      .def_readwrite("pressure", &CARMAStateParameters::pressure)
      .def_readwrite("pressure_levels", &CARMAStateParameters::pressure_levels)
      .def_readwrite("specific_humidity", &CARMAStateParameters::specific_humidity)
      .def_readwrite("relative_humidity", &CARMAStateParameters::relative_humidity)
      .def_readwrite("original_temperature", &CARMAStateParameters::original_temperature)
      .def_readwrite("radiative_intensity", &CARMAStateParameters::radiative_intensity)
      .def_readwrite("radiative_intensity_dim_1_size", &CARMAStateParameters::radiative_intensity_dim_1_size)
      .def_readwrite("radiative_intensity_dim_2_size", &CARMAStateParameters::radiative_intensity_dim_2_size);

  py::class_<CARMASurfaceProperties>(carma, "CARMASurfaceProperties")
      .def(py::init<>())
      .def_readwrite("surface_friction_velocity", &CARMASurfaceProperties::surface_friction_velocity)
      .def_readwrite("aerodynamic_resistance", &CARMASurfaceProperties::aerodynamic_resistance)
      .def_readwrite("area_fraction", &CARMASurfaceProperties::area_fraction);

  py::class_<CARMAStateStepConfig>(carma, "CARMAStateStepConfig")
      .def(py::init<>())
      .def_readwrite("cloud_fraction", &CARMAStateStepConfig::cloud_fraction)
      .def_readwrite("critical_relative_humidity", &CARMAStateStepConfig::critical_relative_humidity)
      .def_readwrite("land", &CARMAStateStepConfig::land)
      .def_readwrite("ocean", &CARMAStateStepConfig::ocean)
      .def_readwrite("ice", &CARMAStateStepConfig::ice);

  py::class_<CarmaStatistics>(carma, "CarmaStatistics")
      .def_readonly("max_number_of_substeps", &CarmaStatistics::max_number_of_substeps)
      .def_readonly("max_number_of_retries", &CarmaStatistics::max_number_of_retries)
      .def_readonly("total_number_of_steps", &CarmaStatistics::total_number_of_steps)
      .def_readonly("total_number_of_substeps", &CarmaStatistics::total_number_of_substeps)
      .def_readonly("total_number_of_retries", &CarmaStatistics::total_number_of_retries)
      .def_readonly("z_substeps", &CarmaStatistics::z_substeps)
      .def_readonly("xc", &CarmaStatistics::xc)
      .def_readonly("yc", &CarmaStatistics::yc);

  py::class_<CarmaBinValues>(carma, "CarmaBinValues")
      .def_readonly("mass_mixing_ratio", &CarmaBinValues::mass_mixing_ratio)
      .def_readonly("number_mixing_ratio", &CarmaBinValues::number_mixing_ratio)
      .def_readonly("number_density", &CarmaBinValues::number_density)
      .def_readonly("nucleation_rate", &CarmaBinValues::nucleation_rate)
      .def_readonly("wet_particle_radius", &CarmaBinValues::wet_particle_radius)
      .def_readonly("wet_particle_density", &CarmaBinValues::wet_particle_density)
      .def_readonly("dry_particle_density", &CarmaBinValues::dry_particle_density)
      .def_readonly("particle_mass_on_surface", &CarmaBinValues::particle_mass_on_surface)
      .def_readonly("sedimentation_flux", &CarmaBinValues::sedimentation_flux)
      .def_readonly("fall_velocity", &CarmaBinValues::fall_velocity)
      .def_readonly("deposition_velocity", &CarmaBinValues::deposition_velocity)
      .def_readonly("delta_particle_temperature", &CarmaBinValues::delta_particle_temperature)
      .def_readonly("kappa", &CarmaBinValues::kappa)
      .def_readonly("total_mass_mixing_ratio", &CarmaBinValues::total_mass_mixing_ratio);

  py::class_<CarmaDetrainValues>(carma, "CarmaDetrainValues")
      .def_readonly("mass_mixing_ratio", &CarmaDetrainValues::mass_mixing_ratio)
      .def_readonly("number_mixing_ratio", &CarmaDetrainValues::number_mixing_ratio)
      .def_readonly("number_density", &CarmaDetrainValues::number_density)
      .def_readonly("wet_particle_radius", &CarmaDetrainValues::wet_particle_radius)
      .def_readonly("wet_particle_density", &CarmaDetrainValues::wet_particle_density);

  py::class_<CarmaGasValues>(carma, "CarmaGasValues")
      .def_readonly("mass_mixing_ratio", &CarmaGasValues::mass_mixing_ratio)
      .def_readonly("gas_saturation_wrt_ice", &CarmaGasValues::gas_saturation_wrt_ice)
      .def_readonly("gas_saturation_wrt_liquid", &CarmaGasValues::gas_saturation_wrt_liquid)
      .def_readonly("gas_vapor_pressure_wrt_ice", &CarmaGasValues::gas_vapor_pressure_wrt_ice)
      .def_readonly("gas_vapor_pressure_wrt_liquid", &CarmaGasValues::gas_vapor_pressure_wrt_liquid)
      .def_readonly("weight_pct_aerosol_composition", &CarmaGasValues::weight_pct_aerosol_composition);

  py::class_<CarmaEnvironmentalValues>(carma, "CarmaEnvironmentalValues")
      .def_readonly("temperature", &CarmaEnvironmentalValues::temperature)
      .def_readonly("pressure", &CarmaEnvironmentalValues::pressure)
      .def_readonly("air_density", &CarmaEnvironmentalValues::air_density)
      .def_readonly("latent_heat", &CarmaEnvironmentalValues::latent_heat);

  py::class_<CARMAState>(carma, "CARMAState")
      .def(
          py::init(
              [](const CARMA& carma_instance, const CARMAStateParameters& params)
              {
                try
                {
                  return std::make_unique<CARMAState>(carma_instance, params);
                }
                catch (const std::exception& e)
                {
                  throw py::value_error("Error creating CARMA state: " + std::string(e.what()));
                }
              }),
          py::arg("carma"),
          py::arg("parameters"),
          py::keep_alive<1, 2>())
      .def("set_bin", &CARMAState::SetBin)
      .def("set_detrain", &CARMAState::SetDetrain)
      .def("set_gas", &CARMAState::SetGas)
      .def("set_temperature", &CARMAState::SetTemperature)
      .def("set_air_density", &CARMAState::SetAirDensity)
      .def("get_step_statistics", &CARMAState::GetStepStatistics)
      .def("get_bin_values", &CARMAState::GetBinValues)
      .def("get_detrain", &CARMAState::GetDetrain)
      .def("get_gas", &CARMAState::GetGas)
      .def("get_environmental_values", &CARMAState::GetEnvironmentalValues)
      .def(
          "step",
          [](CARMAState& self, CARMAStateStepConfig& step_config)
          {
            try
            {
              self.Step(step_config);
            }
            catch (const std::exception& e)
            {
              throw py::value_error("Error stepping CARMA state: " + std::string(e.what()));
            }
          });
}
