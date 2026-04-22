// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Builder that converts musica::miam_config structs into a miam::Model
// and creates a MICM solver with it attached as an external model.

#include <musica/miam/miam_builder.hpp>
#include <musica/miam/miam_types.hpp>
#include <musica/micm/cpu_solver.hpp>
#include <musica/micm/micm.hpp>
#include <musica/util.hpp>

#include <miam/miam.hpp>
#include <miam/constraints/dissolved_equilibrium_constraint_builder.hpp>
#include <miam/constraints/henry_law_equilibrium_constraint_builder.hpp>
#include <miam/constraints/linear_constraint_builder.hpp>
#include <miam/processes/constants/equilibrium_constant.hpp>
#include <miam/processes/constants/henrys_law_constant.hpp>
#include <miam/processes/dissolved_reaction_builder.hpp>
#include <miam/processes/dissolved_reversible_reaction_builder.hpp>
#include <miam/processes/henry_law_phase_transfer_builder.hpp>

#include <micm/CPU.hpp>
#include <micm/solver/backward_euler_solver_parameters.hpp>
#include <micm/solver/rosenbrock_solver_parameters.hpp>

#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_map>

namespace musica
{
  namespace
  {
    // ── Convert config SpeciesDef to micm::Species ──────────────────

    micm::Species ConvertSpecies(const miam_config::SpeciesDef& def)
    {
      micm::Species s{ def.name };
      if (def.molecular_weight.has_value())
        s.SetProperty("molecular weight [kg mol-1]", def.molecular_weight.value());
      if (def.density.has_value())
        s.SetProperty("density [kg m-3]", def.density.value());
      return s;
    }

    // ── Resolve a rate-constant config to std::function ─────────────

    /// Wrapper that provides a Calculate() method for template-based builders
    struct RateConstantWrapper
    {
      std::function<double(const micm::Conditions&)> fn;
      double Calculate(const micm::Conditions& conditions) const
      {
        return fn(conditions);
      }
    };

    RateConstantWrapper ResolveRateConstant(const miam_config::RateConstant& rc)
    {
      return { std::visit(
          [](const auto& val) -> std::function<double(const micm::Conditions&)>
          {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, miam_config::ArrheniusRateConstant>)
            {
              double a = val.a;
              double c = val.c;
              constexpr double T0 = 298.15;
              return [a, c, T0](const micm::Conditions& cond) -> double
              { return a * std::exp(-c * (1.0 / cond.temperature_ - 1.0 / T0)); };
            }
            else  // std::function<double(double)>
            {
              auto callback = val;
              return [callback](const micm::Conditions& cond) -> double { return callback(cond.temperature_); };
            }
          },
          rc) };
    }

    // ── Build the miam::Model from config ───────────────────────────

