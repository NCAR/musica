#include <musica/micm/micm_c.h>

#include <stdio.h>


int main()
{
    const char* config_path = "chapman";
    Micm* micm =create_micm();

    printf("[test micm c api] Parsing configuration file: %s\n" , config_path);
    int solver_creation_status = micm_create_solver(micm, config_path);

    double time_step = 200.0;
    double temperature = 272.5;
    double pressure = 101253.3;
    int num_concentrations = 5;
    double concentrations[] = {0.75, 0.4, 0.8, 0.01, 0.02};

    for (int i=0; i<num_concentrations; i++)
    {
        printf("[test micm c api] Initial concentrations:\t%e\n", concentrations[i]);
    }

    if (solver_creation_status == 0)
    {
        printf("[test micm c api] Created MICM solver. Solving starts...\n");

        micm_solve(micm, time_step, temperature, pressure, num_concentrations, concentrations);

        printf("[test micm c api] Finished solving.\n");

        for (int i=0; i<num_concentrations; i++)
        {
            printf("[test micm c api] Solved concentrations:\t%e\n", concentrations[i]);
        }
    }
    else
    {
        printf("[test micm c api] Failed in creating solver.\n");
    }

    delete_micm(micm);
}