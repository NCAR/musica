#include "binding_common.hpp"

void bind_cuda(py::module_ &);
void bind_musica(py::module_ &);

void bind_mechanism_configuration(py::module_ &);

void bind_all(py::module_ &m) {
    py::module_ core = m.def_submodule("_core", "Wrapper classes for MUSICA C library structs and functions");
    py::module_ mechanism_configuration = m.def_submodule("_mechanism_configuration", "Wrapper classes for Mechanism Configuration library structs and functions");

    bind_cuda(core);
    bind_musica(core);

    bind_mechanism_configuration(mechanism_configuration);
}