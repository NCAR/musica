// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Builder that converts a mechanism_configuration Mechanism (species, phases,
// and aerosol section) into a miam::Model and creates a MICM solver with it
// attached as an external model.

#include <musica/configuration/parse.hpp>
#include <musica/miam/miam_builder.hpp>
#include <musica/micm/cpu_solver.hpp>
#include <musica/micm/micm.hpp>
#include <musica/utils/error_code.hpp>

#include <mechanism_configuration/mechanism_configuration.hpp>
#include <micm/CPU.hpp>
#include <micm/process/rate_constant/rate_constant_functions.hpp>
#include <micm/solver/backward_euler_solver_parameters.hpp>
#include <micm/solver/rosenbrock_solver_parameters.hpp>

#include <miam/miam.hpp>

#include <stdexcept>
#include <string>
#include <unordered_map>

namespace musica
{
  namespace
  {
    namespace types = mechanism_configuration::types;

    // ── Convert a mechanism_configuration Species to a micm::Species ─

    micm::Species ConvertSpecies(const types::Species& def)
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

    RateConstantWrapper ResolveRateConstant(const types::RateConstant& rc)
    {
      return { std::visit(
          [](const auto& val) -> std::function<double(const micm::Conditions&)>
          {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, types::Equilibrium>)
            {
              miam::EquilibriumConstant k({ .A_ = val.A, .C_ = val.C, .T0_ = val.T0 });
              return [k](const micm::Conditions& cond) -> double { return k.Calculate(cond); };
            }
            else if constexpr (std::is_same_v<T, types::Arrhenius>)
            {
              // Full gas-phase Arrhenius, honoring the temperature and pressure terms.
              // Field mapping mirrors ConvertChemistry (1:1 A/B/C/D/E) and the lambda
              // matches DissolvedReactionBuilder::AddRateConstant, so aerosol and gas
              // Arrhenius rate constants evaluate identically.
              micm::ArrheniusRateConstantParameters params;
              params.A_ = val.A;
              params.B_ = val.B;
              params.C_ = val.C;
              params.D_ = val.D;
              params.E_ = val.E;
              return [params](const micm::Conditions& cond) -> double
              { return micm::CalculateArrhenius(params, cond.temperature_, cond.pressure_); };
            }
            else  // std::function<double(double)>
            {
              auto callback = val;
              return [callback](const micm::Conditions& cond) -> double { return callback(cond.temperature_); };
            }
          },
          rc) };
    }

    // ── Build the miam::Model from the mechanism's aerosol section ──

