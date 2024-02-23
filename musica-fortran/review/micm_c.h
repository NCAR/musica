#ifdef __cplusplus
extern "C" {
#endif

struct MICM;
typedef struct MICM Micm;

Micm* create_micm();
void delete_micm(const Micm* micm);
int micm_create_solver(Micm* micm, const char* config_path);
void micm_solve(Micm* micm, double time_step, double temperature, double pressure, int num_concentrations,
                double* concentrations);

#ifdef __cplusplus
}
#endif