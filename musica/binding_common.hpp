#pragma once

#include <musica/util.hpp>

#include <pybind11/pybind11.h>

#include <type_traits>

namespace py = pybind11;

void bind_all(py::module_ &m);