#ifdef __cplusplus
extern "C" {
#endif

void create_micm(void** micm);
void delete_micm(void** micm);
int micm_create_solver(void** micm, const char* config_path);
void micm_solve(void** micm, double time_step, double temperature, double pressure, int num_concentrations,
                double* concentrations);

#ifdef __cplusplus
}
#endif