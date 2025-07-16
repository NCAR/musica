#include "binding_common.hpp"

#include <musica/carma/carma.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

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
          carma_instance->Run(params);
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