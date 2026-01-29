#include "../common.hpp"

#include <musica/tuvx/tuvx.hpp>
#include <musica/util.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void bind_tuvx(py::module_& tuvx)
{
  tuvx.def("_get_tuvx_version", []() { return musica::TUVX::GetVersion(); }, "Get the version of the TUV-x instance");

  tuvx.def(
      "_create_tuvx_from_string",
      [](const char* config_string, musica::GridMap* grids, musica::ProfileMap* profiles, musica::RadiatorMap* radiators)
      {
        try
        {
          musica::Error error;
          auto tuvx_instance = new musica::TUVX();
          tuvx_instance->CreateFromConfigString(config_string, grids, profiles, radiators, &error);
          if (!musica::IsSuccess(error))
          {
            throw py::value_error("Error creating TUV-x instance from config string: " + std::string(error.message_.value_));
          }
          return reinterpret_cast<std::uintptr_t>(tuvx_instance);
        }
        catch (const std::exception& e)
        {
          throw py::value_error("Error creating TUV-x instance from config string: " + std::string(e.what()));
        }
      },
      "Create a TUV-x instance from a JSON/YAML configuration string");

  tuvx.def(
      "_create_tuvx_from_file",
      [](const char* config_path, musica::GridMap* grids, musica::ProfileMap* profiles, musica::RadiatorMap* radiators)
      {
        try
        {
          musica::Error error;
          auto tuvx_instance = new musica::TUVX();
          tuvx_instance->Create(config_path, grids, profiles, radiators, &error);
          if (!musica::IsSuccess(error))
          {
            throw py::value_error("Error creating TUV-x instance from config file: " + std::string(error.message_.value_));
          }
          return reinterpret_cast<std::uintptr_t>(tuvx_instance);
        }
        catch (const std::exception& e)
        {
          throw py::value_error(
              "Error creating TUV-x instance from config file: " + std::string(config_path) + " - " + e.what());
        }
      },
      "Create a TUV-x instance from a JSON/YAML configuration file");

  tuvx.def(
      "_delete_tuvx",
      [](std::uintptr_t tuvx_ptr)
      {
        musica::TUVX* tuvx_instance = reinterpret_cast<musica::TUVX*>(tuvx_ptr);
        delete tuvx_instance;
      },
      "Delete a TUV-x instance");

  tuvx.def(
      "_run_tuvx",
      [](std::uintptr_t tuvx_ptr, double sza_radians, double earth_sun_distance)
      {
        musica::TUVX* tuvx_instance = reinterpret_cast<musica::TUVX*>(tuvx_ptr);

        // Get dimensions
        int n_photolysis = tuvx_instance->GetPhotolysisRateConstantCount();
        int n_heating = tuvx_instance->GetHeatingRateCount();
        int n_dose = tuvx_instance->GetDoseRateCount();
        int n_layers = tuvx_instance->GetNumberOfHeightMidpoints();
        int n_wavelengths = tuvx_instance->GetNumberOfWavelengthMidpoints();

        // Allocate output arrays on the heap (2D: reaction/heating reaction/dose rate type, vertical edge)
        auto photolysis_rates = new std::vector<double>(n_photolysis * (n_layers + 1));
        auto heating_rates = new std::vector<double>(n_heating * (n_layers + 1));
        auto dose_rates = new std::vector<double>(n_dose * (n_layers + 1));
        // ... and 3D arrays for actinic flux and spectral irradiance
        // (wavelength, vertical edge, 3 components: direct, upwelling, downwelling)
        auto actinic_flux = new std::vector<double>(n_wavelengths * (n_layers + 1) * 3);
        auto spectral_irradiance = new std::vector<double>(n_wavelengths * (n_layers + 1) * 3);

        // Run TUV-x
        musica::Error error;
        tuvx_instance->Run(
            sza_radians,
            earth_sun_distance,
            photolysis_rates->data(),
            heating_rates->data(),
            dose_rates->data(),
            actinic_flux->data(),
            spectral_irradiance->data(),
            &error);

        if (!musica::IsSuccess(error))
        {
          std::string error_message = std::string(error.message_.value_);
          musica::DeleteError(&error);
          // Clean up heap allocations before throwing
          delete photolysis_rates;
          delete heating_rates;
          delete dose_rates;
          delete actinic_flux;
          delete spectral_irradiance;
          throw py::value_error("Error running TUV-x: " + error_message);
        }
        musica::DeleteError(&error);

        // Create capsules to manage the lifetime of the heap-allocated vectors
        auto photolysis_capsule =
            py::capsule(photolysis_rates, [](void* v) { delete reinterpret_cast<std::vector<double>*>(v); });
        auto heating_capsule = py::capsule(heating_rates, [](void* v) { delete reinterpret_cast<std::vector<double>*>(v); });
        auto dose_capsule = py::capsule(dose_rates, [](void* v) { delete reinterpret_cast<std::vector<double>*>(v); });
        auto actinic_flux_capsule =
            py::capsule(actinic_flux, [](void* v) { delete reinterpret_cast<std::vector<double>*>(v); });
        auto spectral_irradiance_capsule =
            py::capsule(spectral_irradiance, [](void* v) { delete reinterpret_cast<std::vector<double>*>(v); });

        // Return as numpy arrays with shape (reaction/heating reaction/dose rate type, vertical edge)
        py::array_t<double> py_photolysis =
            py::array_t<double>({ n_photolysis, n_layers + 1 }, photolysis_rates->data(), photolysis_capsule);
        py::array_t<double> py_heating =
            py::array_t<double>({ n_heating, n_layers + 1 }, heating_rates->data(), heating_capsule);
        py::array_t<double> py_dose = py::array_t<double>({ n_dose, n_layers + 1 }, dose_rates->data(), dose_capsule);
        // ... and 3D arrays for actinic flux and spectral irradiance
        // (wavelength, vertical edge, 3 components: direct, upwelling, downwelling)
        py::array_t<double> py_actinic_flux =
            py::array_t<double>({ n_wavelengths, n_layers + 1, 3 }, actinic_flux->data(), actinic_flux_capsule);
        py::array_t<double> py_spectral_irradiance = py::array_t<double>(
            { n_wavelengths, n_layers + 1, 3 }, spectral_irradiance->data(), spectral_irradiance_capsule);

        return py::make_tuple(py_photolysis, py_heating, py_dose, py_actinic_flux, py_spectral_irradiance);
      },
      "Run TUV-x (all parameters come from JSON config)",
      py::arg("tuvx_instance"),
      py::arg("sza_radians"),
      py::arg("earth_sun_distance"));

  tuvx.def(
      "_get_grid_map",
      [](std::uintptr_t tuvx_ptr) -> musica::GridMap*
      {
        musica::TUVX* tuvx_instance = reinterpret_cast<musica::TUVX*>(tuvx_ptr);

        musica::Error error;
        musica::GridMap* grid_map = tuvx_instance->GetGridMap(&error);
        if (!musica::IsSuccess(error))
        {
          std::string error_message = std::string(error.message_.value_);
          musica::DeleteError(&error);
          throw py::value_error("Error getting GridMap from TUV-x instance: " + error_message);
        }
        musica::DeleteError(&error);

        return grid_map;
      },
      "Get the GridMap used in this TUV-x instance");

  tuvx.def(
      "_get_profile_map",
      [](std::uintptr_t tuvx_ptr) -> musica::ProfileMap*
      {
        musica::TUVX* tuvx_instance = reinterpret_cast<musica::TUVX*>(tuvx_ptr);

        musica::Error error;
        musica::ProfileMap* profile_map = tuvx_instance->GetProfileMap(&error);
        if (!musica::IsSuccess(error))
        {
          std::string error_message = std::string(error.message_.value_);
          musica::DeleteError(&error);
          throw py::value_error("Error getting ProfileMap from TUV-x instance: " + error_message);
        }
        musica::DeleteError(&error);

        return profile_map;
      },
      "Get the ProfileMap used in this TUV-x instance");

  tuvx.def(
      "_get_radiator_map",
      [](std::uintptr_t tuvx_ptr) -> musica::RadiatorMap*
      {
        musica::TUVX* tuvx_instance = reinterpret_cast<musica::TUVX*>(tuvx_ptr);

        musica::Error error;
        musica::RadiatorMap* radiator_map = tuvx_instance->GetRadiatorMap(&error);
        if (!musica::IsSuccess(error))
        {
          std::string error_message = std::string(error.message_.value_);
          musica::DeleteError(&error);
          throw py::value_error("Error getting RadiatorMap from TUV-x instance: " + error_message);
        }
        musica::DeleteError(&error);

        return radiator_map;
      },
      "Get the RadiatorMap used in this TUV-x instance");

  tuvx.def(
      "_get_photolysis_rate_constants_ordering",
      [](std::uintptr_t tuvx_ptr)
      {
        musica::TUVX* tuvx_instance = reinterpret_cast<musica::TUVX*>(tuvx_ptr);

        musica::Error error;
        musica::Mappings rate_map;
        tuvx_instance->GetPhotolysisRateConstantsOrdering(&rate_map, &error);
        if (!musica::IsSuccess(error))
        {
          std::string error_message = std::string(error.message_.value_);
          musica::DeleteError(&error);
          throw py::value_error("Error getting photolysis rate constants ordering: " + error_message);
        }
        musica::DeleteError(&error);

        py::dict rate_dict;
        for (std::size_t i = 0; i < rate_map.size_; ++i)
        {
          py::str name(rate_map.mappings_[i].name_.value_);
          std::size_t index = rate_map.mappings_[i].index_;
          rate_dict[name] = index;
        }
        return rate_dict;
      },
      "Get photolysis rate names and their ordering in the output arrays");

  tuvx.def(
      "_get_heating_rates_ordering",
      [](std::uintptr_t tuvx_ptr)
      {
        musica::TUVX* tuvx_instance = reinterpret_cast<musica::TUVX*>(tuvx_ptr);

        musica::Error error;
        musica::Mappings rate_map;
        tuvx_instance->GetHeatingRatesOrdering(&rate_map, &error);
        if (!musica::IsSuccess(error))
        {
          std::string error_message = std::string(error.message_.value_);
          musica::DeleteError(&error);
          throw py::value_error("Error getting heating rates ordering: " + error_message);
        }
        musica::DeleteError(&error);

        py::dict rate_dict;
        for (std::size_t i = 0; i < rate_map.size_; ++i)
        {
          py::str name(rate_map.mappings_[i].name_.value_);
          std::size_t index = rate_map.mappings_[i].index_;
          rate_dict[name] = index;
        }
        return rate_dict;
      },
      "Get heating rate names and their ordering in the output arrays");

  tuvx.def(
      "_get_dose_rates_ordering",
      [](std::uintptr_t tuvx_ptr)
      {
        musica::TUVX* tuvx_instance = reinterpret_cast<musica::TUVX*>(tuvx_ptr);

        musica::Error error;
        musica::Mappings rate_map;
        tuvx_instance->GetDoseRatesOrdering(&rate_map, &error);
        if (!musica::IsSuccess(error))
        {
          std::string error_message = std::string(error.message_.value_);
          musica::DeleteError(&error);
          throw py::value_error("Error getting dose rates ordering: " + error_message);
        }
        musica::DeleteError(&error);

        py::dict rate_dict;
        for (std::size_t i = 0; i < rate_map.size_; ++i)
        {
          py::str name(rate_map.mappings_[i].name_.value_);
          std::size_t index = rate_map.mappings_[i].index_;
          rate_dict[name] = index;
        }
        return rate_dict;
      },
      "Get dose rate names and their ordering in the output arrays");
}
