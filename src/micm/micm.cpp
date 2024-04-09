/**
 * This file contains the implementation of the MICM class, which represents a multi-component
 * reactive transport model. It also includes functions for creating and deleting MICM instances,
 * Copyright (C) 2023-2024 National Center for Atmospheric Research,
 *
 * SPDX-License-Identifier: Apache-2.0* creating solvers, and solving the model.
 */
#include <filesystem>
#include <iostream>
#include <cmath>

#include <micm/solver/rosenbrock_solver_parameters.hpp>
#include <micm/system/species.hpp>
#include <musica/micm.hpp>

MICM *create_micm(const char *config_path, int *error_code)
{
    try
    {
        MICM *micm = new MICM();
        *error_code = micm->create_solver(std::string(config_path));
        return micm;
    }
    catch (const std::bad_alloc &e)
    {
        *error_code = 1;
        return nullptr;
    }
}

void delete_micm(const MICM *micm)
{
    delete micm;
}

void micm_solve(MICM *micm, double time_step, double temperature, double pressure, int num_concentrations, double *concentrations, int num_custom_rate_parameters, double *custom_rate_parameters)
{
    micm->solve(time_step, temperature, pressure, num_concentrations, concentrations, num_custom_rate_parameters, custom_rate_parameters);
}

Mapping *get_species_ordering(MICM *micm, size_t *array_size)
{
    auto map = micm->get_species_ordering();
    Mapping *species_ordering = new Mapping[map.size()];

    // Copy data from the map to the array of structs
    size_t i = 0;
    for (const auto &entry : map)
    {
        std::strcpy(species_ordering[i].name, entry.first.c_str());
        species_ordering[i].index = entry.second;
        species_ordering[i].string_length = entry.first.size();
        ++i;
    }

    // Set the size of the array
    *array_size = map.size();

    return species_ordering;
}

Mapping *get_user_defined_reaction_rates_ordering(MICM *micm, size_t *array_size)
{
    auto map = micm->get_user_defined_reaction_rates_ordering();
    Mapping *reactionRates = new Mapping[map.size()];

    // Copy data from the map to the array of structs
    size_t i = 0;
    for (const auto &entry : map)
    {
        std::strcpy(reactionRates[i].name, entry.first.c_str());
        reactionRates[i].index = entry.second;
        reactionRates[i].string_length = entry.first.size();
        ++i;
    }

    // Set the size of the array
    *array_size = map.size();

    return reactionRates;
}

String get_species_property_string(MICM *micm, const char *species_name, const char *property_name)
{
    std::string species_name_str(species_name);
    std::string property_name_str(property_name);
    const std::string value_str = micm->get_species_property<std::string>(species_name_str, property_name_str);
    String value;
    value.size_ = value_str.length();
    value.value_ = new char[value.size_ + 1];
    std::strcpy(value.value_, value_str.c_str());

    return value;
}

double get_species_property_double(MICM *micm, const char *species_name, const char *property_name)
{
    std::string species_name_str(species_name);
    std::string property_name_str(property_name);
    return micm->get_species_property<double>(species_name_str, property_name_str);
}

int get_species_property_int(MICM *micm, const char *species_name, const char *property_name)
{
    std::string species_name_str(species_name);
    std::string property_name_str(property_name);
    return micm->get_species_property<int>(species_name_str, property_name_str);
}

bool get_species_property_bool(MICM *micm, const char *species_name, const char *property_name)
{
    std::string species_name_str(species_name);
    std::string property_name_str(property_name);
    return micm->get_species_property<bool>(species_name_str, property_name_str);
}

int MICM::create_solver(const std::string &config_path)
{
    int parsing_status = 0; // 0 on success, 1 on failure
    try {
        micm::SolverConfig<> solver_config;
        micm::ConfigParseStatus status = solver_config.ReadAndParse(std::filesystem::path(config_path));

        if (status == micm::ConfigParseStatus::Success)
        {
            solver_parameters_ = std::make_unique<micm::SolverParameters>(solver_config.GetSolverParams());
            auto params = micm::RosenbrockSolverParameters::three_stage_rosenbrock_parameters(NUM_GRID_CELLS);
            params.ignore_unused_species_ = true;
            solver_ = std::make_unique<micm::RosenbrockSolver<>>(solver_parameters_->system_,
                                                                solver_parameters_->processes_,
                                                                params);
        }
        else
        {
            parsing_status = 1;
        }
    }
    catch(std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        parsing_status = 1;
    }


    return parsing_status;
}

void MICM::solve(double time_step, double temperature, double pressure, int num_concentrations, double *concentrations, int num_custom_rate_parameters, double *custom_rate_parameters)
{
    micm::State state = solver_->GetState();

    for (size_t i{}; i < NUM_GRID_CELLS; i++)
    {
        state.conditions_[i].temperature_ = temperature;
        state.conditions_[i].pressure_ = pressure;
    }

    state.variables_.AsVector().assign(concentrations, concentrations + num_concentrations);

    state.custom_rate_parameters_.AsVector().assign(custom_rate_parameters, custom_rate_parameters + num_custom_rate_parameters);

    auto result = solver_->Solve<false>(time_step, state);

    for (int i = 0; i < result.result_.AsVector().size(); i++)
    {
        concentrations[i] = result.result_.AsVector()[i];
    }
}

std::map<std::string, size_t> MICM::get_species_ordering()
{
    micm::State state = solver_->GetState();
    return state.variable_map_;
}

std::map<std::string, size_t> MICM::get_user_defined_reaction_rates_ordering()
{
    micm::State state = solver_->GetState();
    return state.custom_rate_parameter_map_;
}
