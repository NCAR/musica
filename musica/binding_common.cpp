#include "binding_common.hpp"

void bind_mechanism_configuration(py::module_ &);

void bind_all(py::module_ &m)
{
  py::bind_vector<std::vector<double>>(m, "VectorDouble");

  py::module_ core = m.def_submodule("_core", "Wrapper classes for MUSICA C library structs and functions");
  py::module_ mechanism_configuration = m.def_submodule(
      "_mechanism_configuration", "Wrapper classes for Mechanism Configuration library structs and functions");
  py::module_ tuvx = m.def_submodule("_tuvx", "Wrapper classes for TUV-x photolysis calculator");
  py::module_ carma = m.def_submodule("_carma", "Wrapper classes for CARMA photolysis calculator");

  bind_cuda(core);
  bind_musica(core);
#ifdef MUSICA_USE_TUVX
  bind_tuvx_grid(tuvx);
  bind_tuvx_grid_map(tuvx);
  bind_tuvx_profile(tuvx);
  bind_tuvx_profile_map(tuvx);
  bind_tuvx(tuvx);
#endif

#ifdef MUSICA_USE_CARMA
  bind_carma(carma);
#endif

  bind_mechanism_configuration(mechanism_configuration);
}