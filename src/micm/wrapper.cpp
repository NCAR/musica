#include <pybind11/pybind11.h>
#include <pybind11/stl.h> 
#include <musica/micm.hpp>

namespace py = pybind11;


//Wraps micm.cpp
PYBIND11_MODULE(micm, m) {
    py::class_<MICM>(m, "MICM")
        .def(py::init<>())
        .def("create_solver", &MICM::create_solver)
        .def("solve", &MICM::solve)
        .def("__del__", [](MICM &micm) {
            std::cout << "MICM destructor called" << std::endl;
        });

    m.def("create_micm", [](const char* config_path) {
        int error_code;
        MICM* micm = create_micm(config_path, &error_code);
        return micm;
    });

    m.def("delete_micm", &delete_micm);

    m.def("micm_solve", [](MICM* micm, double time_step, double temperature, double pressure, py::list concentrations) {
        std::vector<double> concentrations_cpp;
        for (auto item : concentrations) {
            concentrations_cpp.push_back(item.cast<double>());
        }
        micm_solve(micm, time_step, temperature, pressure, concentrations_cpp.size(), concentrations_cpp.data());
        
         // Update the concentrations list after solving
        for (size_t i = 0; i < concentrations_cpp.size(); ++i) {
            concentrations[i] = concentrations_cpp[i];
        }
    });

}