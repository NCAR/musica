// Copyright (C) 2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <mechanism_configuration/constants.hpp>
#include <mechanism_configuration/v1/parser.hpp>
#include <mechanism_configuration/v1/types.hpp>
#include <mechanism_configuration/v1/validation.hpp>
#include <variant>

namespace py = pybind11;
namespace constants = mechanism_configuration::constants;
namespace validation = mechanism_configuration::v1::validation;
using namespace mechanism_configuration::v1::types;

enum class ReactionType
{
  Arrhenius,
  Branched,
  CondensedPhaseArrhenius,
  CondensedPhasePhotolysis,
  Emission,
  FirstOrderLoss,
  SimpolPhaseTransfer,
  AqueousEquilibrium,
  WetDeposition,
  HenrysLaw,
  Photolysis,
  Surface,
  Troe,
  Tunneling,
  UserDefined
};

struct ReactionsIterator
{
  using VariantType = std::variant<
      Arrhenius,
      Branched,
      CondensedPhaseArrhenius,
      CondensedPhasePhotolysis,
      Emission,
      FirstOrderLoss,
      SimpolPhaseTransfer,
      AqueousEquilibrium,
      WetDeposition,
      HenrysLaw,
      Photolysis,
      Surface,
      Troe,
      Tunneling,
      UserDefined>;

  std::vector<std::vector<VariantType>> reaction_lists;
  size_t outer_index = 0;
  size_t inner_index = 0;

  ReactionsIterator(Reactions &reactions)
      : reaction_lists{ std::vector<VariantType>(reactions.arrhenius.begin(), reactions.arrhenius.end()),
                        std::vector<VariantType>(reactions.branched.begin(), reactions.branched.end()),
                        std::vector<VariantType>(reactions.condensed_phase_arrhenius.begin(), reactions.condensed_phase_arrhenius.end()),
                        std::vector<VariantType>(reactions.condensed_phase_photolysis.begin(), reactions.condensed_phase_photolysis.end()),
                        std::vector<VariantType>(reactions.emission.begin(), reactions.emission.end()),
                        std::vector<VariantType>(reactions.first_order_loss.begin(), reactions.first_order_loss.end()),
                        std::vector<VariantType>(reactions.simpol_phase_transfer.begin(), reactions.simpol_phase_transfer.end()),
                        std::vector<VariantType>(reactions.aqueous_equilibrium.begin(), reactions.aqueous_equilibrium.end()),
                        std::vector<VariantType>(reactions.wet_deposition.begin(), reactions.wet_deposition.end()),
                        std::vector<VariantType>(reactions.henrys_law.begin(), reactions.henrys_law.end()),
                        std::vector<VariantType>(reactions.photolysis.begin(), reactions.photolysis.end()),
                        std::vector<VariantType>(reactions.surface.begin(), reactions.surface.end()),
                        std::vector<VariantType>(reactions.troe.begin(), reactions.troe.end()),
                        std::vector<VariantType>(reactions.tunneling.begin(), reactions.tunneling.end()),
                        std::vector<VariantType>(reactions.user_defined.begin(), reactions.user_defined.end()) }
  {
  }

  py::object next()
  {
    while (outer_index < reaction_lists.size())
    {
      const auto &vec = reaction_lists[outer_index];
      if (inner_index < vec.size())
      {
        return std::visit([](auto &&arg) { return py::cast(arg); }, vec[inner_index++]);
      }
      ++outer_index;
      inner_index = 0;
    }
    throw py::stop_iteration();
  }
};

