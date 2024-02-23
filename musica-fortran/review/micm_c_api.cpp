#include "micm_c.h"
#include "micm.hpp"

// #include <./../../../musica/include/musica/micm/micm_c.h>
// #include <./../../../musica/include/musica/micm/micm.hpp>

Micm* create_micm()
{
    return new MICM();
}

void delete_micm(const Micm* micm)
{
    delete micm;
}

int micm_create_solver(Micm* micm, const char* config_path)
{
    return micm->create_solver(std::string(config_path));
}

void micm_solve(Micm* micm, double time_step, double temperature, double pressure, int num_concentrations, double* concentrations)
{
    micm->solve(time_step, temperature, pressure, num_concentrations, concentrations);
}