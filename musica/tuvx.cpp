#include "binding_common.hpp"

#include <musica/tuvx/tuvx.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void bind_tuvx(py::module_& tuvx)
{
  tuvx.def("_get_tuvx_version", []() { return musica::TUVX::GetVersion(); }, "Get the version of the TUV-x instance");

  tuvx.def(
      "_create_tuvx",
      [](const char* config_path)
      {
        try
        {
          auto tuvx_instance = new musica::TUVX();
          tuvx_instance->CreateFromConfigOnly(config_path);
          return reinterpret_cast<std::uintptr_t>(tuvx_instance);
        }
        catch (const std::exception& e)
        {
          throw py::value_error(
              "Error creating TUV-x instance from config file: " + std::string(config_path) + " - " + e.what());
        }
      },
      "Create a TUV-x instance from a JSON configuration file");

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
        int n_photolysis = tuvx_instance->GetPhotolysisRateCount();

        int n_heating = tuvx_instance->GetHeatingRateCount();

        int n_layers = tuvx_instance->GetNumberOfLayers();

        int n_sza_steps = tuvx_instance->GetNumberOfSzaSteps();

        // Allocate output arrays (3D: sza_step, layer, reaction/heating_type)
        std::vector<double> photolysis_rates(n_sza_steps * n_layers * n_photolysis);
        std::vector<double> heating_rates(n_sza_steps * n_layers * n_heating);

        // Run TUV-x (everything comes from the JSON config)
        tuvx_instance->RunFromConfig(photolysis_rates.data(), heating_rates.data());

        // Return as numpy arrays with shape (n_sza_steps, n_layers, n_reactions/n_heating)
        py::array_t<double> py_photolysis =
            py::array_t<double>({ n_sza_steps, n_layers, n_photolysis }, photolysis_rates.data());
        py::array_t<double> py_heating = py::array_t<double>({ n_sza_steps, n_layers, n_heating }, heating_rates.data());

        return py::make_tuple(py_photolysis, py_heating);
      },
      "Run TUV-x (all parameters come from JSON config)",
      py::arg("tuvx_instance"));

  tuvx.def(
      "_get_photolysis_rate_names",
      [](std::uintptr_t tuvx_ptr)
      {
        musica::TUVX* tuvx_instance = reinterpret_cast<musica::TUVX*>(tuvx_ptr);

        return tuvx_instance->GetPhotolysisRateNames();
      },
      "Get photolysis rate names");

  tuvx.def(
      "_get_heating_rate_names",
      [](std::uintptr_t tuvx_ptr)
      {
        musica::TUVX* tuvx_instance = reinterpret_cast<musica::TUVX*>(tuvx_ptr);

        return tuvx_instance->GetHeatingRateNames();
      },
      "Get heating rate names");
}
