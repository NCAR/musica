#include <pybind11/pybind11.h>
#include <pybind11/stl.h> 
#include <musica/micm.hpp>

namespace py = pybind11;

PYBIND11_MODULE(micm, m) {
    py::class_<MICM>(m, "MICM")
        .def(py::init<>())
        .def("create_solver", &MICM::create_solver)
        .def("solve", &MICM::solve)
        .def("__del__", [](MICM &micm) {
            // Custom destructor
            std::cout << "MICM destructor called" << std::endl;
        });

    m.def("create_micm", [](const char* config_path) {
        int error_code;
        MICM* micm = create_micm(config_path, &error_code);
        return std::make_tuple(micm, error_code);
    });

    m.def("delete_micm", &delete_micm);

}