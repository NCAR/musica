#include <cstddef>

#ifdef __cplusplus
extern "C" {
    class MICM;
    typedef MICM Micm;
#endif

Micm* create_micm(const char* config_path);
void delete_micm(const Micm* micm);
int micm_create_solver(Micm* micm);
void micm_solve(Micm* micm, double temperature, double pressure, double time_step, int num_concentrations, 
                double* concentrations);

#ifdef __cplusplus
}
#endif