// Copyright (C) 2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Pybind11 bindings for the aerosol configuration types defined in
// mechanism_configuration/types/aerosol.hpp. These classes are added to the
// same `_mechanism_configuration` submodule as the species/reaction types and
// are attached to a Mechanism via its optional `aerosol` member.
//
// Following the convention used for the other mechanism_configuration types,
// each struct is bound with a default constructor plus read/write field access;
// the Python wrapper layer resolves Species/Phase objects down to the names the
// C++ types store.
#include "../common.hpp"

#include <mechanism_configuration/types/aerosol.hpp>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

using namespace mechanism_configuration::types;

void bind_aerosol(py::module_ &mechanism_configuration)
{
  // -- Rate constants ------------------------------------------------

  py::class_<Equilibrium>(mechanism_configuration, "_Equilibrium")
      .def(py::init<>())
      .def_readwrite("A", &Equilibrium::A)
      .def_readwrite("C", &Equilibrium::C)
      .def_readwrite("T0", &Equilibrium::T0)
      .def("__repr__", [](const Equilibrium &) { return "<Equilibrium>"; });

  py::class_<HenryLawConstant>(mechanism_configuration, "_HenryLawConstant")
      .def(py::init<>())
      .def_readwrite("HLC_ref", &HenryLawConstant::HLC_ref)
      .def_readwrite("C", &HenryLawConstant::C)
      .def_readwrite("T0", &HenryLawConstant::T0)
      .def("__repr__", [](const HenryLawConstant &) { return "<HenryLawConstant>"; });

  // -- Representations ------------------------------------------------

  py::class_<UniformSection>(mechanism_configuration, "_UniformSection")
      .def(py::init<>())
      .def_readwrite("name", &UniformSection::name)
      .def_readwrite("phases", &UniformSection::phases)
      .def_readwrite("min_radius", &UniformSection::min_radius)
      .def_readwrite("max_radius", &UniformSection::max_radius)
      .def("__str__", [](const UniformSection &r) { return r.name; })
      .def("__repr__", [](const UniformSection &r) { return "<UniformSection: " + r.name + ">"; });

  py::class_<SingleMomentMode>(mechanism_configuration, "_SingleMomentMode")
      .def(py::init<>())
      .def_readwrite("name", &SingleMomentMode::name)
      .def_readwrite("phases", &SingleMomentMode::phases)
      .def_readwrite("geometric_mean_radius", &SingleMomentMode::geometric_mean_radius)
      .def_readwrite("geometric_standard_deviation", &SingleMomentMode::geometric_standard_deviation)
      .def("__str__", [](const SingleMomentMode &r) { return r.name; })
      .def("__repr__", [](const SingleMomentMode &r) { return "<SingleMomentMode: " + r.name + ">"; });

  py::class_<TwoMomentMode>(mechanism_configuration, "_TwoMomentMode")
      .def(py::init<>())
      .def_readwrite("name", &TwoMomentMode::name)
      .def_readwrite("phases", &TwoMomentMode::phases)
      .def_readwrite("geometric_standard_deviation", &TwoMomentMode::geometric_standard_deviation)
      .def("__str__", [](const TwoMomentMode &r) { return r.name; })
      .def("__repr__", [](const TwoMomentMode &r) { return "<TwoMomentMode: " + r.name + ">"; });

  // -- Processes ------------------------------------------------------

  py::class_<DissolvedReaction>(mechanism_configuration, "_DissolvedReaction")
      .def(py::init<>())
      .def_readwrite("phase", &DissolvedReaction::phase)
      .def_readwrite("solvent", &DissolvedReaction::solvent)
      .def_readwrite("reactants", &DissolvedReaction::reactants)
      .def_readwrite("products", &DissolvedReaction::products)
      .def_readwrite("rate_constants", &DissolvedReaction::rate_constants)
      .def_readwrite("solvent_floor", &DissolvedReaction::solvent_floor_)
      .def_readwrite("min_halflife", &DissolvedReaction::min_halflife_)
      .def("__repr__", [](const DissolvedReaction &r) { return "<DissolvedReaction: " + r.phase + ">"; });

  py::class_<DissolvedReversibleReaction>(mechanism_configuration, "_DissolvedReversibleReaction")
      .def(py::init<>())
      .def_readwrite("phase", &DissolvedReversibleReaction::phase)
      .def_readwrite("solvent", &DissolvedReversibleReaction::solvent)
      .def_readwrite("reactants", &DissolvedReversibleReaction::reactants)
      .def_readwrite("products", &DissolvedReversibleReaction::products)
      .def_readwrite("forward_rate_constants", &DissolvedReversibleReaction::forward_rate_constants)
      .def_readwrite("reverse_rate_constants", &DissolvedReversibleReaction::reverse_rate_constants)
      .def_readwrite("equilibrium_constant", &DissolvedReversibleReaction::equilibrium_constant)
      .def_readwrite("solvent_floor", &DissolvedReversibleReaction::solvent_floor_)
      .def(
          "__repr__", [](const DissolvedReversibleReaction &r) { return "<DissolvedReversibleReaction: " + r.phase + ">"; });

  py::class_<HenryLawPhaseTransfer>(mechanism_configuration, "_HenryLawPhaseTransfer")
      .def(py::init<>())
      .def_readwrite("gas_phase", &HenryLawPhaseTransfer::gas_phase)
      .def_readwrite("gas_species", &HenryLawPhaseTransfer::gas_species)
      .def_readwrite("condensed_phase", &HenryLawPhaseTransfer::condensed_phase)
      .def_readwrite("condensed_species", &HenryLawPhaseTransfer::condensed_species)
      .def_readwrite("solvent", &HenryLawPhaseTransfer::solvent)
      .def_readwrite("henry_law_constant", &HenryLawPhaseTransfer::henry_law_constant)
      .def_readwrite("diffusion_coefficient", &HenryLawPhaseTransfer::diffusion_coefficient)
      .def_readwrite("accommodation_coefficient", &HenryLawPhaseTransfer::accommodation_coefficient)
      .def("__repr__", [](const HenryLawPhaseTransfer &r) { return "<HenryLawPhaseTransfer: " + r.gas_species + ">"; });

  // -- Constraints ----------------------------------------------------

  py::class_<HenryLawEquilibrium>(mechanism_configuration, "_HenryLawEquilibrium")
      .def(py::init<>())
      .def_readwrite("gas_phase", &HenryLawEquilibrium::gas_phase)
      .def_readwrite("gas_species", &HenryLawEquilibrium::gas_species)
      .def_readwrite("condensed_phase", &HenryLawEquilibrium::condensed_phase)
      .def_readwrite("condensed_species", &HenryLawEquilibrium::condensed_species)
      .def_readwrite("solvent", &HenryLawEquilibrium::solvent)
      .def_readwrite("henry_law_constant", &HenryLawEquilibrium::henry_law_constant)
      .def_readwrite("solvent_molecular_weight", &HenryLawEquilibrium::solvent_molecular_weight)
      .def_readwrite("solvent_density", &HenryLawEquilibrium::solvent_density)
      .def("__repr__", [](const HenryLawEquilibrium &c) { return "<HenryLawEquilibrium: " + c.gas_species + ">"; });

  py::class_<DissolvedEquilibrium>(mechanism_configuration, "_DissolvedEquilibrium")
      .def(py::init<>())
      .def_readwrite("phase", &DissolvedEquilibrium::phase)
      .def_readwrite("algebraic_species", &DissolvedEquilibrium::algebraic_species)
      .def_readwrite("solvent", &DissolvedEquilibrium::solvent)
      .def_readwrite("reactants", &DissolvedEquilibrium::reactants)
      .def_readwrite("products", &DissolvedEquilibrium::products)
      .def_readwrite("equilibrium_constant", &DissolvedEquilibrium::equilibrium_constant)
      .def_readwrite("solvent_floor", &DissolvedEquilibrium::solvent_floor_)
      .def("__repr__", [](const DissolvedEquilibrium &c) { return "<DissolvedEquilibrium: " + c.phase + ">"; });

  py::class_<LinearConstraintTerm>(mechanism_configuration, "_LinearConstraintTerm")
      .def(py::init<>())
      .def_readwrite("phase", &LinearConstraintTerm::phase)
      .def_readwrite("name", &LinearConstraintTerm::name)
      .def_readwrite("coefficient", &LinearConstraintTerm::coefficient)
      .def("__repr__", [](const LinearConstraintTerm &t) { return "<LinearConstraintTerm: " + t.name + ">"; });

  py::class_<FixedConstant>(mechanism_configuration, "_FixedConstant")
      .def(py::init<>())
      .def_readwrite("value", &FixedConstant::value)
      .def("__repr__", [](const FixedConstant &) { return "<FixedConstant>"; });

  py::class_<DiagnoseFromState>(mechanism_configuration, "_DiagnoseFromState")
      .def(py::init<>())
      .def("__repr__", [](const DiagnoseFromState &) { return "<DiagnoseFromState>"; });

  py::class_<LinearConstraint>(mechanism_configuration, "_LinearConstraint")
      .def(py::init<>())
      .def_readwrite("algebraic_phase", &LinearConstraint::algebraic_phase)
      .def_readwrite("algebraic_species", &LinearConstraint::algebraic_species)
      .def_readwrite("terms", &LinearConstraint::terms)
      .def_readwrite("constant", &LinearConstraint::constant)
      .def("__repr__", [](const LinearConstraint &c) { return "<LinearConstraint: " + c.algebraic_species + ">"; });

  // -- Container ------------------------------------------------------

  py::class_<Aerosol>(mechanism_configuration, "_Aerosol")
      .def(py::init<>())
      .def_readwrite("representations", &Aerosol::representations)
      .def_readwrite("processes", &Aerosol::processes)
      .def_readwrite("constraints", &Aerosol::constraints)
      .def("__repr__", [](const Aerosol &) { return "<Aerosol>"; });
}
