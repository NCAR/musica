// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#include "jlcxx/jlcxx.hpp"
#include "jlcxx/stl.hpp"

#include "musica/version.hpp"
#include "musica/micm/micm.hpp"
#include "musica/micm/micm_c_interface.hpp"
#include "musica/micm/state.hpp"
#include "musica/micm/state_c_interface.hpp"
#include "musica/micm/solver_parameters.hpp"
#include "musica/micm/cuda_availability.hpp"

#include <micm/solver/solver_result.hpp>
#include <micm/version.hpp>

#include <iostream>
#include <string>
#include <vector>

JLCXX_MODULE define_julia_module(jlcxx::Module& mod)
{
  // ── Version ──────────────────────────────────────────────────────────
  mod.method("get_version", []() { return std::string(musica::GetMusicaVersion()); });
  mod.method("get_micm_version", []() { return std::string(micm::GetMicmVersion()); });

  // ── MICMSolver enum constants ────────────────────────────────────────
  mod.set_const("SOLVER_ROSENBROCK", static_cast<int>(musica::MICMSolver::Rosenbrock));
  mod.set_const("SOLVER_ROSENBROCK_STANDARD_ORDER", static_cast<int>(musica::MICMSolver::RosenbrockStandardOrder));
  mod.set_const("SOLVER_BACKWARD_EULER", static_cast<int>(musica::MICMSolver::BackwardEuler));
  mod.set_const("SOLVER_BACKWARD_EULER_STANDARD_ORDER", static_cast<int>(musica::MICMSolver::BackwardEulerStandardOrder));
  mod.set_const("SOLVER_CUDA_ROSENBROCK", static_cast<int>(musica::MICMSolver::CudaRosenbrock));

  // ── SolverState enum constants ───────────────────────────────────────
  mod.set_const("SOLVER_STATE_NOT_YET_CALLED", static_cast<int>(micm::SolverState::NotYetCalled));
  mod.set_const("SOLVER_STATE_RUNNING", static_cast<int>(micm::SolverState::Running));
  mod.set_const("SOLVER_STATE_CONVERGED", static_cast<int>(micm::SolverState::Converged));
  mod.set_const("SOLVER_STATE_CONVERGENCE_EXCEEDED_MAX_STEPS", static_cast<int>(micm::SolverState::ConvergenceExceededMaxSteps));
  mod.set_const("SOLVER_STATE_STEP_SIZE_TOO_SMALL", static_cast<int>(micm::SolverState::StepSizeTooSmall));
  mod.set_const("SOLVER_STATE_REPEATEDLY_SINGULAR_MATRIX", static_cast<int>(micm::SolverState::RepeatedlySingularMatrix));
  mod.set_const("SOLVER_STATE_NAN_DETECTED", static_cast<int>(micm::SolverState::NaNDetected));
  mod.set_const("SOLVER_STATE_INF_DETECTED", static_cast<int>(micm::SolverState::InfDetected));
  mod.set_const("SOLVER_STATE_ACCEPTING_UNCONVERGED", static_cast<int>(micm::SolverState::AcceptingUnconvergedIntegration));

  // ── Conditions type ──────────────────────────────────────────────────
  mod.add_type<micm::Conditions>("CppConditions")
      .constructor<>()
      .method("get_temperature", [](const micm::Conditions& c) { return c.temperature_; })
      .method("set_temperature!", [](micm::Conditions& c, double t) { c.temperature_ = t; })
      .method("get_pressure", [](const micm::Conditions& c) { return c.pressure_; })
      .method("set_pressure!", [](micm::Conditions& c, double p) { c.pressure_ = p; })
      .method("get_air_density", [](const micm::Conditions& c) { return c.air_density_; })
      .method("set_air_density!", [](micm::Conditions& c, double d) { c.air_density_ = d; });

  // ── SolverResultStats type ───────────────────────────────────────────
  mod.add_type<musica::SolverResultStats>("CppSolverStats")
      .method("get_function_calls", [](const musica::SolverResultStats& s) { return static_cast<int64_t>(s.function_calls_); })
      .method("get_jacobian_updates", [](const musica::SolverResultStats& s) { return static_cast<int64_t>(s.jacobian_updates_); })
      .method("get_number_of_steps", [](const musica::SolverResultStats& s) { return static_cast<int64_t>(s.number_of_steps_); })
      .method("get_accepted", [](const musica::SolverResultStats& s) { return static_cast<int64_t>(s.accepted_); })
      .method("get_rejected", [](const musica::SolverResultStats& s) { return static_cast<int64_t>(s.rejected_); })
      .method("get_decompositions", [](const musica::SolverResultStats& s) { return static_cast<int64_t>(s.decompositions_); })
      .method("get_solves", [](const musica::SolverResultStats& s) { return static_cast<int64_t>(s.solves_); })
      .method("get_final_time", [](const musica::SolverResultStats& s) { return s.final_time_; });

  // ── SolverResult type ────────────────────────────────────────────────
  mod.add_type<micm::SolverResult>("CppSolverResult")
      .method("get_solver_state", [](const micm::SolverResult& r) { return static_cast<int>(r.state_); })
      .method("get_solver_stats", [](const micm::SolverResult& r) { return r.stats_; });

  // ── RosenbrockSolverParameters type ──────────────────────────────────
  mod.add_type<musica::RosenbrockSolverParameters>("CppRosenbrockParams")
      .constructor<>()
      .method("get_relative_tolerance", [](const musica::RosenbrockSolverParameters& p) { return p.relative_tolerance; })
      .method("set_relative_tolerance!", [](musica::RosenbrockSolverParameters& p, double v) { p.relative_tolerance = v; })
      .method("get_h_min", [](const musica::RosenbrockSolverParameters& p) { return p.h_min; })
      .method("set_h_min!", [](musica::RosenbrockSolverParameters& p, double v) { p.h_min = v; })
      .method("get_h_max", [](const musica::RosenbrockSolverParameters& p) { return p.h_max; })
      .method("set_h_max!", [](musica::RosenbrockSolverParameters& p, double v) { p.h_max = v; })
      .method("get_h_start", [](const musica::RosenbrockSolverParameters& p) { return p.h_start; })
      .method("set_h_start!", [](musica::RosenbrockSolverParameters& p, double v) { p.h_start = v; })
      .method("get_max_number_of_steps", [](const musica::RosenbrockSolverParameters& p) { return static_cast<int64_t>(p.max_number_of_steps); })
      .method("set_max_number_of_steps!", [](musica::RosenbrockSolverParameters& p, int64_t v) { p.max_number_of_steps = static_cast<std::size_t>(v); })
      .method("get_absolute_tolerances", [](const musica::RosenbrockSolverParameters& p) {
        return std::vector<double>(p.absolute_tolerances.begin(), p.absolute_tolerances.end());
      })
      .method("set_absolute_tolerances!", [](musica::RosenbrockSolverParameters& p, jlcxx::ArrayRef<double> tols) {
        p.absolute_tolerances = std::vector<double>(tols.begin(), tols.end());
      });

  // ── BackwardEulerSolverParameters type ───────────────────────────────
  mod.add_type<musica::BackwardEulerSolverParameters>("CppBackwardEulerParams")
      .constructor<>()
      .method("get_relative_tolerance", [](const musica::BackwardEulerSolverParameters& p) { return p.relative_tolerance; })
      .method("set_relative_tolerance!", [](musica::BackwardEulerSolverParameters& p, double v) { p.relative_tolerance = v; })
      .method("get_max_number_of_steps", [](const musica::BackwardEulerSolverParameters& p) { return static_cast<int64_t>(p.max_number_of_steps); })
      .method("set_max_number_of_steps!", [](musica::BackwardEulerSolverParameters& p, int64_t v) { p.max_number_of_steps = static_cast<std::size_t>(v); })
      .method("get_absolute_tolerances", [](const musica::BackwardEulerSolverParameters& p) {
        return std::vector<double>(p.absolute_tolerances.begin(), p.absolute_tolerances.end());
      })
      .method("set_absolute_tolerances!", [](musica::BackwardEulerSolverParameters& p, jlcxx::ArrayRef<double> tols) {
        p.absolute_tolerances = std::vector<double>(tols.begin(), tols.end());
      })
      .method("get_time_step_reductions", [](const musica::BackwardEulerSolverParameters& p) {
        return std::vector<double>(p.time_step_reductions.begin(), p.time_step_reductions.end());
      })
      .method("set_time_step_reductions!", [](musica::BackwardEulerSolverParameters& p, jlcxx::ArrayRef<double> vals) {
        p.time_step_reductions = std::vector<double>(vals.begin(), vals.end());
      });

  // ── MICM opaque type ────────────────────────────────────────────────
  mod.add_type<musica::MICM>("CppMICM");

  // ── State opaque type ───────────────────────────────────────────────
  mod.add_type<musica::State>("CppState");

  // ── MICM creation / deletion ────────────────────────────────────────
  mod.method("cpp_create_solver", [](const std::string& config_path, int solver_type) {
    musica::Error error;
    musica::MICM* micm = musica::CreateMicm(
        config_path.c_str(),
        static_cast<musica::MICMSolver>(solver_type),
        &error);
    if (!musica::IsSuccess(error))
    {
      std::string msg = "Error creating solver: " + std::string(error.message_.value_);
      musica::DeleteError(&error);
      throw std::runtime_error(msg);
    }
    return micm;
  });

  mod.method("cpp_delete_solver", [](musica::MICM* micm) {
    if (!micm) return;
    musica::Error error;
    musica::DeleteMicm(micm, &error);
    if (!musica::IsSuccess(error))
    {
      std::cerr << "Error deleting MICM: " << error.message_.value_ << std::endl;
      musica::DeleteError(&error);
    }
  });

  // ── State creation / deletion ───────────────────────────────────────
  mod.method("cpp_create_state", [](musica::MICM* micm, int64_t number_of_grid_cells) {
    musica::Error error;
    musica::State* state = musica::CreateMicmState(
        micm,
        static_cast<std::size_t>(number_of_grid_cells),
        &error);
    if (!musica::IsSuccess(error))
    {
      std::string msg = "Error creating state: " + std::string(error.message_.value_);
      musica::DeleteError(&error);
      throw std::runtime_error(msg);
    }
    return state;
  });

  mod.method("cpp_delete_state", [](musica::State* state) {
    if (!state) return;
    musica::Error error;
    musica::DeleteState(state, &error);
    if (!musica::IsSuccess(error))
    {
      std::cerr << "Error deleting State: " << error.message_.value_ << std::endl;
      musica::DeleteError(&error);
    }
  });

  // ── Solve ───────────────────────────────────────────────────────────
  mod.method("cpp_micm_solve", [](musica::MICM* micm, musica::State* state, double time_step) {
    return micm->Solve(state, time_step);
  });

  // ── Vector size ─────────────────────────────────────────────────────
  mod.method("cpp_get_vector_size", [](int solver_type) {
    return static_cast<int64_t>(musica::GetVectorSize(static_cast<musica::MICMSolver>(solver_type)));
  });

  // ── State accessors ─────────────────────────────────────────────────

  // Number of grid cells
  mod.method("cpp_state_num_grid_cells", [](musica::State* state) {
    return static_cast<int64_t>(state->NumberOfGridCells());
  });

  // Concentrations: get size, get element, set element (0-based index from Julia side)
  mod.method("cpp_state_concentrations_size", [](musica::State* state) {
    return static_cast<int64_t>(state->GetOrderedConcentrations().size());
  });

  mod.method("cpp_state_get_concentration", [](musica::State* state, int64_t idx) {
    return state->GetOrderedConcentrations()[static_cast<std::size_t>(idx)];
  });

  mod.method("cpp_state_set_concentration!", [](musica::State* state, int64_t idx, double value) {
    state->GetOrderedConcentrations()[static_cast<std::size_t>(idx)] = value;
  });

  // Rate parameters: get size, get element, set element (0-based index from Julia side)
  mod.method("cpp_state_rate_params_size", [](musica::State* state) {
    return static_cast<int64_t>(state->GetOrderedRateParameters().size());
  });

  mod.method("cpp_state_get_rate_param", [](musica::State* state, int64_t idx) {
    return state->GetOrderedRateParameters()[static_cast<std::size_t>(idx)];
  });

  mod.method("cpp_state_set_rate_param!", [](musica::State* state, int64_t idx, double value) {
    state->GetOrderedRateParameters()[static_cast<std::size_t>(idx)] = value;
  });

  // Conditions: get number, get/set per grid cell (0-based index)
  mod.method("cpp_state_get_condition_temperature", [](musica::State* state, int64_t idx) {
    return state->GetConditions()[static_cast<std::size_t>(idx)].temperature_;
  });

  mod.method("cpp_state_get_condition_pressure", [](musica::State* state, int64_t idx) {
    return state->GetConditions()[static_cast<std::size_t>(idx)].pressure_;
  });

  mod.method("cpp_state_get_condition_air_density", [](musica::State* state, int64_t idx) {
    return state->GetConditions()[static_cast<std::size_t>(idx)].air_density_;
  });

  mod.method("cpp_state_set_condition!", [](musica::State* state, int64_t idx, double temp, double pres, double dens) {
    auto& conditions = state->GetConditions();
    auto i = static_cast<std::size_t>(idx);
    conditions[i].temperature_ = temp;
    conditions[i].pressure_ = pres;
    conditions[i].air_density_ = dens;
  });

  // Species ordering: return parallel arrays of names and indices
  mod.method("cpp_state_species_ordering_names", [](musica::State* state) {
    auto map = state->GetVariableMap();
    std::vector<std::string> names;
    names.reserve(map.size());
    for (const auto& [name, _] : map)
      names.push_back(name);
    return names;
  });

  mod.method("cpp_state_species_ordering_indices", [](musica::State* state) {
    auto map = state->GetVariableMap();
    std::vector<int64_t> indices;
    indices.reserve(map.size());
    for (const auto& [_, idx] : map)
      indices.push_back(static_cast<int64_t>(idx));
    return indices;
  });

  // Rate parameter ordering: return parallel arrays of names and indices
  mod.method("cpp_state_rate_param_ordering_names", [](musica::State* state) {
    auto map = state->GetRateParameterMap();
    std::vector<std::string> names;
    names.reserve(map.size());
    for (const auto& [name, _] : map)
      names.push_back(name);
    return names;
  });

  mod.method("cpp_state_rate_param_ordering_indices", [](musica::State* state) {
    auto map = state->GetRateParameterMap();
    std::vector<int64_t> indices;
    indices.reserve(map.size());
    for (const auto& [_, idx] : map)
      indices.push_back(static_cast<int64_t>(idx));
    return indices;
  });

  // ── Solver parameters ───────────────────────────────────────────────
  mod.method("cpp_set_rosenbrock_params", [](musica::MICM* micm, const musica::RosenbrockSolverParameters& params) {
    micm->SetSolverParameters(params);
  });

  mod.method("cpp_set_backward_euler_params", [](musica::MICM* micm, const musica::BackwardEulerSolverParameters& params) {
    micm->SetSolverParameters(params);
  });

  mod.method("cpp_get_rosenbrock_params", [](musica::MICM* micm) {
    return micm->GetRosenbrockSolverParameters();
  });

  mod.method("cpp_get_backward_euler_params", [](musica::MICM* micm) {
    return micm->GetBackwardEulerSolverParameters();
  });

  // ── CUDA availability ───────────────────────────────────────────────
  mod.method("cpp_is_cuda_available", []() {
    return musica::IsCudaAvailable();
  });
}
