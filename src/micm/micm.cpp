#include <musica/micm/micm.hpp>

#include <micm/configure/solver_config.hpp>
#include <micm/solver/rosenbrock_solver_parameters.hpp>

#include <filesystem>
#include <cstddef>

MICM::MICM() : solver_(nullptr) {}

MICM::~MICM() {}

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

    std::cout << "Grid cells: " << NUM_GRID_CELLS << std::endl;
    for (size_t i{}; i < NUM_GRID_CELLS; i++)
    {
        state.conditions_[i].temperature_ = temperature;
        state.conditions_[i].pressure_ = pressure;
    }

    std::cout << concentrations[0] << std::endl;
    // std::cout << state.variables_.AsVector()[0] << std::endl;

    // state.variables_.AsVector().assign(concentrations, concentrations + num_concentrations);

    // auto result = solver_->Solve<false>(time_step, state);

    // for (int i=0; i < result.result_.AsVector().size(); i++)
    // {
    //     concentrations[i] = result.result_.AsVector()[i];
    // }
}