std::vector<ReactionComponent> get_reaction_components(const py::list& components)
{
  std::vector<ReactionComponent> reaction_components;
  for (const auto &item : components) {
    if (py::isinstance<Species>(item)) {
        ReactionComponent component;
        component.species_name = item.cast<Species>().name;
        reaction_components.push_back(component);
    } else if (py::isinstance<py::tuple>(item) && py::len(item.cast<py::tuple>()) == 2) {
        auto item_tuple = item.cast<py::tuple>();
        if (py::isinstance<py::float_>(item_tuple[0]) && py::isinstance<Species>(item_tuple[1])) {
            ReactionComponent component;
            component.species_name = item_tuple[1].cast<Species>().name;
            component.coefficient = item_tuple[0].cast<double>();
            reaction_components.push_back(component);
        } else if (py::isinstance<py::int_>(item_tuple[0]) && py::isinstance<Species>(item_tuple[1])) {
            ReactionComponent component;
            component.species_name = item_tuple[1].cast<Species>().name;
            component.coefficient = item_tuple[0].cast<int>();
            reaction_components.push_back(component);
        } else {
            throw py::value_error("Invalid tuple format. Expected (float, Species).");
        }
    } else {
        throw py::value_error("Invalid type for reactant. Expected a Species or a tuple of (float, Species).");
    }
  }
  std::unordered_set<std::string> component_names;
  for (const auto &component : reaction_components) {
    if (!component_names.insert(component.species_name).second) {
        throw py::value_error("Duplicate reaction component name found: " + component.species_name);
    }
  }
  return reaction_components;
}

Reactions create_reactions(const py::list& reactions)
{
  Reactions reaction_obj;
  for (const auto &item : reactions) {
    if (py::isinstance<Arrhenius>(item)) {
      reaction_obj.arrhenius.push_back(item.cast<Arrhenius>());
    } else if (py::isinstance<Branched>(item)) {
      reaction_obj.branched.push_back(item.cast<Branched>());
    } else if (py::isinstance<CondensedPhaseArrhenius>(item)) {
      reaction_obj.condensed_phase_arrhenius.push_back(item.cast<CondensedPhaseArrhenius>());
    } else if (py::isinstance<CondensedPhasePhotolysis>(item)) {
      reaction_obj.condensed_phase_photolysis.push_back(item.cast<CondensedPhasePhotolysis>());
    } else if (py::isinstance<Emission>(item)) {
      reaction_obj.emission.push_back(item.cast<Emission>());
    } else if (py::isinstance<FirstOrderLoss>(item)) {
      reaction_obj.first_order_loss.push_back(item.cast<FirstOrderLoss>());
    } else if (py::isinstance<SimpolPhaseTransfer>(item)) {
      reaction_obj.simpol_phase_transfer.push_back(item.cast<SimpolPhaseTransfer>());
    } else if (py::isinstance<AqueousEquilibrium>(item)) {
      reaction_obj.aqueous_equilibrium.push_back(item.cast<AqueousEquilibrium>());
    } else if (py::isinstance<WetDeposition>(item)) {
      reaction_obj.wet_deposition.push_back(item.cast<WetDeposition>());
    } else if (py::isinstance<HenrysLaw>(item)) {
      reaction_obj.henrys_law.push_back(item.cast<HenrysLaw>());
    } else if (py::isinstance<Photolysis>(item)) {
      reaction_obj.photolysis.push_back(item.cast<Photolysis>());
    } else if (py::isinstance<Surface>(item)) {
      reaction_obj.surface.push_back(item.cast<Surface>());
    } else if (py::isinstance<Troe>(item)) {
      reaction_obj.troe.push_back(item.cast<Troe>());
    } else if (py::isinstance<Tunneling>(item)) {
      reaction_obj.tunneling.push_back(item.cast<Tunneling>());
    } else if (py::isinstance<UserDefined>(item)) {
      reaction_obj.user_defined.push_back(item.cast<UserDefined>());
    } else {
        throw py::value_error("Invalid reaction type.");
    }
  }
  return reaction_obj;
}

