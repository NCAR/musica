#include "../../include/micm/MICM_C.h"
#include "../../include/micm/MICM.hpp"

// #include "MICM_C.h"
// #include "MICM.hpp"


Micm* create_micm(const char* config_path)
{
    std::cout << "   * [C API] Creating MICM" << std::endl;
    return new MICM(std::string(config_path));
}

void delete_micm(const Micm* micm)
{
    std::cout << "   * [C API] Deleting MICM" << std::endl;
#include "../../include/micm/MICM.hpp"
    micm->delete_solver();
    delete micm;
}

int micm_create_solver(Micm* micm)
{
    std::cout << "   * [C API] Creating solver" << std::endl;
    return micm->create_solver();
}

void micm_solve(Micm* micm, double temperature, double pressure, double time_step, double* concentrations, size_t num_concentrations)
{
    std::cout << "   * [C API] Starting solving" << std::endl;

    micm->solve(temperature, pressure, time_step, concentrations, num_concentrations);
}