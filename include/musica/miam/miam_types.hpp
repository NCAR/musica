// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Configuration types for MIAM aerosol model integration.
// These structs carry MIAM configuration from the Python/pybind11
// layer to the C++ builder that constructs miam::Model objects.
#pragma once

#include <functional>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace musica
{
  namespace miam_config
  {
    // ── Rate constants ──────────────────────────────────────────────────

    /// @brief Henry's law constant: HLC(T) = HLC_ref * exp(C * (1/T - 1/298.15))
    struct HenrysLawConstant
    {
      double hlc_ref;   ///< Reference HLC at 298.15 K [mol m-3 Pa-1]
      double c = 0.0;   ///< Temperature dependence factor [K]
    };

    /// @brief Equilibrium constant: K(T) = A * exp(C * (1/298.15 - 1/T))
    struct EquilibriumConstant
    {
      double a;          ///< Pre-exponential factor [units vary]
      double c = 0.0;    ///< Temperature dependence factor [K]
    };

    /// @brief Arrhenius rate constant: k(T) = A * exp(-C * (1/T - 1/298.15))
    struct ArrheniusRateConstant
    {
      double a;          ///< Pre-exponential factor [units vary]
      double c = 0.0;    ///< Ea/R activation parameter [K]
    };

    /// @brief Rate constant: either Arrhenius or a callback(temperature_K) -> rate
    using RateConstant = std::variant<ArrheniusRateConstant, std::function<double(double)>>;

    // ── Representations ─────────────────────────────────────────────────

    struct UniformSection
    {
      std::string name;
      std::vector<std::string> phase_names;
      double min_radius;   ///< Minimum section radius [m]
      double max_radius;   ///< Maximum section radius [m]
    };

    struct SingleMomentMode
    {
      std::string name;
      std::vector<std::string> phase_names;
      double geometric_mean_radius;          ///< Geometric mean radius [m]
      double geometric_standard_deviation;   ///< Geometric standard deviation [-]
    };

    struct TwoMomentMode
    {
      std::string name;
      std::vector<std::string> phase_names;
      double geometric_standard_deviation;   ///< Geometric standard deviation [-]
    };

    using Representation = std::variant<UniformSection, SingleMomentMode, TwoMomentMode>;

    // ── Processes ───────────────────────────────────────────────────────

    struct DissolvedReaction
    {
      std::string phase_name;
      std::vector<std::string> reactant_names;
      std::vector<std::string> product_names;
      std::string solvent_name;
      RateConstant rate_constant;
      double solvent_damping_epsilon = 1.0e-20;  ///< Regularization parameter to prevent singularity as solvent → 0
      double min_halflife = 0.0;  ///< When > 0, caps reaction rate so no reactant is depleted faster than this half-life [s]
    };

    struct DissolvedReversibleReaction
    {
      std::string phase_name;
      std::vector<std::string> reactant_names;
      std::vector<std::string> product_names;
      std::string solvent_name;
      std::optional<RateConstant> forward_rate_constant;
      std::optional<RateConstant> reverse_rate_constant;
      std::optional<EquilibriumConstant> equilibrium_constant;
      double solvent_damping_epsilon = 1.0e-20;  ///< Regularization parameter to prevent singularity as solvent → 0
    };

    struct HenryLawPhaseTransfer
    {
      std::string condensed_phase_name;
      std::string gas_species_name;
      std::string condensed_species_name;
      std::string solvent_name;
      HenrysLawConstant henrys_law_constant;
      double diffusion_coefficient;         ///< Gas-phase diffusion coefficient [m2 s-1]
      double accommodation_coefficient;     ///< Mass accommodation coefficient [-]
    };

    using Process = std::variant<DissolvedReaction, DissolvedReversibleReaction, HenryLawPhaseTransfer>;

    // ── Constraints ─────────────────────────────────────────────────────

    struct HenryLawEquilibriumConstraint
    {
      std::string gas_species_name;
      std::string condensed_species_name;
      std::string solvent_name;
      std::string condensed_phase_name;
      HenrysLawConstant henrys_law_constant;
      double mw_solvent;      ///< Solvent molecular weight [kg mol-1]
      double rho_solvent;     ///< Solvent density [kg m-3]
    };

    struct DissolvedEquilibriumConstraint
    {
      std::string phase_name;
      std::vector<std::string> reactant_names;
      std::vector<std::string> product_names;
      std::string algebraic_species_name;
      std::string solvent_name;
      EquilibriumConstant equilibrium_constant;
      double solvent_damping_epsilon = 1.0e-20;  ///< Regularization parameter to prevent singularity as solvent → 0
    };

    struct LinearConstraintTerm
    {
      std::string phase_name;
      std::string species_name;
      double coefficient;
    };

    struct LinearConstraint
    {
      std::string algebraic_phase_name;
      std::string algebraic_species_name;
      std::vector<LinearConstraintTerm> terms;
      double constant;
      bool diagnose_from_state = false;
    };

    using Constraint = std::variant<HenryLawEquilibriumConstraint, DissolvedEquilibriumConstraint, LinearConstraint>;

    // ── Model ───────────────────────────────────────────────────────────

    /// @brief Species definition for MIAM (carries properties needed by MIAM processes)
    struct SpeciesDef
    {
      std::string name;
      std::optional<double> molecular_weight;   ///< [kg mol-1]
      std::optional<double> density;            ///< [kg m-3]
    };

    /// @brief Phase definition for MIAM
    struct PhaseDef
    {
      std::string name;
      std::vector<std::string> species_names;
    };

    /// @brief Complete MIAM model configuration
    struct ModelConfig
    {
      std::string name;
      std::vector<SpeciesDef> species;
      std::vector<PhaseDef> gas_phases;
      std::vector<PhaseDef> condensed_phases;
      std::vector<Representation> representations;
      std::vector<Process> processes;
      std::vector<Constraint> constraints;
    };

  }  // namespace miam_config
}  // namespace musica