void bind_mechanism_configuration(py::module_ & mechanism_configuration)
{
  py::enum_<ReactionType>(mechanism_configuration, "_ReactionType")
      .value("Arrhenius", ReactionType::Arrhenius)
      .value("Branched", ReactionType::Branched)
      .value("CondensedPhaseArrhenius", ReactionType::CondensedPhaseArrhenius)
      .value("CondensedPhasePhotolysis", ReactionType::CondensedPhasePhotolysis)
      .value("Emission", ReactionType::Emission)
      .value("FirstOrderLoss", ReactionType::FirstOrderLoss)
      .value("SimpolPhaseTransfer", ReactionType::SimpolPhaseTransfer)
      .value("AqueousEquilibrium", ReactionType::AqueousEquilibrium)
      .value("WetDeposition", ReactionType::WetDeposition)
      .value("HenrysLaw", ReactionType::HenrysLaw)
      .value("Photolysis", ReactionType::Photolysis)
      .value("Surface", ReactionType::Surface)
      .value("Troe", ReactionType::Troe)
      .value("Tunneling", ReactionType::Tunneling)
      .value("UserDefined", ReactionType::UserDefined);

  py::class_<Species>(mechanism_configuration, "_Species")
      .def(py::init<>())
      .def_readwrite("name", &Species::name)
      .def_readwrite("absolute_tolerance", &Species::absolute_tolerance)
      .def_readwrite("diffusion_coefficient_m2_s", &Species::diffusion_coefficient)
      .def_readwrite("molecular_weight_kg_mol", &Species::molecular_weight)
      .def_readwrite("HLC_298K_mol_m3_Pa", &Species::henrys_law_constant_298)
      .def_readwrite("HLC_exponential_factor_K", &Species::henrys_law_constant_exponential_factor)
      .def_readwrite("N_star", &Species::n_star)
      .def_readwrite("density_kg_m3", &Species::density)
      .def_readwrite("tracer_type", &Species::tracer_type)
      .def_readwrite("other_properties", &Species::unknown_properties)
      .def("__str__", [](const Species &s) { return s.name; })
      .def("__repr__", [](const Species &s) { return "<Species: " + s.name + ">"; });

  py::class_<Phase>(mechanism_configuration, "_Phase")
      .def(py::init<>())
      .def_readwrite("name", &Phase::name)
      .def_readwrite("species", &Phase::species)
      .def_readwrite("other_properties", &Phase::unknown_properties)
      .def("__str__", [](const Phase &p) { return p.name; })
      .def("__repr__", [](const Phase &p) { return "<Phase: " + p.name + ">"; });

  py::class_<ReactionComponent>(mechanism_configuration, "_ReactionComponent")
      .def(py::init<>())
      .def(py::init([](const std::string &species_name) {
          ReactionComponent rc;
          rc.species_name = species_name;
          return rc;
      }))
      .def(py::init([](const std::string &species_name, double coefficient) {
          ReactionComponent rc;
          rc.species_name = species_name;
          rc.coefficient = coefficient;
          return rc;
      }))
      .def_readwrite("species_name", &ReactionComponent::species_name)
      .def_readwrite("coefficient", &ReactionComponent::coefficient)
      .def_readwrite("other_properties", &ReactionComponent::unknown_properties)
      .def("__str__", [](const ReactionComponent &rc) { return rc.species_name; })
      .def("__repr__", [](const ReactionComponent &rc) { return "<ReactionComponent: " + rc.species_name + ">"; });

  py::class_<Arrhenius>(mechanism_configuration, "_Arrhenius")
      .def(py::init<>())
      .def_readwrite("A", &Arrhenius::A)
      .def_readwrite("B", &Arrhenius::B)
      .def_readwrite("C", &Arrhenius::C)
      .def_readwrite("D", &Arrhenius::D)
      .def_readwrite("E", &Arrhenius::E)
      .def_readwrite("reactants", &Arrhenius::reactants)
      .def_readwrite("products", &Arrhenius::products)
      .def_readwrite("name", &Arrhenius::name)
      .def_readwrite("gas_phase", &Arrhenius::gas_phase)
      .def_readwrite("other_properties", &Arrhenius::unknown_properties)
      .def("__str__", [](const Arrhenius &a) { return a.name; })
      .def("__repr__", [](const Arrhenius &a) { return "<Arrhenius: " + a.name + ">"; })
      .def_property_readonly("type", [](const Arrhenius &) { return ReactionType::Arrhenius; });

  py::class_<CondensedPhaseArrhenius>(mechanism_configuration, "_CondensedPhaseArrhenius")
      .def(py::init<>())
      .def_readwrite("A", &CondensedPhaseArrhenius::A)
      .def_readwrite("B", &CondensedPhaseArrhenius::B)
      .def_readwrite("C", &CondensedPhaseArrhenius::C)
      .def_readwrite("D", &CondensedPhaseArrhenius::D)
      .def_readwrite("E", &CondensedPhaseArrhenius::E)
      .def_readwrite("reactants", &CondensedPhaseArrhenius::reactants)
      .def_readwrite("products", &CondensedPhaseArrhenius::products)
      .def_readwrite("name", &CondensedPhaseArrhenius::name)
      .def_readwrite("aerosol_phase", &CondensedPhaseArrhenius::aerosol_phase)
      .def_readwrite("aerosol_phase_water", &CondensedPhaseArrhenius::aerosol_phase_water)
      .def_readwrite("other_properties", &CondensedPhaseArrhenius::unknown_properties)
      .def("__str__", [](const CondensedPhaseArrhenius &cpa) { return cpa.name; })
      .def("__repr__", [](const CondensedPhaseArrhenius &cpa) { return "<CondensedPhaseArrhenius: " + cpa.name + ">"; })
      .def_property_readonly("type", [](const CondensedPhaseArrhenius &) { return ReactionType::CondensedPhaseArrhenius; });

  py::class_<Troe>(mechanism_configuration, "_Troe")
      .def(py::init<>())
      .def_readwrite("k0_A", &Troe::k0_A)
      .def_readwrite("k0_B", &Troe::k0_B)
      .def_readwrite("k0_C", &Troe::k0_C)
      .def_readwrite("kinf_A", &Troe::kinf_A)
      .def_readwrite("kinf_B", &Troe::kinf_B)
      .def_readwrite("kinf_C", &Troe::kinf_C)
      .def_readwrite("Fc", &Troe::Fc)
      .def_readwrite("N", &Troe::N)
      .def_readwrite("reactants", &Troe::reactants)
      .def_readwrite("products", &Troe::products)
      .def_readwrite("name", &Troe::name)
      .def_readwrite("gas_phase", &Troe::gas_phase)
      .def_readwrite("other_properties", &Troe::unknown_properties)
      .def("__str__", [](const Troe &t) { return t.name; })
      .def("__repr__", [](const Troe &t) { return "<Troe: " + t.name + ">"; })
      .def_property_readonly("type", [](const Troe &) { return ReactionType::Troe; });

  py::class_<Branched>(mechanism_configuration, "_Branched")
      .def(py::init<>())
      .def_readwrite("X", &Branched::X)
      .def_readwrite("Y", &Branched::Y)
      .def_readwrite("a0", &Branched::a0)
      .def_readwrite("n", &Branched::n)
      .def_readwrite("reactants", &Branched::reactants)
      .def_readwrite("nitrate_products", &Branched::nitrate_products)
      .def_readwrite("alkoxy_products", &Branched::alkoxy_products)
      .def_readwrite("name", &Branched::name)
      .def_readwrite("gas_phase", &Branched::gas_phase)
      .def_readwrite("other_properties", &Branched::unknown_properties)
      .def("__str__", [](const Branched &b) { return b.name; })
      .def("__repr__", [](const Branched &b) { return "<Branched: " + b.name + ">"; })
      .def_property_readonly("type", [](const Branched &) { return ReactionType::Branched; });

  py::class_<Tunneling>(mechanism_configuration, "_Tunneling")
      .def(py::init<>())
      .def_readwrite("A", &Tunneling::A)
      .def_readwrite("B", &Tunneling::B)
      .def_readwrite("C", &Tunneling::C)
      .def_readwrite("reactants", &Tunneling::reactants)
      .def_readwrite("products", &Tunneling::products)
      .def_readwrite("name", &Tunneling::name)
      .def_readwrite("gas_phase", &Tunneling::gas_phase)
      .def_readwrite("other_properties", &Tunneling::unknown_properties)
      .def("__str__", [](const Tunneling &t) { return t.name; })
      .def("__repr__", [](const Tunneling &t) { return "<Tunneling: " + t.name + ">"; })
      .def_property_readonly("type", [](const Tunneling &) { return ReactionType::Tunneling; });

  py::class_<Surface>(mechanism_configuration, "_Surface")
      .def(py::init<>())
      .def_readwrite("reaction_probability", &Surface::reaction_probability)
      .def_readwrite("gas_phase_species", &Surface::gas_phase_species)
      .def_readwrite("gas_phase_products", &Surface::gas_phase_products)
      .def_readwrite("name", &Surface::name)
      .def_readwrite("gas_phase", &Surface::gas_phase)
      .def_readwrite("aerosol_phase", &Surface::aerosol_phase)
      .def_readwrite("other_properties", &Surface::unknown_properties)
      .def("__str__", [](const Surface &s) { return s.name; })
      .def("__repr__", [](const Surface &s) { return "<Surface: " + s.name + ">"; })
      .def_property_readonly("type", [](const Surface &) { return ReactionType::Surface; });

  py::class_<Photolysis>(mechanism_configuration, "_Photolysis")
      .def(py::init<>())
      .def_readwrite("scaling_factor", &Photolysis::scaling_factor)
      .def_readwrite("reactants", &Photolysis::reactants)
      .def_readwrite("products", &Photolysis::products)
      .def_readwrite("name", &Photolysis::name)
      .def_readwrite("gas_phase", &Photolysis::gas_phase)
      .def_readwrite("other_properties", &Photolysis::unknown_properties)
      .def("__str__", [](const Photolysis &p) { return p.name; })
      .def("__repr__", [](const Photolysis &p) { return "<Photolysis: " + p.name + ">"; })
      .def_property_readonly("type", [](const Photolysis &) { return ReactionType::Photolysis; });

  py::class_<CondensedPhasePhotolysis>(mechanism_configuration, "_CondensedPhasePhotolysis")
      .def(py::init<>())
      .def_readwrite("scaling_factor", &CondensedPhasePhotolysis::scaling_factor)
      .def_readwrite("reactants", &CondensedPhasePhotolysis::reactants)
      .def_readwrite("products", &CondensedPhasePhotolysis::products)
      .def_readwrite("name", &CondensedPhasePhotolysis::name)
      .def_readwrite("aerosol_phase", &CondensedPhasePhotolysis::aerosol_phase)
      .def_readwrite("aerosol_phase_water", &CondensedPhasePhotolysis::aerosol_phase_water)
      .def_readwrite("other_properties", &CondensedPhasePhotolysis::unknown_properties)
      .def("__str__", [](const CondensedPhasePhotolysis &cpp) { return cpp.name; })
      .def("__repr__", [](const CondensedPhasePhotolysis &cpp) { return "<CondensedPhasePhotolysis: " + cpp.name + ">"; })
      .def_property_readonly("type", [](const CondensedPhasePhotolysis &) { return ReactionType::CondensedPhasePhotolysis; });

  py::class_<Emission>(mechanism_configuration, "_Emission")
      .def(py::init<>())
      .def_readwrite("scaling_factor", &Emission::scaling_factor)
      .def_readwrite("products", &Emission::products)
      .def_readwrite("name", &Emission::name)
      .def_readwrite("gas_phase", &Emission::gas_phase)
      .def_readwrite("other_properties", &Emission::unknown_properties)
      .def("__str__", [](const Emission &e) { return e.name; })
      .def("__repr__", [](const Emission &e) { return "<Emission: " + e.name + ">"; })
      .def_property_readonly("type", [](const Emission &) { return ReactionType::Emission; });

  py::class_<FirstOrderLoss>(mechanism_configuration, "_FirstOrderLoss")
      .def(py::init<>())
      .def_readwrite("scaling_factor", &FirstOrderLoss::scaling_factor)
      .def_readwrite("reactants", &FirstOrderLoss::reactants)
      .def_readwrite("name", &FirstOrderLoss::name)
      .def_readwrite("gas_phase", &FirstOrderLoss::gas_phase)
      .def_readwrite("other_properties", &FirstOrderLoss::unknown_properties)
      .def("__str__", [](const FirstOrderLoss &fol) { return fol.name; })
      .def("__repr__", [](const FirstOrderLoss &fol) { return "<FirstOrderLoss: " + fol.name + ">"; })
      .def_property_readonly("type", [](const FirstOrderLoss &) { return ReactionType::FirstOrderLoss; });

  py::class_<AqueousEquilibrium>(mechanism_configuration, "_AqueousEquilibrium")
      .def(py::init<>())
      .def_readwrite("name", &AqueousEquilibrium::name)
      .def_readwrite("gas_phase", &AqueousEquilibrium::gas_phase)
      .def_readwrite("aerosol_phase", &AqueousEquilibrium::aerosol_phase)
      .def_readwrite("aerosol_phase_water", &AqueousEquilibrium::aerosol_phase_water)
      .def_readwrite("reactants", &AqueousEquilibrium::reactants)
      .def_readwrite("products", &AqueousEquilibrium::products)
      .def_readwrite("A", &AqueousEquilibrium::A)
      .def_readwrite("C", &AqueousEquilibrium::C)
      .def_readwrite("k_reverse", &AqueousEquilibrium::k_reverse)
      .def_readwrite("other_properties", &AqueousEquilibrium::unknown_properties)
      .def("__str__", [](const AqueousEquilibrium &ae) { return ae.name; })
      .def("__repr__", [](const AqueousEquilibrium &ae) { return "<AqueousEquilibrium: " + ae.name + ">"; })
      .def_property_readonly("type", [](const AqueousEquilibrium &) { return ReactionType::AqueousEquilibrium; });

  py::class_<WetDeposition>(mechanism_configuration, "_WetDeposition")
      .def(py::init<>())
      .def_readwrite("scaling_factor", &WetDeposition::scaling_factor)
      .def_readwrite("name", &WetDeposition::name)
      .def_readwrite("aerosol_phase", &WetDeposition::aerosol_phase)
      .def_readwrite("other_properties", &WetDeposition::unknown_properties)
      .def("__str__", [](const WetDeposition &wd) { return wd.name; })
      .def("__repr__", [](const WetDeposition &wd) { return "<WetDeposition: " + wd.name + ">"; })
      .def_property_readonly("type", [](const WetDeposition &) { return ReactionType::WetDeposition; });

  py::class_<HenrysLaw>(mechanism_configuration, "_HenrysLaw")
      .def(py::init<>())
      .def_readwrite("name", &HenrysLaw::name)
      .def_readwrite("gas_phase", &HenrysLaw::gas_phase)
      .def_readwrite("gas_phase_species", &HenrysLaw::gas_phase_species)
      .def_readwrite("aerosol_phase", &HenrysLaw::aerosol_phase)
      .def_readwrite("aerosol_phase_water", &HenrysLaw::aerosol_phase_water)
      .def_readwrite("aerosol_phase_species", &HenrysLaw::aerosol_phase_species)
      .def_readwrite("other_properties", &HenrysLaw::unknown_properties)
      .def("__str__", [](const HenrysLaw &hl) { return hl.name; })
      .def("__repr__", [](const HenrysLaw &hl) { return "<HenrysLaw: " + hl.name + ">"; })
      .def_property_readonly("type", [](const HenrysLaw &) { return ReactionType::HenrysLaw; });

  py::class_<SimpolPhaseTransfer>(mechanism_configuration, "_SimpolPhaseTransfer")
      .def(py::init<>())
      .def_readwrite("gas_phase", &SimpolPhaseTransfer::gas_phase)
      .def_readwrite("gas_phase_species", &SimpolPhaseTransfer::gas_phase_species)
      .def_readwrite("aerosol_phase", &SimpolPhaseTransfer::aerosol_phase)
      .def_readwrite("aerosol_phase_species", &SimpolPhaseTransfer::aerosol_phase_species)
      .def_readwrite("name", &SimpolPhaseTransfer::name)
      .def_readwrite("B", &SimpolPhaseTransfer::B)
      .def_readwrite("other_properties", &SimpolPhaseTransfer::unknown_properties)
      .def("__str__", [](const SimpolPhaseTransfer &spt) { return spt.name; })
      .def("__repr__", [](const SimpolPhaseTransfer &spt) { return "<SimpolPhaseTransfer: " + spt.name + ">"; })
      .def_property_readonly("type", [](const SimpolPhaseTransfer &) { return ReactionType::SimpolPhaseTransfer; });

  py::class_<UserDefined>(mechanism_configuration, "_UserDefined")
      .def(py::init<>())
      .def_readwrite("scaling_factor", &UserDefined::scaling_factor)
      .def_readwrite("reactants", &UserDefined::reactants)
      .def_readwrite("products", &UserDefined::products)
      .def_readwrite("name", &UserDefined::name)
      .def_readwrite("gas_phase", &UserDefined::gas_phase)
      .def_readwrite("other_properties", &UserDefined::unknown_properties)
      .def("__str__", [](const UserDefined &p) { return p.name; })
      .def("__repr__", [](const UserDefined &p) { return "<UserDefined: " + p.name + ">"; })
      .def_property_readonly("type", [](const UserDefined &) { return ReactionType::UserDefined; });

  py::class_<Reactions>(mechanism_configuration, "_Reactions")
      .def(py::init<>())
      .def(py::init([](const py::list &reactions) {
          return create_reactions(reactions);
      }))
      .def_readwrite("arrhenius", &Reactions::arrhenius)
      .def_readwrite("branched", &Reactions::branched)
      .def_readwrite("condensed_phase_arrhenius", &Reactions::condensed_phase_arrhenius)
      .def_readwrite("condensed_phase_photolysis", &Reactions::condensed_phase_photolysis)
      .def_readwrite("emission", &Reactions::emission)
      .def_readwrite("first_order_loss", &Reactions::first_order_loss)
      .def_readwrite("simpol_phase_transfer", &Reactions::simpol_phase_transfer)
      .def_readwrite("aqueous_equilibrium", &Reactions::aqueous_equilibrium)
      .def_readwrite("wet_deposition", &Reactions::wet_deposition)
      .def_readwrite("henrys_law", &Reactions::henrys_law)
      .def_readwrite("photolysis", &Reactions::photolysis)
      .def_readwrite("surface", &Reactions::surface)
      .def_readwrite("troe", &Reactions::troe)
      .def_readwrite("tunneling", &Reactions::tunneling)
      .def_readwrite("user_defined", &Reactions::user_defined)
      .def(
          "__len__",
          [](const Reactions &r)
          {
            return r.arrhenius.size() + r.branched.size() + r.condensed_phase_arrhenius.size() + r.condensed_phase_photolysis.size() +
                   r.emission.size() + r.first_order_loss.size() + r.simpol_phase_transfer.size() + r.aqueous_equilibrium.size() +
                   r.wet_deposition.size() + r.henrys_law.size() + r.photolysis.size() + r.surface.size() + r.troe.size() + r.tunneling.size() +
                   r.user_defined.size();
          })
      .def("__str__", [](const Reactions &r) { return "Reactions"; })
      .def("__repr__", [](const Reactions &r) { return "<Reactions>"; })
      .def("__iter__", [](Reactions &r) { return ReactionsIterator(r); });

  py::class_<ReactionsIterator>(mechanism_configuration, "_ReactionsIterator")
      .def("__iter__", [](ReactionsIterator &it) -> ReactionsIterator & { return it; })
      .def("__next__", &ReactionsIterator::next);

  py::class_<Mechanism>(mechanism_configuration, "_Mechanism")
      .def(py::init<>())
      .def_readwrite("name", &Mechanism::name)
      .def_readwrite("species", &Mechanism::species)
      .def_readwrite("phases", &Mechanism::phases)
      .def_readwrite("reactions", &Mechanism::reactions)
      .def_readwrite("version", &Mechanism::version)
      .def("__str__", [](const Mechanism &m) { return m.name; })
      .def("__repr__", [](const Mechanism &m) { return "<Mechanism: " + m.name + ">"; });

  py::class_<mechanism_configuration::Version>(mechanism_configuration, "_Version")
      .def(py::init<>())
      .def(py::init<unsigned int, unsigned int, unsigned int>())
      .def(py::init<std::string>())
      .def_readwrite("major", &mechanism_configuration::Version::major)
      .def_readwrite("minor", &mechanism_configuration::Version::minor)
      .def_readwrite("patch", &mechanism_configuration::Version::patch)
      .def("to_string", &mechanism_configuration::Version::to_string)
      .def("__str__", &mechanism_configuration::Version::to_string)
      .def("__repr__", [](const mechanism_configuration::Version &v) { return "<Version: " + v.to_string() + ">"; });

  using V1Parser = mechanism_configuration::v1::Parser;

  py::class_<V1Parser>(mechanism_configuration, "_Parser")
      .def(py::init<>())
      .def(
          "parse",
          [](V1Parser &self, const std::string &path)
          {
            auto parsed = self.Parse(std::filesystem::path(path));
            if (parsed)
            {
              return *parsed;
            }
            else
            {
              std::string error = "Error parsing file: " + path + "\n";
              for (auto &e : parsed.errors)
              {
                error += e.second + "\n";
              }
              throw std::runtime_error(error);
            }
          });
}
