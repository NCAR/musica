#include "common.hpp"

void bind_mechanism_configuration(py::module_ &);

void bind_all(py::module_ &m)
{
  py::bind_vector<std::vector<double>>(m, "VectorDouble");

  py::module_ micm = m.def_submodule("_micm", "Wrapper classes for MICM");
  py::module_ mechanism_configuration = m.def_submodule(
      "_mechanism_configuration",
      "Wrapper classes for Mechanism Configuration for the chemical system configuration schema");
  py::module_ tuvx = m.def_submodule("_tuvx", "Wrapper classes for TUV-x photolysis calculator");
  py::module_ carma = m.def_submodule("_carma", "Wrapper classes for CARMA for modeling clouds and aerosols");

  bind_cuda(micm);
  bind_micm(micm);
  bind_micm_conditions(micm);
  bind_micm_solver(micm);
  bind_micm_state(micm);
  bind_mechanism_configuration(mechanism_configuration);

#ifdef MUSICA_USE_TUVX
  bind_tuvx_grid(tuvx);
  bind_tuvx_grid_map(tuvx);
  bind_tuvx_profile(tuvx);
  bind_tuvx_profile_map(tuvx);
  bind_tuvx_radiator(tuvx);
  bind_tuvx_radiator_map(tuvx);
  bind_tuvx(tuvx);
#endif

#ifdef MUSICA_USE_CARMA
  bind_carma(carma);
#endif
}