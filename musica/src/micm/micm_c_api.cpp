#include <micm/micm_c.h>
#include <micm/micm.hpp>

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

void micm_solve(Micm* micm, double temperature, double pressure, double time_step, int num_concentrations, double* concentrations)
{
    micm->solve(temperature, pressure, time_step, num_concentrations, concentrations);
}