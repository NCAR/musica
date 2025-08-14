
#include <musica/micm/parse.hpp>

#include <mechanism_configuration/v0/types.hpp>
#include <mechanism_configuration/v0/validation.hpp>
#include <mechanism_configuration/v1/types.hpp>
#include <mechanism_configuration/v1/validation.hpp>

static constexpr double avogadro = 6.02214076e23;  // # mol^{-1}
static constexpr double MolesM3ToMoleculesCm3 = 1.0e-6 * avogadro;
static constexpr double MoleculesCm3ToMolesM3 = 1.0 / MolesM3ToMoleculesCm3;

namespace musica
{
  double convert_molecules_cm3_to_moles_m3(
      std::vector<mechanism_configuration::v1::types::ReactionComponent> reactants,
      double molecules_cm3)
  {
    // This converts preexponential factors that were calculated for molec cm-3 units and
    // converts them to SI units of mol m-3 for species concentrations
    // (molec cm-3)^-(N-1) s-1 --> (mol m-3)^-(N-1) s-1
    // N is the number of reactants
    int total_reactants = 0;
    for (const auto& reactant : reactants)
    {
      total_reactants += static_cast<int>(reactant.coefficient);
    }

    // The total reactants ensures that the rate always ends up in moles m-3 s-1
    return molecules_cm3 * std::pow(MoleculesCm3ToMolesM3, -(total_reactants - 1));
  }

  double k0_A_convert_molecules_cm3_to_moles_m3(
      std::vector<mechanism_configuration::v1::types::ReactionComponent> reactants,
      double molecules_cm3)
  {
    // This is special to the Troe reactions because M is included in the rate
    // This converts preexponential factors that were calculated for molec cm-3 units and
    // converts them to SI units of mol m-3 for species concentrations
    // (molec cm-3)^-N s-1 --> (mol m-3)^-N s-1
    // N is the number of reactants
    int total_reactants = 0;
    for (const auto& reactant : reactants)
    {
      total_reactants += static_cast<int>(reactant.coefficient);
    }

    // The total reactants ensures that the rate always ends up in moles m-3 s-1
    return molecules_cm3 * std::pow(MoleculesCm3ToMolesM3, -(total_reactants));
  }

  // Forward declarations
  std::vector<mechanism_configuration::v1::types::Species> convert_species_v0_to_v1(
      const std::vector<mechanism_configuration::v0::types::Species>& v0_species);

  mechanism_configuration::v1::types::Reactions convert_reactions_v0_to_v1(
      const mechanism_configuration::v0::types::Reactions& v0_reactions);

  std::vector<mechanism_configuration::v1::types::ReactionComponent> convert_reaction_components_v0_to_v1(
      const std::vector<mechanism_configuration::v0::types::ReactionComponent>& v0_components);

  mechanism_configuration::v1::types::ReactionComponent convert_reaction_component_v0_to_v1(
      const mechanism_configuration::v0::types::ReactionComponent& v0_component);

  mechanism_configuration::v1::types::Mechanism ConvertV0MechanismToV1(const std::string& config_path)
  {
    mechanism_configuration::v0::Parser parser;
    auto parsed = parser.Parse(config_path);
    if (!parsed)
    {
      throw std::system_error(make_error_code(MusicaParseErrc::ParsingFailed), "Failed to parse V0 mechanism configuration");
    }
    mechanism_configuration::v0::types::Mechanism mechanism = *parsed;
    return ConvertV0MechanismToV1(mechanism);
  }

  mechanism_configuration::v1::types::Mechanism ConvertV0MechanismToV1(
      const mechanism_configuration::v0::types::Mechanism& v0_mechanism)
  {
    mechanism_configuration::v1::types::Mechanism v1_mechanism;

    // Convert species
    v1_mechanism.species = convert_species_v0_to_v1(v0_mechanism.species);

    // Create a single gas phase containing all species (v0 doesn't have explicit phases)
    mechanism_configuration::v1::types::Phase gas_phase;
    gas_phase.name = "gas";
    for (const auto& species : v1_mechanism.species)
    {
      gas_phase.species.push_back(species.name);
    }
    v1_mechanism.phases.push_back(gas_phase);

    mechanism_configuration::v1::types::Phase condensed_phase;
    condensed_phase.name = "condensed";
    for (const auto& species : v1_mechanism.species)
    {
      condensed_phase.species.push_back(species.name);
    }
    v1_mechanism.phases.push_back(condensed_phase);

    // Convert reactions
    v1_mechanism.reactions = convert_reactions_v0_to_v1(v0_mechanism.reactions);

    // Set mechanism name if available (assuming v0 has a name field, otherwise use default)
    v1_mechanism.name = v0_mechanism.name;

    // Set version to v1
    v1_mechanism.version.major = 1;
    v1_mechanism.version.minor = 0;
    v1_mechanism.version.patch = 0;

    return v1_mechanism;
  }

