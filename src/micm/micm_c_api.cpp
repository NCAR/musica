#include <musica/micm/micm.hpp>
#include <musica/micm/micm_c.h>

#include <iostream>


Micm* create_micm()
{
    Micm* micm = new MICM();
    std::cout << micm << std::endl;
    return micm;
}

void delete_micm(const Micm* micm)
{
    delete micm;
}

int micm_create_solver(Micm* micm, const char* config_path)
{
    std::cout << micm << std::endl;
    return micm->create_solver(std::string(config_path));
}

void micm_solve(Micm* micm, double time_step, double temperature, double pressure, int num_concentrations, double* concentrations)
{
    std::cout << micm << std::endl;
    std::cout << time_step << " " << temperature << " " << pressure << " " << num_concentrations << " " << *concentrations << std::endl;
    micm->solve(time_step, temperature, pressure, num_concentrations, concentrations);
}