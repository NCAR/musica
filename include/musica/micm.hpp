/**
 * This file contains the defintion of the MICM class, which represents a multi-component
 * reactive transport model. It also includes functions for creating and deleting MICM instances with c binding
 * Copyright (C) 2023-2024 National Center for Atmospheric Research,
 *
 * SPDX-License-Identifier: Apache-2.0* creating solvers, and solving the model.
 */
#pragma once

#include <micm/configure/solver_config.hpp>
#include <micm/solver/rosenbrock.hpp>

#include <memory>
#include <string>
#include <vector>

#include <musica/util.hpp>

namespace musica {

class MICM;

#ifdef __cplusplus
extern "C"
{
#endif

    MICM *create_micm(const char *config_path, int *error_code);
    void delete_micm(const MICM *micm);
    void micm_solve(MICM *micm, double time_step, double temperature, double pressure, int num_concentrations, double *concentrations, int num_custom_rate_parameters, double *custom_rate_parameters);
    Mapping *get_species_ordering(MICM *micm, size_t *array_size);
    Mapping *get_user_defined_reaction_rates_ordering(MICM *micm, size_t *array_size);
    String get_species_property_string(MICM *micm, const char *species_name, const char *property_name);
    double get_species_property_double(MICM *micm, const char *species_name, const char *property_name);
    int get_species_property_int(MICM *micm, const char *species_name, const char *property_name);
    bool get_species_property_bool(MICM *micm, const char *species_name, const char *property_name);

#ifdef __cplusplus
}
#endif

class MICM
{
public:
    /// @brief Create a solver by reading and parsing configuration file
    /// @param config_path Path to configuration file or directory containing configuration file
    /// @return 0 on success, 1 on failure in parsing configuration file
    int create(const std::string &config_path);

    /// @brief Solve the system
    /// @param time_step Time [s] to advance the state by
    /// @para/ @briefm temperature Temperature [K]
    /// @param pressure Pressure/ [P@brief C@param num_concentrations The number oconfiguration file
    /// @param config_path Path to configuration file or directory containing configuration file
    /// @return 0 on success, 1 on failure in parsing species' concentrations
    /// @param concentrations Species's concentrations
    void solve(double time_step, double temperature, double pressure, int num_concentrations, double *concentrations, int num_custom_rate_parameters, double *custom_rate_parameters);

    /// @brief Get a property for a chemical species
    /// @param species_name Name of the species
    /// @param property_name Name of the property
    /// @return Value of the property
    template<class T>
    T get_species_property(const std::string &species_name, const std::string &property_name);
    
    /// @brief Get the ordering of species
    /// @return Map of species names to their indices
    std::map<std::string, size_t> get_species_ordering();

    /// @brief Get the ordering of user-defined reaction rates
    /// @return Map of reaction rate names to their indices
    std::map<std::string, size_t> get_user_defined_reaction_rates_ordering();

    static constexpr size_t NUM_GRID_CELLS = 1;

private:
    std::unique_ptr<micm::RosenbrockSolver<>> solver_;
    std::unique_ptr<micm::SolverParameters> solver_parameters_;
};

template<class T>
inline T MICM::get_species_property(const std::string &species_name, const std::string &property_name)
{
    for (const auto &species : solver_parameters_->system_.gas_phase_.species_)
    {
        if (species.name_ == species_name)
        {
            try {
                return species.GetProperty<T>(property_name);
            }
            catch (const std::exception &e) {
                throw std::runtime_error(std::string(e.what()) + " for species '" + species_name + "'");
            }
        }
    }
   throw std::runtime_error("Species '" + species_name + "' not found");
}

}