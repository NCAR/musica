/**
 * @file micm.cpp
 * @brief Implementation of the MICM class and related functions.
 * 
 * This file contains the implementation of the MICM class, which represents a multi-component
 * reactive transport model. It also includes functions for creating and deleting MICM instances,
 * Copyright (C) 2023-2024 National Center for Atmospheric Research,
 * 
 * SPDX-License-Identifier: Apache-2.0* creating solvers, and solving the model.
 */
#include <filesystem>
#include <iostream>

#include <micm/configure/solver_config.hpp>
#include <micm/solver/rosenbrock_solver_parameters.hpp>
#include <musica/micm.hpp>
#include <musica/micm.hpp>

/**
 * @brief Creates a new instance of the MICM class.
 * 
 * This function attempts to create a new instance of the MICM class and assigns it to the provided pointer.
 * If the creation is successful, it sets the error code to 0. If a bad_alloc exception is thrown (indicating
 * that the allocation failed), it sets the error code to 1.
 * 
 * @param micm A pointer to a pointer where the new MICM instance will be stored.
 * @param error_code A pointer to an integer where the error code will be stored.
 */
void create_micm(void** micm, int error_code)
{
    try
    {
        *micm = new MICM(); // Attempt to create a new MICM instance
        error_code = 0; // 0 indicates success
    }
    catch (const std::bad_alloc&)
    {
        error_code = 1; // 1 indicates failure due to bad allocation
    }
}

/**
 * @brief Deletes an instance of the MICM class.
 * 
 * This function attempts to delete an instance of the MICM class pointed to by the provided pointer.
 * If the pointer is not null, it deletes the MICM instance and sets the pointer to null.
 * 
 * @param micm A pointer to a pointer to the MICM instance to be deleted.
 */
void delete_micm(void **micm)
{
    // Check if the pointer is not null
    if (*micm)
    {
        // Delete the MICM instance
        delete static_cast<MICM *>(*micm);
        // Set the pointer to null
        *micm = nullptr;
    }
}


/**
 * @brief Creates a solver for the MICM instance.
 * 
 * This function attempts to create a solver for the MICM instance pointed to by the provided pointer.
 * It uses the provided configuration path to configure the solver.
 * 
 * @param micm A pointer to a pointer to the MICM instance for which the solver will be created.
 * @param config_path A string representing the path to the configuration file for the solver.
 * @return An integer representing the status of the solver creation (0 on success, 1 on failure).
 */
int micm_create_solver(void **micm, const char *config_path)
{
    return static_cast<MICM *>(*micm)->create_solver(std::string(config_path));
}

/**
 * @brief Solves the MICM model.
 * 
 * This function attempts to solve the MICM model for the instance pointed to by the provided pointer.
 * It uses the provided time step, temperature, pressure, number of concentrations, and concentrations.
 * 
 * @param micm A pointer to a pointer to the MICM instance to be solved.
 * @param time_step A double representing the time step for the solver.
 * @param temperature A double representing the temperature for the solver.
 * @param pressure A double representing the pressure for the solver.
 * @param num_concentrations An integer representing the number of concentrations for the solver.
 * @param concentrations A pointer to a double representing the concentrations for the solver.
 */
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