    miam::Model BuildMiamModel(
        const miam_config::ModelConfig& config,
        const std::unordered_map<std::string, micm::Species>& species_map,
        const std::unordered_map<std::string, micm::Phase>& phase_map)
    {
      // Lambdas for safe lookup
      auto find_species = [&](const std::string& name) -> micm::Species
      {
        auto it = species_map.find(name);
        if (it == species_map.end())
          throw std::runtime_error("MIAM: Unknown species '" + name + "'");
        return it->second;
      };

      auto find_species_vec = [&](const std::vector<std::string>& names) -> std::vector<micm::Species>
      {
        std::vector<micm::Species> result;
        result.reserve(names.size());
        for (const auto& n : names)
          result.push_back(find_species(n));
        return result;
      };

      auto find_phase = [&](const std::string& name) -> micm::Phase
      {
        auto it = phase_map.find(name);
        if (it == phase_map.end())
          throw std::runtime_error("MIAM: Unknown phase '" + name + "'");
        return it->second;
      };

      auto resolve_phases = [&](const std::vector<std::string>& names) -> std::vector<micm::Phase>
      {
        std::vector<micm::Phase> result;
        result.reserve(names.size());
        for (const auto& n : names)
          result.push_back(find_phase(n));
        return result;
      };

      // ── Representations ──
      std::vector<miam::Model::RepresentationVariant> representations;
      for (const auto& repr_cfg : config.representations)
      {
        std::visit(
            [&](const auto& r)
            {
              using T = std::decay_t<decltype(r)>;
              if constexpr (std::is_same_v<T, miam_config::UniformSection>)
              {
                representations.emplace_back(
                    miam::representation::UniformSection(r.name, resolve_phases(r.phase_names), r.min_radius, r.max_radius));
              }
              else if constexpr (std::is_same_v<T, miam_config::SingleMomentMode>)
              {
                representations.emplace_back(miam::representation::SingleMomentMode(
                    r.name, resolve_phases(r.phase_names), r.geometric_mean_radius, r.geometric_standard_deviation));
              }
              else if constexpr (std::is_same_v<T, miam_config::TwoMomentMode>)
              {
                representations.emplace_back(
                    miam::representation::TwoMomentMode(r.name, resolve_phases(r.phase_names), r.geometric_standard_deviation));
              }
            },
            repr_cfg);
      }

      miam::Model model{ .name_ = config.name, .representations_ = std::move(representations) };

      // ── Processes ──
      for (const auto& proc_cfg : config.processes)
      {
        std::visit(
            [&](const auto& p)
            {
              using T = std::decay_t<decltype(p)>;
              if constexpr (std::is_same_v<T, miam_config::DissolvedReaction>)
              {
                model.AddProcesses(miam::process::DissolvedReactionBuilder()
                                       .SetPhase(find_phase(p.phase_name))
                                       .SetReactants(find_species_vec(p.reactant_names))
                                       .SetProducts(find_species_vec(p.product_names))
                                       .SetSolvent(find_species(p.solvent_name))
                                       .SetRateConstant(ResolveRateConstant(p.rate_constant))
                                       .SetSolventDampingEpsilon(p.solvent_damping_epsilon)
                                       .SetMinHalflife(p.min_halflife)
                                       .Build());
              }
              else if constexpr (std::is_same_v<T, miam_config::DissolvedReversibleReaction>)
              {
                auto builder = miam::process::DissolvedReversibleReactionBuilder()
                                   .SetPhase(find_phase(p.phase_name))
                                   .SetReactants(find_species_vec(p.reactant_names))
                                   .SetProducts(find_species_vec(p.product_names))
                                   .SetSolvent(find_species(p.solvent_name))
                                   .SetSolventDampingEpsilon(p.solvent_damping_epsilon);
                if (p.forward_rate_constant.has_value())
                  builder.SetForwardRateConstant(ResolveRateConstant(p.forward_rate_constant.value()));
                if (p.reverse_rate_constant.has_value())
                  builder.SetReverseRateConstant(ResolveRateConstant(p.reverse_rate_constant.value()));
                if (p.equilibrium_constant.has_value())
                {
                  const auto& ec = p.equilibrium_constant.value();
                  builder.SetEquilibriumConstant(
                      miam::process::constant::EquilibriumConstant({ .A_ = ec.a, .C_ = ec.c }));
                }
                model.AddProcesses(builder.Build());
              }
              else if constexpr (std::is_same_v<T, miam_config::HenryLawPhaseTransfer>)
              {
                model.AddProcesses(miam::process::HenryLawPhaseTransferBuilder()
                                       .SetCondensedPhase(find_phase(p.condensed_phase_name))
                                       .SetGasSpecies(find_species(p.gas_species_name))
                                       .SetCondensedSpecies(find_species(p.condensed_species_name))
                                       .SetSolvent(find_species(p.solvent_name))
                                       .SetHenrysLawConstant(miam::process::constant::HenrysLawConstant(
                                           { .HLC_ref_ = p.henrys_law_constant.hlc_ref, .C_ = p.henrys_law_constant.c }))
                                       .SetDiffusionCoefficient(p.diffusion_coefficient)
                                       .SetAccommodationCoefficient(p.accommodation_coefficient)
                                       .Build());
              }
            },
            proc_cfg);
      }

      // ── Constraints ──
      for (const auto& con_cfg : config.constraints)
      {
        std::visit(
            [&](const auto& c)
            {
              using T = std::decay_t<decltype(c)>;
              if constexpr (std::is_same_v<T, miam_config::HenryLawEquilibriumConstraint>)
              {
                model.AddConstraints(miam::constraint::HenryLawEquilibriumConstraintBuilder()
                                         .SetGasSpecies(find_species(c.gas_species_name))
                                         .SetCondensedSpecies(find_species(c.condensed_species_name))
                                         .SetSolvent(find_species(c.solvent_name))
                                         .SetCondensedPhase(find_phase(c.condensed_phase_name))
                                         .SetHenryLawConstant(miam::process::constant::HenrysLawConstant(
                                             { .HLC_ref_ = c.henrys_law_constant.hlc_ref, .C_ = c.henrys_law_constant.c }))
                                         .SetMwSolvent(c.mw_solvent)
                                         .SetRhoSolvent(c.rho_solvent)
                                         .Build());
              }
              else if constexpr (std::is_same_v<T, miam_config::DissolvedEquilibriumConstraint>)
              {
                model.AddConstraints(miam::constraint::DissolvedEquilibriumConstraintBuilder()
                                         .SetPhase(find_phase(c.phase_name))
                                         .SetReactants(find_species_vec(c.reactant_names))
                                         .SetProducts(find_species_vec(c.product_names))
                                         .SetAlgebraicSpecies(find_species(c.algebraic_species_name))
                                         .SetSolvent(find_species(c.solvent_name))
                                         .SetEquilibriumConstant(miam::process::constant::EquilibriumConstant(
                                             { .A_ = c.equilibrium_constant.a, .C_ = c.equilibrium_constant.c }))
                                         .SetSolventDampingEpsilon(c.solvent_damping_epsilon)
                                         .Build());
              }
              else if constexpr (std::is_same_v<T, miam_config::LinearConstraint>)
              {
                auto builder = miam::constraint::LinearConstraintBuilder().SetAlgebraicSpecies(
                    find_phase(c.algebraic_phase_name), find_species(c.algebraic_species_name));
                for (const auto& term : c.terms)
                  builder.AddTerm(find_phase(term.phase_name), find_species(term.species_name), term.coefficient);
                if (c.diagnose_from_state)
                  builder.DiagnoseConstantFromState();
                else
                  builder.SetConstant(c.constant);
                model.AddConstraints(builder.Build());
              }
            },
            con_cfg);
      }

      return model;
    }

