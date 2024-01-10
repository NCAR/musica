#include "../../include/micm/micm_c.h" // TODO(jiwon) - relative path?
#include "../../include/micm/micm.hpp"


Micm* create_micm(const char* config_path)
{
    return new MICM(std::string(config_path));
}

void delete_micm(const Micm* micm)
{
    delete micm;
}

int micm_create_solver(Micm* micm)
{
    return micm->create_solver();
}

void micm_solve(Micm* micm, double temperature, double pressure, double time_step, double* concentrations, size_t num_concentrations)
{
    micm->solve(temperature, pressure, time_step, concentrations, num_concentrations);
}