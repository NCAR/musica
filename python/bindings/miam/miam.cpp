// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Pybind11 bindings for MIAM aerosol/cloud model configuration types
// and solver creation with MIAM as external model.

#include "../common.hpp"

#include <musica/miam/miam_builder.hpp>
#include <musica/miam/miam_types.hpp>
#include <musica/micm/micm.hpp>
#include <musica/micm/micm_c_interface.hpp>
#include <musica/micm/parse.hpp>
#include <musica/util.hpp>

#include <mechanism_configuration/v1/types.hpp>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

namespace py = pybind11;
namespace v1 = mechanism_configuration::v1::types;
namespace mc = musica::miam_config;

void bind_miam(py::module_& miam)
{
  // ── Constants ─────────────────────────────────────────────────────

  py::class_<mc::HenrysLawConstant>(miam, "_HenrysLawConstant")
      .def(py::init<double, double>(), py::arg("hlc_ref"), py::arg("c") = 0.0)
      .def_readwrite("hlc_ref", &mc::HenrysLawConstant::hlc_ref)
      .def_readwrite("c", &mc::HenrysLawConstant::c);

  py::class_<mc::EquilibriumConstant>(miam, "_EquilibriumConstant")
      .def(py::init<double, double>(), py::arg("a"), py::arg("c") = 0.0)
      .def_readwrite("a", &mc::EquilibriumConstant::a)
      .def_readwrite("c", &mc::EquilibriumConstant::c);

  py::class_<mc::ArrheniusRateConstant>(miam, "_ArrheniusRateConstant")
      .def(py::init<double, double>(), py::arg("a"), py::arg("c") = 0.0)
      .def_readwrite("a", &mc::ArrheniusRateConstant::a)
      .def_readwrite("c", &mc::ArrheniusRateConstant::c);

  // ── Species / Phase definitions ───────────────────────────────────

  py::class_<mc::SpeciesDef>(miam, "_SpeciesDef")
      .def(
          py::init(
              [](const std::string& name, std::optional<double> molecular_weight, std::optional<double> density)
              {
                return mc::SpeciesDef{ name, molecular_weight, density };
              }),
          py::arg("name"),
          py::arg("molecular_weight") = py::none(),
          py::arg("density") = py::none())
      .def_readwrite("name", &mc::SpeciesDef::name)
      .def_readwrite("molecular_weight", &mc::SpeciesDef::molecular_weight)
      .def_readwrite("density", &mc::SpeciesDef::density);

  py::class_<mc::PhaseDef>(miam, "_PhaseDef")
      .def(py::init<std::string, std::vector<std::string>>(), py::arg("name"), py::arg("species_names"))
      .def_readwrite("name", &mc::PhaseDef::name)
      .def_readwrite("species_names", &mc::PhaseDef::species_names);

  // ── Representations ───────────────────────────────────────────────

  py::class_<mc::UniformSection>(miam, "_UniformSection")
      .def(
          py::init<std::string, std::vector<std::string>, double, double>(),
          py::arg("name"),
          py::arg("phase_names"),
          py::arg("min_radius"),
          py::arg("max_radius"))
      .def_readwrite("name", &mc::UniformSection::name)
      .def_readwrite("phase_names", &mc::UniformSection::phase_names)
      .def_readwrite("min_radius", &mc::UniformSection::min_radius)
      .def_readwrite("max_radius", &mc::UniformSection::max_radius);

  py::class_<mc::SingleMomentMode>(miam, "_SingleMomentMode")
      .def(
          py::init<std::string, std::vector<std::string>, double, double>(),
          py::arg("name"),
          py::arg("phase_names"),
          py::arg("geometric_mean_radius"),
          py::arg("geometric_standard_deviation"))
      .def_readwrite("name", &mc::SingleMomentMode::name)
      .def_readwrite("phase_names", &mc::SingleMomentMode::phase_names)
      .def_readwrite("geometric_mean_radius", &mc::SingleMomentMode::geometric_mean_radius)
      .def_readwrite("geometric_standard_deviation", &mc::SingleMomentMode::geometric_standard_deviation);

  py::class_<mc::TwoMomentMode>(miam, "_TwoMomentMode")
      .def(
          py::init<std::string, std::vector<std::string>, double>(),
          py::arg("name"),
          py::arg("phase_names"),
          py::arg("geometric_standard_deviation"))
      .def_readwrite("name", &mc::TwoMomentMode::name)
      .def_readwrite("phase_names", &mc::TwoMomentMode::phase_names)
      .def_readwrite("geometric_standard_deviation", &mc::TwoMomentMode::geometric_standard_deviation);

  // ── Processes ─────────────────────────────────────────────────────

  py::class_<mc::DissolvedReaction>(miam, "_DissolvedReaction")
      .def(
          py::init(
              [](const std::string& phase_name,
                 const std::vector<std::string>& reactant_names,
                 const std::vector<std::string>& product_names,
                 const std::string& solvent_name,
                 py::object rate_constant)
              {
                mc::RateConstant rc;
                if (py::isinstance<mc::ArrheniusRateConstant>(rate_constant))
                  rc = rate_constant.cast<mc::ArrheniusRateConstant>();
                else
                  rc = rate_constant.cast<std::function<double(double)>>();
                return mc::DissolvedReaction{ phase_name, reactant_names, product_names, solvent_name, std::move(rc) };
              }),
          py::arg("phase_name"),
          py::arg("reactant_names"),
          py::arg("product_names"),
          py::arg("solvent_name"),
          py::arg("rate_constant"))
      .def_readwrite("phase_name", &mc::DissolvedReaction::phase_name)
      .def_readwrite("reactant_names", &mc::DissolvedReaction::reactant_names)
      .def_readwrite("product_names", &mc::DissolvedReaction::product_names)
      .def_readwrite("solvent_name", &mc::DissolvedReaction::solvent_name);

  py::class_<mc::DissolvedReversibleReaction>(miam, "_DissolvedReversibleReaction")
      .def(
          py::init(
              [](const std::string& phase_name,
                 const std::vector<std::string>& reactant_names,
                 const std::vector<std::string>& product_names,
                 const std::string& solvent_name,
                 py::object forward_rate_constant,
                 py::object reverse_rate_constant,
                 py::object equilibrium_constant)
              {
                mc::DissolvedReversibleReaction rxn;
                rxn.phase_name = phase_name;
                rxn.reactant_names = reactant_names;
                rxn.product_names = product_names;
                rxn.solvent_name = solvent_name;
                if (!forward_rate_constant.is_none())
                {
                  if (py::isinstance<mc::ArrheniusRateConstant>(forward_rate_constant))
                    rxn.forward_rate_constant = forward_rate_constant.cast<mc::ArrheniusRateConstant>();
                  else
                    rxn.forward_rate_constant = forward_rate_constant.cast<std::function<double(double)>>();
                }
                if (!reverse_rate_constant.is_none())
                {
                  if (py::isinstance<mc::ArrheniusRateConstant>(reverse_rate_constant))
                    rxn.reverse_rate_constant = reverse_rate_constant.cast<mc::ArrheniusRateConstant>();
                  else
                    rxn.reverse_rate_constant = reverse_rate_constant.cast<std::function<double(double)>>();
                }
                if (!equilibrium_constant.is_none())
                  rxn.equilibrium_constant = equilibrium_constant.cast<mc::EquilibriumConstant>();
                return rxn;
              }),
          py::arg("phase_name"),
          py::arg("reactant_names"),
          py::arg("product_names"),
          py::arg("solvent_name"),
          py::arg("forward_rate_constant") = py::none(),
          py::arg("reverse_rate_constant") = py::none(),
          py::arg("equilibrium_constant") = py::none())
      .def_readwrite("phase_name", &mc::DissolvedReversibleReaction::phase_name)
      .def_readwrite("reactant_names", &mc::DissolvedReversibleReaction::reactant_names)
      .def_readwrite("product_names", &mc::DissolvedReversibleReaction::product_names)
      .def_readwrite("solvent_name", &mc::DissolvedReversibleReaction::solvent_name);

  py::class_<mc::HenryLawPhaseTransfer>(miam, "_HenryLawPhaseTransfer")
      .def(
          py::init<std::string, std::string, std::string, std::string, mc::HenrysLawConstant, double, double>(),
          py::arg("condensed_phase_name"),
          py::arg("gas_species_name"),
          py::arg("condensed_species_name"),
          py::arg("solvent_name"),
          py::arg("henrys_law_constant"),
          py::arg("diffusion_coefficient"),
          py::arg("accommodation_coefficient"))
      .def_readwrite("condensed_phase_name", &mc::HenryLawPhaseTransfer::condensed_phase_name)
      .def_readwrite("gas_species_name", &mc::HenryLawPhaseTransfer::gas_species_name)
      .def_readwrite("condensed_species_name", &mc::HenryLawPhaseTransfer::condensed_species_name)
      .def_readwrite("solvent_name", &mc::HenryLawPhaseTransfer::solvent_name)
      .def_readwrite("henrys_law_constant", &mc::HenryLawPhaseTransfer::henrys_law_constant)
      .def_readwrite("diffusion_coefficient", &mc::HenryLawPhaseTransfer::diffusion_coefficient)
      .def_readwrite("accommodation_coefficient", &mc::HenryLawPhaseTransfer::accommodation_coefficient);

  // ── Constraints ───────────────────────────────────────────────────

  py::class_<mc::HenryLawEquilibriumConstraint>(miam, "_HenryLawEquilibriumConstraint")
      .def(
          py::init<std::string, std::string, std::string, std::string, mc::HenrysLawConstant, double, double>(),
          py::arg("gas_species_name"),
          py::arg("condensed_species_name"),
          py::arg("solvent_name"),
          py::arg("condensed_phase_name"),
          py::arg("henrys_law_constant"),
          py::arg("mw_solvent"),
          py::arg("rho_solvent"))
      .def_readwrite("gas_species_name", &mc::HenryLawEquilibriumConstraint::gas_species_name)
      .def_readwrite("condensed_species_name", &mc::HenryLawEquilibriumConstraint::condensed_species_name)
      .def_readwrite("solvent_name", &mc::HenryLawEquilibriumConstraint::solvent_name)
      .def_readwrite("condensed_phase_name", &mc::HenryLawEquilibriumConstraint::condensed_phase_name)
      .def_readwrite("henrys_law_constant", &mc::HenryLawEquilibriumConstraint::henrys_law_constant)
      .def_readwrite("mw_solvent", &mc::HenryLawEquilibriumConstraint::mw_solvent)
      .def_readwrite("rho_solvent", &mc::HenryLawEquilibriumConstraint::rho_solvent);

  py::class_<mc::DissolvedEquilibriumConstraint>(miam, "_DissolvedEquilibriumConstraint")
      .def(
          py::init<std::string, std::vector<std::string>, std::vector<std::string>, std::string, std::string, mc::EquilibriumConstant>(),
          py::arg("phase_name"),
          py::arg("reactant_names"),
          py::arg("product_names"),
          py::arg("algebraic_species_name"),
          py::arg("solvent_name"),
          py::arg("equilibrium_constant"))
      .def_readwrite("phase_name", &mc::DissolvedEquilibriumConstraint::phase_name)
      .def_readwrite("reactant_names", &mc::DissolvedEquilibriumConstraint::reactant_names)
      .def_readwrite("product_names", &mc::DissolvedEquilibriumConstraint::product_names)
      .def_readwrite("algebraic_species_name", &mc::DissolvedEquilibriumConstraint::algebraic_species_name)
      .def_readwrite("solvent_name", &mc::DissolvedEquilibriumConstraint::solvent_name)
      .def_readwrite("equilibrium_constant", &mc::DissolvedEquilibriumConstraint::equilibrium_constant);

  py::class_<mc::LinearConstraintTerm>(miam, "_LinearConstraintTerm")
      .def(
          py::init<std::string, std::string, double>(),
          py::arg("phase_name"),
          py::arg("species_name"),
          py::arg("coefficient"))
      .def_readwrite("phase_name", &mc::LinearConstraintTerm::phase_name)
      .def_readwrite("species_name", &mc::LinearConstraintTerm::species_name)
      .def_readwrite("coefficient", &mc::LinearConstraintTerm::coefficient);

  py::class_<mc::LinearConstraint>(miam, "_LinearConstraint")
      .def(
          py::init<std::string, std::string, std::vector<mc::LinearConstraintTerm>, double>(),
          py::arg("algebraic_phase_name"),
          py::arg("algebraic_species_name"),
          py::arg("terms"),
          py::arg("constant") = 0.0)
      .def_readwrite("algebraic_phase_name", &mc::LinearConstraint::algebraic_phase_name)
      .def_readwrite("algebraic_species_name", &mc::LinearConstraint::algebraic_species_name)
      .def_readwrite("terms", &mc::LinearConstraint::terms)
      .def_readwrite("constant", &mc::LinearConstraint::constant);

  // ── ModelConfig ───────────────────────────────────────────────────

  py::class_<mc::ModelConfig>(miam, "_ModelConfig")
      .def(py::init<>())
      .def_readwrite("name", &mc::ModelConfig::name)
      .def_readwrite("species", &mc::ModelConfig::species)
      .def_readwrite("condensed_phases", &mc::ModelConfig::condensed_phases)
      .def_readwrite("representations", &mc::ModelConfig::representations)
      .def_readwrite("processes", &mc::ModelConfig::processes)
      .def_readwrite("constraints", &mc::ModelConfig::constraints);

  // ── Solver creation ───────────────────────────────────────────────

  miam.def(
      "_create_solver_with_miam",
      [](const v1::Mechanism& mechanism, musica::MICMSolver solver_type, const mc::ModelConfig& miam_config)
      {
        musica::Error error;
        musica::Chemistry chemistry = musica::ConvertV1Mechanism(mechanism);
        musica::MICM* micm = musica::CreateMicmWithMiam(chemistry, solver_type, miam_config, &error);
        if (!musica::IsSuccess(error))
        {
          std::string message = "Error creating solver with MIAM: " + std::string(error.message_.value_);
          musica::DeleteError(&error);
          throw py::value_error(message);
        }

        return std::shared_ptr<musica::MICM>(
            micm,
            [](musica::MICM* ptr)
            {
              musica::Error error;
              musica::DeleteMicm(ptr, &error);
              musica::DeleteError(&error);
            });
      },
      py::arg("mechanism"),
      py::arg("solver_type"),
      py::arg("miam_config"),
      "Create a MICM solver with MIAM aerosol model as external model");
}