  std::vector<mechanism_configuration::v1::types::Species> convert_species_v0_to_v1(
      const std::vector<mechanism_configuration::v0::types::Species>& v0_species)
  {
    std::vector<mechanism_configuration::v1::types::Species> v1_species;

    for (const auto& v0_spec : v0_species)
    {
      mechanism_configuration::v1::types::Species v1_spec;

      v1_spec.name = v0_spec.name;
      v1_spec.molecular_weight = v0_spec.molecular_weight;
      v1_spec.diffusion_coefficient = v0_spec.diffusion_coefficient;
      v1_spec.absolute_tolerance = v0_spec.absolute_tolerance;
      v1_spec.tracer_type = v0_spec.tracer_type;

      // Convert unknown properties
      for (const auto& prop : v0_spec.unknown_properties)
      {
        v1_spec.unknown_properties[prop.first] = prop.second;
      }

      v1_species.push_back(v1_spec);
    }

    return v1_species;
  }

  mechanism_configuration::v1::types::Reactions convert_reactions_v0_to_v1(
      const mechanism_configuration::v0::types::Reactions& v0_reactions)
  {
    mechanism_configuration::v1::types::Reactions v1_reactions;

    // Convert arrhenius reactions
    for (const auto& arr : v0_reactions.arrhenius)
    {
      mechanism_configuration::v1::types::Arrhenius v1_arr;
      v1_arr.reactants = convert_reaction_components_v0_to_v1(arr.reactants);
      v1_arr.products = convert_reaction_components_v0_to_v1(arr.products);
      v1_arr.A = convert_molecules_cm3_to_moles_m3(v1_arr.reactants, arr.A);
      v1_arr.B = arr.B;
      v1_arr.C = arr.C;
      v1_arr.D = arr.D;
      v1_arr.E = arr.E;
      v1_arr.gas_phase = "gas";
      v1_arr.unknown_properties = arr.unknown_properties;
      v1_reactions.arrhenius.push_back(v1_arr);
    }

    // Convert branched reactions
    for (const auto& branched : v0_reactions.branched)
    {
      mechanism_configuration::v1::types::Branched v1_branched;
      v1_branched.reactants = convert_reaction_components_v0_to_v1(branched.reactants);
      v1_branched.alkoxy_products = convert_reaction_components_v0_to_v1(branched.alkoxy_products);
      v1_branched.nitrate_products = convert_reaction_components_v0_to_v1(branched.nitrate_products);
      v1_branched.X = convert_molecules_cm3_to_moles_m3(v1_branched.reactants, branched.X);
      v1_branched.Y = branched.Y;
      v1_branched.a0 = branched.a0;
      v1_branched.n = branched.n;
      v1_branched.gas_phase = "gas";
      v1_branched.unknown_properties = branched.unknown_properties;
      v1_reactions.branched.push_back(v1_branched);
    }

    // Convert surface reactions
    for (const auto& surface : v0_reactions.surface)
    {
      mechanism_configuration::v1::types::Surface v1_surface;
      v1_surface.name = surface.name;
      v1_surface.reaction_probability = surface.reaction_probability;
      v1_surface.gas_phase_species = convert_reaction_component_v0_to_v1(surface.gas_phase_species);
      v1_surface.gas_phase_products = convert_reaction_components_v0_to_v1(surface.gas_phase_products);
      v1_surface.gas_phase = "gas";
      v1_surface.condensed_phase = "condensed";
      v1_surface.unknown_properties = surface.unknown_properties;
      v1_reactions.surface.push_back(v1_surface);
    }

    // Convert troe reactions
    for (const auto& troe : v0_reactions.troe)
    {
      mechanism_configuration::v1::types::Troe v1_troe;
      v1_troe.reactants = convert_reaction_components_v0_to_v1(troe.reactants);
      v1_troe.products = convert_reaction_components_v0_to_v1(troe.products);
      v1_troe.k0_A = k0_A_convert_molecules_cm3_to_moles_m3(v1_troe.reactants, troe.k0_A);
      v1_troe.kinf_A = convert_molecules_cm3_to_moles_m3(v1_troe.reactants, troe.kinf_A);
      v1_troe.k0_B = troe.k0_B;
      v1_troe.k0_C = troe.k0_C;
      v1_troe.kinf_B = troe.kinf_B;
      v1_troe.kinf_C = troe.kinf_C;
      v1_troe.Fc = troe.Fc;
      v1_troe.N = troe.N;
      v1_troe.gas_phase = "gas";
      v1_troe.unknown_properties = troe.unknown_properties;
      v1_reactions.troe.push_back(v1_troe);
    }

    // Convert ternary chemical activation reactions
    for (const auto& ternary : v0_reactions.ternary_chemical_activation)
    {
      mechanism_configuration::v1::types::TernaryChemicalActivation v1_ternary;
      v1_ternary.reactants = convert_reaction_components_v0_to_v1(ternary.reactants);
      v1_ternary.products = convert_reaction_components_v0_to_v1(ternary.products);
      v1_ternary.k0_A = convert_molecules_cm3_to_moles_m3(v1_ternary.reactants, ternary.k0_A);
      v1_ternary.kinf_A = convert_molecules_cm3_to_moles_m3(v1_ternary.reactants, ternary.kinf_A);
      v1_ternary.k0_B = ternary.k0_B;
      v1_ternary.k0_C = ternary.k0_C;
      v1_ternary.kinf_B = ternary.kinf_B;
      v1_ternary.kinf_C = ternary.kinf_C;
      v1_ternary.Fc = ternary.Fc;
      v1_ternary.N = ternary.N;
      v1_ternary.gas_phase = "gas";
      v1_ternary.unknown_properties = ternary.unknown_properties;
      v1_reactions.ternary_chemical_activation.push_back(v1_ternary);
    }

    // Convert tunneling reactions
    for (const auto& tunneling : v0_reactions.tunneling)
    {
      mechanism_configuration::v1::types::Tunneling v1_tunneling;
      v1_tunneling.reactants = convert_reaction_components_v0_to_v1(tunneling.reactants);
      v1_tunneling.products = convert_reaction_components_v0_to_v1(tunneling.products);
      v1_tunneling.A = convert_molecules_cm3_to_moles_m3(v1_tunneling.reactants, tunneling.A);
      v1_tunneling.B = tunneling.B;
      v1_tunneling.C = tunneling.C;
      v1_tunneling.gas_phase = "gas";
      v1_tunneling.unknown_properties = tunneling.unknown_properties;
      v1_reactions.tunneling.push_back(v1_tunneling);
    }

    // Convert user defined reactions - in v1, these remain as user_defined
    for (const auto& user_def : v0_reactions.user_defined)
    {
      mechanism_configuration::v1::types::UserDefined v1_user_def;
      v1_user_def.name = user_def.name;
      v1_user_def.scaling_factor = user_def.scaling_factor;
      v1_user_def.reactants = convert_reaction_components_v0_to_v1(user_def.reactants);
      v1_user_def.products = convert_reaction_components_v0_to_v1(user_def.products);
      v1_user_def.gas_phase = "gas";
      v1_user_def.unknown_properties = user_def.unknown_properties;
      v1_reactions.user_defined.push_back(v1_user_def);
    }

    return v1_reactions;
  }

  std::vector<mechanism_configuration::v1::types::ReactionComponent> convert_reaction_components_v0_to_v1(
      const std::vector<mechanism_configuration::v0::types::ReactionComponent>& v0_components)
  {
    std::vector<mechanism_configuration::v1::types::ReactionComponent> v1_components;

    for (const auto& v0_comp : v0_components)
    {
      mechanism_configuration::v1::types::ReactionComponent v1_comp;
      v1_comp.species_name = v0_comp.species_name;
      v1_comp.coefficient = v0_comp.coefficient;
      v1_comp.unknown_properties = v0_comp.unknown_properties;
      v1_components.push_back(v1_comp);
    }

    return v1_components;
  }

  mechanism_configuration::v1::types::ReactionComponent convert_reaction_component_v0_to_v1(
      const mechanism_configuration::v0::types::ReactionComponent& v0_component)
  {
    mechanism_configuration::v1::types::ReactionComponent v1_component;
    v1_component.species_name = v0_component.species_name;
    v1_component.coefficient = v0_component.coefficient;
    v1_component.unknown_properties = v0_component.unknown_properties;

    return v1_component;
  }
}  // namespace musica