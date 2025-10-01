// Copyright (C) 2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include "binding_common.hpp"

#include <musica/micm/parse.hpp>

#include <mechanism_configuration/constants.hpp>
#include <mechanism_configuration/v1/parser.hpp>
#include <mechanism_configuration/v1/reaction_types.hpp>
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
  Emission,
  FirstOrderLoss,
  Photolysis,
  Surface,
  TaylorSeries,
  TernaryChemicalActivation,
  Troe,
  Tunneling,
  UserDefined
};

struct ReactionsIterator
{
  using VariantType = std::variant<
      Arrhenius,
      Branched,
      Emission,
      FirstOrderLoss,
      Photolysis,
      Surface,
      TaylorSeries,
      TernaryChemicalActivation,
      Troe,
      Tunneling,
      UserDefined>;

  std::vector<std::vector<VariantType>> reaction_lists;
  size_t outer_index = 0;
  size_t inner_index = 0;

  ReactionsIterator(Reactions &reactions)
      : reaction_lists{ std::vector<VariantType>(reactions.arrhenius.begin(), reactions.arrhenius.end()),
                        std::vector<VariantType>(reactions.branched.begin(), reactions.branched.end()),
                        std::vector<VariantType>(reactions.emission.begin(), reactions.emission.end()),
                        std::vector<VariantType>(reactions.first_order_loss.begin(), reactions.first_order_loss.end()),
                        std::vector<VariantType>(reactions.photolysis.begin(), reactions.photolysis.end()),
                        std::vector<VariantType>(reactions.surface.begin(), reactions.surface.end()),
                        std::vector<VariantType>(reactions.taylor_series.begin(), reactions.taylor_series.end()),
                        std::vector<VariantType>(
                            reactions.ternary_chemical_activation.begin(),
                            reactions.ternary_chemical_activation.end()),
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

std::vector<ReactionComponent> get_reaction_components(const py::list &components)
{
  std::vector<ReactionComponent> reaction_components;
  for (const auto &item : components)
  {
    if (py::isinstance<Species>(item))
    {
      ReactionComponent component;
      component.species_name = item.cast<Species>().name;
      reaction_components.push_back(component);
    }
    else if (py::isinstance<py::tuple>(item) && py::len(item.cast<py::tuple>()) == 2)
    {
      auto item_tuple = item.cast<py::tuple>();
      if (py::isinstance<py::float_>(item_tuple[0]) && py::isinstance<Species>(item_tuple[1]))
      {
        ReactionComponent component;
        component.species_name = item_tuple[1].cast<Species>().name;
        component.coefficient = item_tuple[0].cast<double>();
        reaction_components.push_back(component);
      }
      else if (py::isinstance<py::int_>(item_tuple[0]) && py::isinstance<Species>(item_tuple[1]))
      {
        ReactionComponent component;
        component.species_name = item_tuple[1].cast<Species>().name;
        component.coefficient = item_tuple[0].cast<int>();
        reaction_components.push_back(component);
      }
      else
      {
        throw py::value_error("Invalid tuple format. Expected (float, Species).");
      }
    }
    else
    {
      throw py::value_error("Invalid type for reactant. Expected a Species or a tuple of (float, Species).");
    }
  }
  std::unordered_set<std::string> component_names;
  for (const auto &component : reaction_components)
  {
    if (!component_names.insert(component.species_name).second)
    {
      throw py::value_error("Duplicate reaction component name found: " + component.species_name);
    }
  }
  return reaction_components;
}

Reactions create_reactions(const py::list &reactions)
{
  Reactions reaction_obj;
  for (const auto &item : reactions)
  {
    if (py::isinstance<Arrhenius>(item))
    {
      reaction_obj.arrhenius.push_back(item.cast<Arrhenius>());
    }
    else if (py::isinstance<Branched>(item))
    {
      reaction_obj.branched.push_back(item.cast<Branched>());
    }
    else if (py::isinstance<Emission>(item))
    {
      reaction_obj.emission.push_back(item.cast<Emission>());
    }
    else if (py::isinstance<FirstOrderLoss>(item))
    {
      reaction_obj.first_order_loss.push_back(item.cast<FirstOrderLoss>());
    }
    else if (py::isinstance<Photolysis>(item))
    {
      reaction_obj.photolysis.push_back(item.cast<Photolysis>());
    }
    else if (py::isinstance<Surface>(item))
    {
      reaction_obj.surface.push_back(item.cast<Surface>());
    }
    else if (py::isinstance<TaylorSeries>(item))
    {
      reaction_obj.taylor_series.push_back(item.cast<TaylorSeries>());
    }
    else if (py::isinstance<TernaryChemicalActivation>(item))
    {
      reaction_obj.ternary_chemical_activation.push_back(item.cast<TernaryChemicalActivation>());
    }
    else if (py::isinstance<Troe>(item))
    {
      reaction_obj.troe.push_back(item.cast<Troe>());
    }
    else if (py::isinstance<Tunneling>(item))
    {
      reaction_obj.tunneling.push_back(item.cast<Tunneling>());
    }
    else if (py::isinstance<UserDefined>(item))
    {
      reaction_obj.user_defined.push_back(item.cast<UserDefined>());
    }
    else
    {
      throw py::value_error("Invalid reaction type.");
    }
  }
  return reaction_obj;
}

void bind_mechanism_configuration(py::module_ &mechanism_configuration)
{
  py::enum_<ReactionType>(mechanism_configuration, "_ReactionType")
      .value("Arrhenius", ReactionType::Arrhenius)
      .value("Branched", ReactionType::Branched)
      .value("Emission", ReactionType::Emission)
      .value("FirstOrderLoss", ReactionType::FirstOrderLoss)
      .value("Photolysis", ReactionType::Photolysis)
      .value("Surface", ReactionType::Surface)
      .value("TaylorSeries", ReactionType::TaylorSeries)
      .value("TernaryChemicalActivation", ReactionType::TernaryChemicalActivation)
      .value("Troe", ReactionType::Troe)
      .value("Tunneling", ReactionType::Tunneling)
      .value("UserDefined", ReactionType::UserDefined);

  py::class_<Species>(mechanism_configuration, "_Species")
      .def(py::init<>())
      .def_readwrite("name", &Species::name)
      .def_readwrite("molecular_weight_kg_mol", &Species::molecular_weight)
      .def_readwrite("constant_concentration_mol_m3", &Species::constant_concentration)
      .def_readwrite("constant_mixing_ratio_mol_mol", &Species::constant_mixing_ratio)
      .def_readwrite("is_third_body", &Species::is_third_body)
      .def_readwrite("other_properties", &Species::unknown_properties)
      .def("__str__", [](const Species &s) { return s.name; })
      .def("__repr__", [](const Species &s) { return "<Species: " + s.name + ">"; });

  py::class_<PhaseSpecies>(mechanism_configuration, "_PhaseSpecies")
      .def(py::init<>())
      .def_readwrite("name", &PhaseSpecies::name)
      .def_readwrite("diffusion_coefficient_m2_s", &PhaseSpecies::diffusion_coefficient)
      .def_readwrite("other_properties", &PhaseSpecies::unknown_properties)
      .def("__str__", [](const PhaseSpecies &s) { return s.name; })
      .def("__repr__", [](const PhaseSpecies &s) { return "<PhaseSpecies: " + s.name + ">"; });

  py::class_<Phase>(mechanism_configuration, "_Phase")
      .def(py::init<>())
      .def_readwrite("name", &Phase::name)
      .def_readwrite("species", &Phase::species)
      .def_readwrite("other_properties", &Phase::unknown_properties)
      .def("__str__", [](const Phase &p) { return p.name; })
      .def("__repr__", [](const Phase &p) { return "<Phase: " + p.name + ">"; });

  py::class_<ReactionComponent>(mechanism_configuration, "_ReactionComponent")
      .def(py::init<>())
      .def(py::init(
          [](const std::string &species_name)
          {
            ReactionComponent rc;
            rc.species_name = species_name;
            return rc;
          }))
      .def(py::init(
          [](const std::string &species_name, double coefficient)
          {
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

  py::class_<TaylorSeries>(mechanism_configuration, "_TaylorSeries")
      .def(py::init<>())
      .def_readwrite("A", &TaylorSeries::A)
      .def_readwrite("B", &TaylorSeries::B)
      .def_readwrite("C", &TaylorSeries::C)
      .def_readwrite("D", &TaylorSeries::D)
      .def_readwrite("E", &TaylorSeries::E)
      .def_readwrite("taylor_coefficients", &TaylorSeries::taylor_coefficients)
      .def_readwrite("name", &TaylorSeries::name)
      .def_readwrite("gas_phase", &TaylorSeries::gas_phase)
      .def_readwrite("reactants", &TaylorSeries::reactants)
      .def_readwrite("products", &TaylorSeries::products)
      .def_readwrite("other_properties", &TaylorSeries::unknown_properties)
      .def("__str__", [](const TaylorSeries &ts) { return "TaylorSeries"; })
      .def("__repr__", [](const TaylorSeries &ts) { return "<TaylorSeries>"; });

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

  py::class_<TernaryChemicalActivation>(mechanism_configuration, "_TernaryChemicalActivation")
      .def(py::init<>())
      .def_readwrite("k0_A", &TernaryChemicalActivation::k0_A)
      .def_readwrite("k0_B", &TernaryChemicalActivation::k0_B)
      .def_readwrite("k0_C", &TernaryChemicalActivation::k0_C)
      .def_readwrite("kinf_A", &TernaryChemicalActivation::kinf_A)
      .def_readwrite("kinf_B", &TernaryChemicalActivation::kinf_B)
      .def_readwrite("kinf_C", &TernaryChemicalActivation::kinf_C)
      .def_readwrite("Fc", &TernaryChemicalActivation::Fc)
      .def_readwrite("N", &TernaryChemicalActivation::N)
      .def_readwrite("reactants", &TernaryChemicalActivation::reactants)
      .def_readwrite("products", &TernaryChemicalActivation::products)
      .def_readwrite("name", &TernaryChemicalActivation::name)
      .def_readwrite("gas_phase", &TernaryChemicalActivation::gas_phase)
      .def_readwrite("other_properties", &TernaryChemicalActivation::unknown_properties)
      .def("__str__", [](const TernaryChemicalActivation &t) { return t.name; })
      .def("__repr__", [](const TernaryChemicalActivation &t) { return "<TernaryChemicalActivation: " + t.name + ">"; })
      .def_property_readonly(
          "type", [](const TernaryChemicalActivation &) { return ReactionType::TernaryChemicalActivation; });

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
      .def(py::init([](const py::list &reactions) { return create_reactions(reactions); }))
      .def_readwrite("arrhenius", &Reactions::arrhenius)
      .def_readwrite("branched", &Reactions::branched)
      .def_readwrite("emission", &Reactions::emission)
      .def_readwrite("first_order_loss", &Reactions::first_order_loss)
      .def_readwrite("photolysis", &Reactions::photolysis)
      .def_readwrite("surface", &Reactions::surface)
      .def_readwrite("taylor_series", &Reactions::taylor_series)
      .def_readwrite("ternary_chemical_activation", &Reactions::ternary_chemical_activation)
      .def_readwrite("troe", &Reactions::troe)
      .def_readwrite("tunneling", &Reactions::tunneling)
      .def_readwrite("user_defined", &Reactions::user_defined)
      .def(
          "__len__",
          [](const Reactions &r)
          {
            return r.arrhenius.size() + r.branched.size() + r.emission.size() + r.first_order_loss.size() +
                   r.photolysis.size() + r.surface.size() + r.taylor_series.size() + r.troe.size() +
                   r.ternary_chemical_activation.size() + r.tunneling.size() + r.user_defined.size();
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
          })
      .def(
          "parse_and_convert_v0",
          [](V1Parser &self, const std::string &path)
          {
            mechanism_configuration::v1::types::Mechanism mechanism = musica::ConvertV0MechanismToV1(path);
            return mechanism;
          },
          "Parse a v0 mechanism configuration file");
}
