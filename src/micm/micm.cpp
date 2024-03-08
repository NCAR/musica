#include <cstddef>
#include <filesystem>
#include <iostream>

#include <micm/configure/solver_config.hpp>
#include <micm/solver/rosenbrock_solver_parameters.hpp>
#include <musica/micm.hpp>
#include <musica/micm.hpp>

void create_micm(void **micm)
{
    *micm = new MICM();
}

void delete_micm(void **micm)
{
    if (*micm)
    {
        delete static_cast<MICM *>(*micm);
        *micm = nullptr;
    }
}

int micm_create_solver(void **micm, const char *config_path)
{
    return static_cast<MICM *>(*micm)->create_solver(std::string(config_path));
}

void micm_solve(void **micm, double time_step, double temperature, double pressure, int num_concentrations, double *concentrations)
{
    static_cast<MICM *>(*micm)->solve(time_step, temperature, pressure, num_concentrations, concentrations);
}

MICM::MICM() : solver_(nullptr) {}

MICM::~MICM()
{
    std::cout << "MICM destructor called" << std::endl;
}

int MICM::create_solver(const std::string &config_path)
{
    int parsing_status = 0; // 0 on success, 1 on failure

    micm::SolverConfig<> solver_config;
    micm::ConfigParseStatus status = solver_config.ReadAndParse(std::filesystem::path(config_path));

    if (status == micm::ConfigParseStatus::Success)
    {
        micm::SolverParameters solver_params = solver_config.GetSolverParams();
        auto params = micm::RosenbrockSolverParameters::three_stage_rosenbrock_parameters(NUM_GRID_CELLS);
        solver_ = std::make_unique<micm::RosenbrockSolver<>>(solver_params.system_,
                                                             solver_params.processes_,
                                                             params);
    }
    else
    {
        parsing_status = 1;
    }

    return parsing_status;
}

void MICM::solve(double time_step, double temperature, double pressure, int num_concentrations, double *concentrations)
{
    micm::State state = solver_->GetState();

    for (size_t i{}; i < NUM_GRID_CELLS; i++)
    {
        state.conditions_[i].temperature_ = temperature;
        state.conditions_[i].pressure_ = pressure;
    }

    state.variables_.AsVector().assign(concentrations, concentrations + num_concentrations);

    auto result = solver_->Solve<false>(time_step, state);

    for (int i = 0; i < result.result_.AsVector().size(); i++)
    {
        concentrations[i] = result.result_.AsVector()[i];
    }
}