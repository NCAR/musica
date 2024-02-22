#include <musica/micm/micm.hpp>

#include <micm/configure/solver_config.hpp>

#include <filesystem>

MICM::MICM()
    :  solver_(nullptr)
    {}

MICM::~MICM()
{   
    delete solver_;
}

int MICM::create_solver(std::string config_path)
{
    int failure = 0;

    micm::SolverConfig solver_config;

    micm::ConfigParseStatus status= solver_config.ReadAndParse(std::filesystem::path(config_path));

    if (status == micm::ConfigParseStatus::Success)
    {
        micm::SolverParameters solver_params = solver_config.GetSolverParams();
        auto params = micm::RosenbrockSolverParameters::three_stage_rosenbrock_parameters(NUM_GRID_CELLS);
        params.reorder_state_ = false;
        solver_ = new VectorRosenbrockSolver{solver_params.system_,
                                            solver_params.processes_,
                                            params};
    }
    else
    {
        // TODO(jiwon) populate error msg to atmospheric physics
        // std::cout << "int MICM::create_solver() Failed " << std::endl;
        failure = 1;
    }

    return failure;
}

void MICM::solve(double time_step, double temperature, double pressure, int num_concentrations, double*& concentrations)
{
    // TODO(jiwon) set concentration according to the species
    micm::State state = solver_->GetState();

    for(size_t i{}; i < NUM_GRID_CELLS; ++i) {
        state.conditions_[i].temperature_ = temperature;
        state.conditions_[i].pressure_ = pressure;
    }

    v_concentrations_.assign(concentrations, concentrations + num_concentrations);
    state.variables_[0] = v_concentrations_;

    auto result = solver_->Solve(time_step, state);

    v_concentrations_ = result.result_.AsVector();
    for (int i=0; i<v_concentrations_.size(); i++)
    {
        concentrations[i] = v_concentrations_[i];
    }
}