#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <musica/micm.hpp>

namespace py = pybind11;

// Wraps micm.cpp
PYBIND11_MODULE(musica, m)
{
    py::class_<MICM>(m, "MICM")
        .def(py::init<>())
        .def("create_solver", &MICM::create_solver)
        .def("solve", &MICM::solve)
        .def("__del__", [](MICM &micm) {});

    m.def("create_micm", [](const char *config_path)
          {
        int error_code;
        MICM* micm = create_micm(config_path, &error_code);
        return micm; });

    m.def("delete_micm", &delete_micm);

    m.def(
        "micm_solve", [](MICM *micm, double time_step, double temperature, double pressure, py::list concentrations, py::object custom_rate_parameters = py::none())
        {
        std::vector<double> concentrations_cpp;
        for (auto item : concentrations) {
            concentrations_cpp.push_back(item.cast<double>());
        }

        std::vector<double> custom_rate_parameters_cpp;
        if (!custom_rate_parameters.is_none()) {
            py::list parameters = custom_rate_parameters.cast<py::list>();
            for (auto item : parameters) {
                custom_rate_parameters_cpp.push_back(item.cast<double>());
            }
        }

        micm_solve(micm, time_step, temperature, pressure, 
            concentrations_cpp.size(), concentrations_cpp.data(), 
            custom_rate_parameters_cpp.size(), custom_rate_parameters_cpp.data());
        
         // Update the concentrations list after solving
        for (size_t i = 0; i < concentrations_cpp.size(); ++i) {
            concentrations[i] = concentrations_cpp[i];
        } },
        "Solve the system");

    m.def(
        "species_ordering", [](MICM *micm)
        { return micm->get_species_ordering(); },
        "Return map of get_species_ordering rates");

    m.def(
        "user_defined_reaction_rates", [](MICM *micm)
        { return micm->get_user_defined_reaction_rates_ordering(); },
        "Return map of reaction rates");
}