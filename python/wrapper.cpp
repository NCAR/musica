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
   .value("rosenbrock_standard_order", musica::MICMSolver::RosenbrockStandardOrder)
   .value("backward_euler", musica::MICMSolver::BackwardEuler)
   .value("backward_euler_standard_order", musica::MICMSolver::BackwardEulerStandardOrder);

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
         py::object temperature,
         py::object pressure,
         py::object air_density,
         py::list concentrations,
         py::object custom_rate_parameters = py::none())
      {
        std::vector<double> temperature_cpp;
        if (py::isinstance<py::float_>(temperature))
        {
          temperature_cpp.push_back(temperature.cast<double>());
        }
        else if(py::isinstance<py::list>(temperature))
        {
          py::list temperature_list = temperature.cast<py::list>();
          temperature_cpp.reserve(len(temperature_list));
          for (auto item : temperature_list)
          {
            temperature_cpp.push_back(item.cast<double>());
          }
        }
        else
        {
          throw std::runtime_error("Temperature must be a list or a double. Got " + std::string(py::str(temperature.get_type()).cast<std::string>()));
        }

        std::vector<double> pressure_cpp;
        if (py::isinstance<py::float_>(pressure))
        {
          pressure_cpp.push_back(pressure.cast<double>());
        }
        else if(py::isinstance<py::list>(pressure))
        {
          py::list pressure_list = pressure.cast<py::list>();
          pressure_cpp.reserve(len(pressure_list));
          for (auto item : pressure_list)
          {
            pressure_cpp.push_back(item.cast<double>());
          }
        }
        else
        {
          throw std::runtime_error("Pressure must be a list or a double. Got " + std::string(py::str(pressure.get_type()).cast<std::string>()));
        }
        std::vector<double> air_density_cpp;
        if (py::isinstance<py::float_>(air_density))
        {
          air_density_cpp.push_back(air_density.cast<double>());
        }
        else if(py::isinstance<py::list>(air_density))
        {
          py::list air_density_list = air_density.cast<py::list>();
          air_density_cpp.reserve(len(air_density_list));
          for (auto item : air_density_list)
          {
            air_density_cpp.push_back(item.cast<double>());
          }
        }
        else
        {
          throw std::runtime_error("Air density must be a list or a double. Got " + std::string(py::str(air_density.get_type()).cast<std::string>()));
        }
        
        std::vector<double> concentrations_cpp;
        concentrations_cpp.reserve(len(concentrations));
        for (auto item : concentrations)
        {
          concentrations_cpp.push_back(item.cast<double>());
        }

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
        musica::MicmSolve(
            micm,
            time_step,
            temperature_cpp.data(),
            pressure_cpp.data(),
            air_density_cpp.data(),
            concentrations_cpp.data(),
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
        else if (micm->solver_type_ == musica::MICMSolver::BackwardEuler)
        {
          map = micm->GetSpeciesOrdering(micm->backward_euler_, &error);
        }
        else if (micm->solver_type_ == musica::MICMSolver::BackwardEulerStandardOrder)
        {
          map = micm->GetSpeciesOrdering(micm->backward_euler_standard_, &error);
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
        else if (micm->solver_type_ == musica::MICMSolver::BackwardEuler)
        {
          map = micm->GetUserDefinedReactionRatesOrdering(micm->backward_euler_, &error);
        }
        else if (micm->solver_type_ == musica::MICMSolver::BackwardEulerStandardOrder)
        {
          map = micm->GetUserDefinedReactionRatesOrdering(micm->backward_euler_standard_, &error);
        }

        return map;
      },
      "Return map of reaction rates");
}
