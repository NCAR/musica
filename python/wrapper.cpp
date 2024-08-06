// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/micm.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

// Wraps micm.cpp
PYBIND11_MODULE(musica, m)
{
  py::class_<musica::MICM>(m, "micm")
      .def(py::init<>())
      .def("__del__", [](musica::MICM &micm) {});

  py::enum_<musica::MICMSolver>(m, "micmsolver")
   .value("rosenbrock", musica::MICMSolver::Rosenbrock)
   .value("rosenbrock_standard_order", musica::MICMSolver::RosenbrockStandardOrder);

  m.def(
      "create_solver",
      [](const char *config_path, musica::MICMSolver solver_type, int num_grid_cells)
      {
        musica::Error error;
        musica::MICM *micm = musica::CreateMicm(config_path, solver_type, num_grid_cells, &error);
        if (!musica::IsSuccess(error))
        {
          std::string message = "Error creating solver: " + std::string(error.message_.value_);
          DeleteError(&error);
          throw std::runtime_error(message);
        }
        return micm;
      });

  m.def("delete_micm", &musica::DeleteMicm);

  m.def(
      "micm_solve",
      [](musica::MICM *micm,
         double time_step,
         double temperature,
         double pressure,
         double air_density,
         py::list concentrations,
         py::object custom_rate_parameters = py::none())
      {
        std::cout << "Setting concentrations" << std::endl;
        std::vector<double> concentrations_cpp;
        concentrations_cpp.reserve(len(concentrations));
        for (auto item : concentrations)
        {
          concentrations_cpp.push_back(item.cast<double>());
        }

        std::cout << "Setting custom rate parameters" << std::endl;
        std::vector<double> custom_rate_parameters_cpp;
        if (!custom_rate_parameters.is_none())
        {
          py::list parameters = custom_rate_parameters.cast<py::list>();
          custom_rate_parameters_cpp.reserve(len(parameters));
          for (auto item : parameters)
          {
            custom_rate_parameters_cpp.push_back(item.cast<double>());
          }
        }
        musica::String solver_state;
        musica::SolverResultStats solver_stats;
        musica::Error error;
        std::cout << "Solving system" << std::endl;
        musica::MicmSolve(
            micm,
            time_step,
            temperature,
            pressure,
            air_density,
            concentrations_cpp.size(),
            concentrations_cpp.data(),
            custom_rate_parameters_cpp.size(),
            custom_rate_parameters_cpp.data(),
            &solver_state,
            &solver_stats,
            &error);
        if (!musica::IsSuccess(error))
        {
          std::string message = "Error solving system: " + std::string(error.message_.value_);
          DeleteError(&error);
          throw std::runtime_error(message);
        }

        std::cout << "Updating concentrations" << std::endl;
        // Update the concentrations list after solving
        for (std::size_t i = 0; i < concentrations_cpp.size(); ++i)
        {
          concentrations[i] = concentrations_cpp[i];
        }
      },
      "Solve the system");

  m.def(
      "species_ordering",
      [](musica::MICM *micm)
      {
        musica::Error error;
        std::map<std::string, std::size_t> map;

        if (micm->solver_type_ == musica::MICMSolver::Rosenbrock)
        {
          map = micm->GetSpeciesOrdering(micm->rosenbrock_, &error);
        }
        else if (micm->solver_type_ == musica::MICMSolver::RosenbrockStandardOrder)
        {
          map = micm->GetSpeciesOrdering(micm->rosenbrock_standard_, &error);
        }

        return map;
      },
      "Return map of get_species_ordering rates");

  m.def(
      "user_defined_reaction_rates",
      [](musica::MICM *micm)
      {
        musica::Error error;
        std::map<std::string, std::size_t> map;

        if (micm->solver_type_ == musica::MICMSolver::Rosenbrock)
        {
          map = micm->GetUserDefinedReactionRatesOrdering(micm->rosenbrock_, &error);
        }
        else if (micm->solver_type_ == musica::MICMSolver::RosenbrockStandardOrder)
        {
          map = micm->GetUserDefinedReactionRatesOrdering(micm->rosenbrock_standard_, &error);
        }

        return map;
      },
      "Return map of reaction rates");
}