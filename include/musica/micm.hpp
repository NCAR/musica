#pragma once

#include <micm/solver/rosenbrock.hpp>

#include <memory>
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

void create_micm(void** micm, int error_code);
void delete_micm(void** micm);
int micm_create_solver(void** micm, const char* config_path);
void micm_solve(void** micm, double time_step, double temperature, double pressure, int num_concentrations,
                double* concentrations);

#ifdef __cplusplus
}
#endif

class MICM
{
public:
    /// @brief Constructor
    MICM();

    /// @brief Destructor
    ~MICM();

    /// @brief Create a solver by reading and parsing configuration file
    /// @param config_path Path to configuration file or directory containing configuration file
    /// @return 0 on success, 1 on failure in parsing configuration file
    int create_solver(const std::string& config_path);
    
    /// @brief Solve the system
    /// @param time_step Time [s] to advance the state by
    /// @param temperature Temperature [K]
    /// @param pressure Pressure [Pa-1]
    /// @param num_concentrations The number of species' concentrations
    /// @param concentrations Species's concentrations
    void solve(double time_step, double temperature, double pressure, int num_concentrations, double* concentrations);

    static constexpr size_t NUM_GRID_CELLS = 1;

private:
    std::unique_ptr<micm::RosenbrockSolver<>> solver_;
};