#include <iostream>

// #include "MICM.hpp" // TODO(jiwon) - do we want angle bracket, also how about relative path?
#include "../../include/micm/MICM.hpp"

// #include <nlohmann/json.hpp>  // TODO - remove
#include <micm/configure/solver_config.hpp>


MICM::MICM(const std::string& config_path)
    : config_path_(config_path),
      solver_(nullptr)
    {
        std::cout << "   * [C++] MICM constructor" << std::endl;
    }

MICM::~MICM()
{
    std::cout << "   * [C++] MICM Destructor" << std::endl;
}

int MICM::create_solver()
{
    std::cout << "   * [C++] Creating solver" << std::endl;
    bool success = 1;    // TODO(jiwon): can we specifiy error type with int?

    // read and parse the config
    micm::SolverConfig solver_config;
    micm::ConfigParseStatus status = solver_config.ReadAndParse(config_path_);
    micm::SolverParameters solver_params = solver_config.GetSolverParams();

    if (status == micm::ConfigParseStatus::Success)
    {
        std::cout << "   * [C++] Successfully read confiure file from the direcotry path: "  << config_path_ << std::endl;

        auto params = micm::RosenbrockSolverParameters::three_stage_rosenbrock_parameters(NUM_GRID_CELLS);  // TODO(jiwon) - three_stage
        params.reorder_state_ = false;
        solver_ = new VectorRosenbrockSolver{solver_params.system_,
                                            solver_params.processes_,
                                            params}; 
    }
    else
    {
        std::cout << "   * [C++] Failed creating solver" << std::endl;
        success = 0; 
    }

    return success;
}

void MICM::delete_solver() const
{
    std::cout << "   * [C++] Deallocating solver" << std::endl;
    delete solver_;
}

void MICM::solve(double temperature, double pressure, double time_step, double*& concentrations, size_t num_concentrations)
{
    std::cout << "   * [C++] Start solving " << std::endl;

    v_concentrations_.assign(concentrations, concentrations + num_concentrations);

    micm::State<Vector1MatrixParam> state = solver_->GetState();

    for(size_t i{}; i < NUM_GRID_CELLS; ++i) {
        state.conditions_[i].temperature_ = temperature;
        state.conditions_[i].pressure_ = pressure;
    }

    state.variables_[0] = v_concentrations_;  // TODO:

    auto result = solver_->Solve(time_step, state);
    // state.variables_[0] = result.result_.AsVector();   //TODO(jiwon): doesn't need because stae will get destroyed?
    
    v_concentrations_ = result.result_.AsVector();
    for (int i=0; i<v_concentrations_.size(); i++)
    {
        concentrations[i] = v_concentrations_[i];
    }
}