    miam::Model BuildMiamModel(
        const std::string& name,
        const types::Aerosol& aerosol,
        const std::unordered_map<std::string, micm::Species>& species_map,
        const std::unordered_map<std::string, micm::Phase>& phase_map)
    {
      // Lambdas for safe lookup (throw on an unknown name so typos surface here).
      auto find_species = [&](const std::string& name) -> micm::Species
      {
        auto it = species_map.find(name);
        if (it == species_map.end())
          throw std::runtime_error("MIAM: Unknown species '" + name + "'");
        return it->second;
      };

      // Resolve a list of ReactionComponents to micm::Species (coefficients are not
      // carried into the MIAM builders today; only the referenced species are used).
      auto find_species_components =
          [&](const std::vector<types::ReactionComponent>& components) -> std::vector<micm::Species>
      {
        std::vector<micm::Species> result;
        result.reserve(components.size());
        for (const auto& c : components)
          result.push_back(find_species(c.name));
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
      for (const auto& repr_cfg : aerosol.representations)
      {
        std::visit(
            [&](const auto& r)
            {
              using T = std::decay_t<decltype(r)>;
              if constexpr (std::is_same_v<T, types::UniformSection>)
              {
                representations.emplace_back(
                    miam::UniformSection(r.name, resolve_phases(r.phases), r.min_radius, r.max_radius));
              }
              else if constexpr (std::is_same_v<T, types::SingleMomentMode>)
              {
                representations.emplace_back(miam::SingleMomentMode(
                    r.name, resolve_phases(r.phases), r.geometric_mean_radius, r.geometric_standard_deviation));
              }
              else if constexpr (std::is_same_v<T, types::TwoMomentMode>)
              {
                representations.emplace_back(
                    miam::TwoMomentMode(r.name, resolve_phases(r.phases), r.geometric_standard_deviation));
              }
            },
            repr_cfg);
      }

      miam::Model model{ .name_ = name, .representations_ = std::move(representations) };

      // ── Processes ──
      for (const auto& proc_cfg : aerosol.processes)
      {
        std::visit(
            [&](const auto& p)
            {
              using T = std::decay_t<decltype(p)>;
              if constexpr (std::is_same_v<T, types::DissolvedReaction>)
              {
                auto builder = miam::DissolvedReactionBuilder()
                                   .SetPhase(find_phase(p.phase))
                                   .SetReactants(find_species_components(p.reactants))
                                   .SetProducts(find_species_components(p.products))
                                   .SetSolvent(find_species(p.solvent));
                if (!p.rate_constants.empty())
                  builder.SetRateConstant(ResolveRateConstant(p.rate_constants.begin()->second).fn);
                if (p.solvent_floor_.has_value())
                  builder.SetSolventFloor(p.solvent_floor_.value());
                if (p.min_halflife_.has_value())
                  builder.SetMinHalflife(p.min_halflife_.value());
                model.AddProcesses(builder.Build());
              }
              else if constexpr (std::is_same_v<T, types::DissolvedReversibleReaction>)
              {
                auto builder = miam::DissolvedReversibleReactionBuilder()
                                   .SetPhase(find_phase(p.phase))
                                   .SetReactants(find_species_components(p.reactants))
                                   .SetProducts(find_species_components(p.products))
                                   .SetSolvent(find_species(p.solvent));
                // Exactly two of {forward, reverse, equilibrium} must be given; the
                // builder derives the third.
                if (!p.forward_rate_constants.empty())
                  builder.SetForwardRateConstant(ResolveRateConstant(p.forward_rate_constants.begin()->second));
                if (!p.reverse_rate_constants.empty())
                  builder.SetReverseRateConstant(ResolveRateConstant(p.reverse_rate_constants.begin()->second));
                if (p.equilibrium_constant.has_value())
                {
                  const auto& ec = p.equilibrium_constant.value();
                  builder.SetEquilibriumConstant(miam::EquilibriumConstant({ .A_ = ec.A, .C_ = ec.C, .T0_ = ec.T0 }));
                }
                if (p.solvent_floor_.has_value())
                  builder.SetSolventFloor(p.solvent_floor_.value());
                model.AddProcesses(builder.Build());
              }
              else if constexpr (std::is_same_v<T, types::HenryLawPhaseTransfer>)
              {
                model.AddProcesses(
                    miam::HenryLawPhaseTransferBuilder()
                        .SetCondensedPhase(find_phase(p.condensed_phase))
                        .SetGasSpecies(find_species(p.gas_species))
                        .SetCondensedSpecies(find_species(p.condensed_species))
                        .SetSolvent(find_species(p.solvent))
                        .SetHenryLawConstant(miam::HenryLawConstant({ .HLC_ref_ = p.henry_law_constant.HLC_ref,
                                                                      .C_ = p.henry_law_constant.C,
                                                                      .T0_ = p.henry_law_constant.T0 }))
                        .SetDiffusionCoefficient(p.diffusion_coefficient)
                        .SetAccommodationCoefficient(p.accommodation_coefficient)
                        .Build());
              }
            },
            proc_cfg);
      }

      // ── Constraints ──
      for (const auto& con_cfg : aerosol.constraints)
      {
        std::visit(
            [&](const auto& c)
            {
              using T = std::decay_t<decltype(c)>;
              if constexpr (std::is_same_v<T, types::HenryLawEquilibrium>)
              {
                // The MIAM builder reads the solvent's molecular weight and density
                // from the species properties at Build(); apply the constraint's
                // explicit solvent_molecular_weight / solvent_density (when set) as
                // overrides on the solvent passed to it.
                micm::Species solvent = find_species(c.solvent);
                if (c.solvent_molecular_weight > 0.0)
                  solvent.SetProperty("molecular weight [kg mol-1]", c.solvent_molecular_weight);
                if (c.solvent_density > 0.0)
                  solvent.SetProperty("density [kg m-3]", c.solvent_density);
                model.AddConstraints(
                    miam::HenryLawEquilibriumConstraintBuilder()
                        .SetGasSpecies(find_species(c.gas_species))
                        .SetCondensedSpecies(find_species(c.condensed_species))
                        .SetSolvent(solvent)
                        .SetCondensedPhase(find_phase(c.condensed_phase))
                        .SetHenryLawConstant(miam::HenryLawConstant({ .HLC_ref_ = c.henry_law_constant.HLC_ref,
                                                                      .C_ = c.henry_law_constant.C,
                                                                      .T0_ = c.henry_law_constant.T0 }))
                        .Build());
              }
              else if constexpr (std::is_same_v<T, types::DissolvedEquilibrium>)
              {
                auto builder = miam::DissolvedEquilibriumConstraintBuilder()
                                   .SetPhase(find_phase(c.phase))
                                   .SetReactants(find_species_components(c.reactants))
                                   .SetProducts(find_species_components(c.products))
                                   .SetAlgebraicSpecies(find_species(c.algebraic_species))
                                   .SetSolvent(find_species(c.solvent))
                                   .SetEquilibriumConstant(miam::EquilibriumConstant({ .A_ = c.equilibrium_constant.A,
                                                                                       .C_ = c.equilibrium_constant.C,
                                                                                       .T0_ = c.equilibrium_constant.T0 }));
                if (c.solvent_floor_.has_value())
                  builder.SetSolventFloor(c.solvent_floor_.value());
                model.AddConstraints(builder.Build());
              }
              else if constexpr (std::is_same_v<T, types::LinearConstraint>)
              {
                auto builder = miam::LinearConstraintBuilder().SetAlgebraicSpecies(
                    find_phase(c.algebraic_phase), find_species(c.algebraic_species));
                for (const auto& term : c.terms)
                  builder.AddTerm(find_phase(term.phase), find_species(term.name), term.coefficient);
                // `constant` selects one of the two mutually exclusive modes.
                std::visit(
                    [&](const auto& k)
                    {
                      using K = std::decay_t<decltype(k)>;
                      if constexpr (std::is_same_v<K, types::FixedConstant>)
                        builder.SetConstant(k.value);
                      else  // types::DiagnoseFromState
                        builder.DiagnoseConstantFromState();
                    },
                    c.constant);
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
          return std::make_unique<micm::RosenbrockStandard>(
              configure(micm::CpuSolverBuilder<micm::RosenbrockSolverParameters>(
                  micm::RosenbrockSolverParameters::ThreeStageRosenbrockParameters())));

        case MICMSolver::BackwardEuler:
          return std::make_unique<micm::BackwardEuler>(
              configure(micm::BackwardEulerBuilder(micm::BackwardEulerSolverParameters())));

        case MICMSolver::BackwardEulerStandardOrder:
          return std::make_unique<micm::BackwardEulerStandard>(
              configure(micm::CpuSolverBuilder<micm::BackwardEulerSolverParameters>(micm::BackwardEulerSolverParameters())));

        case MICMSolver::RosenbrockDAE4:
          return std::make_unique<micm::Rosenbrock>(configure(micm::RosenbrockThreeStageBuilder(
              micm::RosenbrockSolverParameters::FourStageDifferentialAlgebraicRosenbrockParameters())));

        case MICMSolver::RosenbrockDAE4StandardOrder:
          return std::make_unique<micm::RosenbrockStandard>(
              configure(micm::CpuSolverBuilder<micm::RosenbrockSolverParameters>(
                  micm::RosenbrockSolverParameters::FourStageDifferentialAlgebraicRosenbrockParameters())));

        case MICMSolver::RosenbrockDAE6:
          return std::make_unique<micm::Rosenbrock>(configure(micm::RosenbrockThreeStageBuilder(
              micm::RosenbrockSolverParameters::SixStageDifferentialAlgebraicRosenbrockParameters())));

        case MICMSolver::RosenbrockDAE6StandardOrder:
          return std::make_unique<micm::RosenbrockStandard>(
              configure(micm::CpuSolverBuilder<micm::RosenbrockSolverParameters>(
                  micm::RosenbrockSolverParameters::SixStageDifferentialAlgebraicRosenbrockParameters())));

        default:
          throw musica::Exception(
              musica::MicmErrorCode::SolverTypeNotFound, "Solver type " + ToString(solver_type) + " not supported for MIAM");
      }
    }

  }  // anonymous namespace

  MICM* CreateMicmWithMiam(const mechanism_configuration::Mechanism& mechanism, MICMSolver solver_type, Error* error)
  {
    DeleteError(error);
    try
    {
      if (!mechanism.aerosol.has_value())
        throw std::runtime_error("MIAM: mechanism has no aerosol section");

      // Resolve aerosol name references against the mechanism's species and phases.
      auto validation_errors = mechanism_configuration::ValidateAerosolModel(mechanism);
      if (!validation_errors.empty())
      {
        std::string message = "MIAM: invalid aerosol configuration:";
        for (const auto& [code, msg] : validation_errors)
          message += "\n  - " + msg;
        throw std::runtime_error(message);
      }
      // Build the species map from the mechanism's species list.
      std::unordered_map<std::string, micm::Species> species_map;
      for (const auto& sp : mechanism.species)
        species_map[sp.name] = ConvertSpecies(sp);

      // Build the phase map from the mechanism's phases (gas + condensed).
      std::unordered_map<std::string, micm::Phase> phase_map;
      for (const auto& ph : mechanism.phases)
      {
        std::vector<micm::PhaseSpecies> phase_species;
        for (const auto& ps : ph.species)
        {
          auto it = species_map.find(ps.name);
          if (it == species_map.end())
            throw std::runtime_error("MIAM: Species '" + ps.name + "' in phase '" + ph.name + "' not found");
          // Carry the phase-specific properties (diffusion coefficient, density) from
          // the config's PhaseSpecies onto the micm::PhaseSpecies MIAM reads.
          micm::PhaseSpecies phase_sp(it->second);
          if (ps.diffusion_coefficient.has_value())
            phase_sp.SetDiffusionCoefficient(ps.diffusion_coefficient.value());
          if (ps.density.has_value())
            phase_sp.SetDensity(ps.density.value());
          phase_species.push_back(phase_sp);
        }
        phase_map[ph.name] = micm::Phase(ph.name, phase_species);
      }

      // Gas-phase chemistry is derived from the mechanism (single source of truth).
      const Chemistry chemistry = ConvertChemistry(mechanism);

      // Overlay the mechanism's gas phase from the built chemistry so linear
      // constraints referencing it get the exact micm gas-phase object.
      phase_map[chemistry.system.gas_phase_.name_] = chemistry.system.gas_phase_;

      // Build the miam::Model
      auto miam_model = BuildMiamModel(mechanism.name, mechanism.aerosol.value(), species_map, phase_map);

      // Build solver with MIAM as external model
      auto solver_variant = BuildSolverVariant(chemistry.system.gas_phase_, chemistry.processes, solver_type, miam_model);

      // Wrap in CpuSolver -> SolverPtr -> MICM
      auto default_deleter = [](IMicmSolver* ptr) { delete ptr; };
      SolverPtr solver_ptr(new CpuSolver(std::move(solver_variant), static_cast<int>(solver_type)), default_deleter);

      return new MICM(std::move(solver_ptr), solver_type);
    }
    catch (const std::exception& e)
    {
      ToError(e, MUSICA_SEVERITY_ERROR, error);
      return nullptr;
    }
  }

}  // namespace musica
