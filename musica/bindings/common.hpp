#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;

PYBIND11_MAKE_OPAQUE(std::vector<double>)

void bind_all(py::module_ &m);
void bind_cuda(py::module_ &);
void bind_musica(py::module_ &);
#ifdef MUSICA_USE_TUVX
void bind_tuvx(py::module_ &);
void bind_tuvx_grid(py::module_ &);
void bind_tuvx_grid_map(py::module_ &);
void bind_tuvx_profile(py::module_ &);
void bind_tuvx_profile_map(py::module_ &);
#endif
#ifdef MUSICA_USE_CARMA
void bind_carma(py::module_ &);
#endif
