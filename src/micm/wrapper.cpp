#include <pybind11/pybind11.h>
#include <pybind11/stl.h> 
#include <musica/micm.hpp>

namespace py = pybind11;

PYBIND11_MODULE(solver, m) {
    // Expose the MICM class
    py::class_<MICM>(m, "MICM")
        .def(py::init<>()) 
        .def("solve", &MICM::solve); 

    // Expose the helper functions
    m.def("create_micm", &create_micm, "Create MICM instance");
    m.def("delete_micm", &delete_micm, "Delete MICM instance");

}
