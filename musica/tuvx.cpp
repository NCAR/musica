#include "binding_common.hpp"

#include <musica/tuvx/tuvx.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void bind_tuvx(py::module_& tuvx)
{
  tuvx.def("_get_tuvx_version", []() { return musica::TUVX::GetVersion(); }, "Get the version of the TUV-x instance");
}
