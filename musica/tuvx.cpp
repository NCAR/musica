#include "binding_common.hpp"

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
      [](const char* config_string)
      {
        try
        {
          auto tuvx_instance = new musica::TUVX();
          tuvx_instance->CreateFromConfigString(config_string);
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
      [](const char* config_path)
      {
        try
        {
          auto tuvx_instance = new musica::TUVX();
          tuvx_instance->CreateFromConfigFile(config_path);
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
      [](std::uintptr_t tuvx_ptr)
      {
        musica::TUVX* tuvx_instance = reinterpret_cast<musica::TUVX*>(tuvx_ptr);

        // Get dimensions
        int n_photolysis = tuvx_instance->GetPhotolysisRateConstantCount();
        int n_heating = tuvx_instance->GetHeatingRateCount();
        int n_dose = tuvx_instance->GetDoseRateCount();
        int n_layers = tuvx_instance->GetNumberOfLayers();
        int n_sza_steps = tuvx_instance->GetNumberOfSzaSteps();

        // Allocate output arrays (3D: sza_step, layer, reaction/heating_type)
        std::vector<double> photolysis_rates(n_sza_steps * n_layers * n_photolysis);
        std::vector<double> heating_rates(n_sza_steps * n_layers * n_heating);
        std::vector<double> dose_rates(n_sza_steps * n_layers * n_dose);

        // Run TUV-x (everything comes from the JSON config)
        tuvx_instance->RunFromConfig(photolysis_rates.data(), heating_rates.data(), dose_rates.data());

        // Return as numpy arrays with shape (n_sza_steps, n_layers, n_reactions/n_heating)
        py::array_t<double> py_photolysis =
            py::array_t<double>({ n_sza_steps, n_layers, n_photolysis }, photolysis_rates.data());
        py::array_t<double> py_heating = py::array_t<double>({ n_sza_steps, n_layers, n_heating }, heating_rates.data());
        py::array_t<double> py_dose = py::array_t<double>({ n_sza_steps, n_layers, n_dose }, dose_rates.data());

        return py::make_tuple(py_photolysis, py_heating, py_dose);
      },
      "Run TUV-x (all parameters come from JSON config)",
      py::arg("tuvx_instance"));

  tuvx.def(
      "_get_photolysis_rate_constants_ordering",
      [](std::uintptr_t tuvx_ptr)
      {
        musica::TUVX* tuvx_instance = reinterpret_cast<musica::TUVX*>(tuvx_ptr);

        musica::Error error;
        musica::Mappings rate_map = tuvx_instance->GetPhotolysisRateConstantsOrdering(&error);
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
        musica::Mappings rate_map = tuvx_instance->GetHeatingRatesOrdering(&error);
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
        musica::Mappings rate_map = tuvx_instance->GetDoseRatesOrdering(&error);
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