    // ── Create the MICM solver with MIAM as external model ──────────

    CpuSolver::SolverVariant BuildSolverVariant(
        const micm::System& system,
        const std::vector<micm::Process>& processes,
        MICMSolver solver_type,
        miam::Model& miam_model)
    {
      auto configure = [&](auto builder)
      {
        return builder.SetSystem(system)
            .SetReactions(processes)
            .SetIgnoreUnusedSpecies(true)
            .AddExternalModel(miam_model)
            .Build();
      };

      switch (solver_type)
      {
        case MICMSolver::Rosenbrock:
          return std::make_unique<micm::Rosenbrock>(configure(
              micm::RosenbrockThreeStageBuilder(micm::RosenbrockSolverParameters::ThreeStageRosenbrockParameters())));

        case MICMSolver::RosenbrockStandardOrder:
          return std::make_unique<micm::RosenbrockStandard>(configure(
              micm::CpuSolverBuilder<micm::RosenbrockSolverParameters>(
                  micm::RosenbrockSolverParameters::ThreeStageRosenbrockParameters())));

        case MICMSolver::BackwardEuler:
          return std::make_unique<micm::BackwardEuler>(
              configure(micm::BackwardEulerBuilder(micm::BackwardEulerSolverParameters())));

        case MICMSolver::BackwardEulerStandardOrder:
          return std::make_unique<micm::BackwardEulerStandard>(configure(
              micm::CpuSolverBuilder<micm::BackwardEulerSolverParameters>(micm::BackwardEulerSolverParameters())));

        case MICMSolver::RosenbrockDAE4:
          return std::make_unique<micm::Rosenbrock>(configure(micm::RosenbrockThreeStageBuilder(
              micm::RosenbrockSolverParameters::FourStageDifferentialAlgebraicRosenbrockParameters())));

        case MICMSolver::RosenbrockDAE4StandardOrder:
          return std::make_unique<micm::RosenbrockStandard>(configure(
              micm::CpuSolverBuilder<micm::RosenbrockSolverParameters>(
                  micm::RosenbrockSolverParameters::FourStageDifferentialAlgebraicRosenbrockParameters())));

        case MICMSolver::RosenbrockDAE6:
          return std::make_unique<micm::Rosenbrock>(configure(micm::RosenbrockThreeStageBuilder(
              micm::RosenbrockSolverParameters::SixStageDifferentialAlgebraicRosenbrockParameters())));

        case MICMSolver::RosenbrockDAE6StandardOrder:
          return std::make_unique<micm::RosenbrockStandard>(configure(
              micm::CpuSolverBuilder<micm::RosenbrockSolverParameters>(
                  micm::RosenbrockSolverParameters::SixStageDifferentialAlgebraicRosenbrockParameters())));

        default:
          throw std::system_error(
              make_error_code(MusicaErrCode::SolverTypeNotFound),
              "Solver type " + ToString(solver_type) + " not supported for MIAM");
      }
    }

  }  // anonymous namespace

  MICM* CreateMicmWithMiam(
      const Chemistry& chemistry,
      MICMSolver solver_type,
      const miam_config::ModelConfig& miam_config,
      Error* error)
  {
    DeleteError(error);
    try
    {
      // Build species map from MIAM config
      std::unordered_map<std::string, micm::Species> species_map;
      for (const auto& sd : miam_config.species)
        species_map[sd.name] = ConvertSpecies(sd);

      // Build phase map from MIAM condensed phases
      std::unordered_map<std::string, micm::Phase> phase_map;
      for (const auto& pd : miam_config.condensed_phases)
      {
        std::vector<micm::PhaseSpecies> phase_species;
        for (const auto& sn : pd.species_names)
        {
          auto it = species_map.find(sn);
          if (it == species_map.end())
            throw std::runtime_error("MIAM: Species '" + sn + "' in phase '" + pd.name + "' not found");
          phase_species.push_back(micm::PhaseSpecies(it->second));
        }
        phase_map[pd.name] = micm::Phase(pd.name, phase_species);
      }

      // Include the gas phase from the mechanism so linear constraints can reference it
      phase_map[chemistry.system.gas_phase_.name_] = chemistry.system.gas_phase_;

      // Build the miam::Model
      auto miam_model = BuildMiamModel(miam_config, species_map, phase_map);

      // Create system: mechanism's gas phase + MIAM model
      auto system = micm::System(chemistry.system.gas_phase_, miam_model);

      // Build solver with MIAM as external model
      auto solver_variant = BuildSolverVariant(system, chemistry.processes, solver_type, miam_model);

      // Wrap in CpuSolver -> SolverPtr -> MICM
      auto default_deleter = [](IMicmSolver* ptr) { delete ptr; };
      SolverPtr solver_ptr(
          new CpuSolver(std::move(solver_variant), static_cast<int>(solver_type)),
          default_deleter);

      return new MICM(std::move(solver_ptr), solver_type);
    }
    catch (const std::exception& e)
    {
      ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_UNKNOWN, e.what(), MUSICA_SEVERITY_ERR, error);
      return nullptr;
    }
  }

}  // namespace musica
