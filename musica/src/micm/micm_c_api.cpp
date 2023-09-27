#include "../../include/micm/micm_c.h" // TODO(jiwon) - relative path?
#include "../../include/micm/micm.hpp"


Micm* create_micm(const char* config_path)
{
    std::cout << "   * [C API] Creating MICM" << std::endl;

    return new MICM(std::string(config_path));
}

void delete_micm(const Micm* micm)
{
    std::cout << "   * [C API] Deleting MICM" << std::endl;
    
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