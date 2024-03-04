#include <musica/micm/micm.hpp>
#include <musica/micm/micm_c.h>

#include <iostream>

void create_micm(void **micm)
{
    *micm = new MICM();
}

void delete_micm(void **micm)
{
    delete static_cast<MICM *>(*micm);
    *micm = nullptr;
}

int micm_create_solver(void **micm, const char *config_path)
{
    return static_cast<MICM *>(*micm)->create_solver(std::string(config_path));
}

void micm_solve(void **micm, double time_step, double temperature, double pressure, int num_concentrations, double *concentrations)
{
    std::cout << micm << std::endl;
    std::cout << time_step << " " << temperature << " " << pressure << " " << num_concentrations << " " << *concentrations << std::endl;
    static_cast<MICM *>(*micm)->solve(time_step, temperature, pressure, num_concentrations, concentrations);
}