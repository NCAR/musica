#include <musica/micm/parse.hpp>

#include <micm/Process.hpp>
#include <micm/System.hpp>

#include <mechanism_configuration/v0/types.hpp>
#include <mechanism_configuration/v0/validation.hpp>

namespace musica
{
  void convert_species(Chemistry& chemistry, const std::vector<mechanism_configuration::v0::types::Species>& species)
  {
    using namespace mechanism_configuration::v0;
    micm::Phase gas_phase;
    for (const auto& elem : species)
    {
      micm::Species s;
      s.name_ = elem.name;
      if (elem.molecular_weight.has_value())
      {
        s.SetProperty(validation::MOL_WEIGHT, elem.molecular_weight.value());
      }
      if (elem.diffusion_coefficient.has_value())
      {
        s.SetProperty(validation::DIFFUSION_COEFF, elem.diffusion_coefficient.value());
      }
      if (elem.absolute_tolerance.has_value())
      {
        s.SetProperty(validation::ABS_TOLERANCE, elem.absolute_tolerance.value());
      }
      if (elem.tracer_type.has_value())
      {
        s.SetProperty(validation::TRACER_TYPE, elem.tracer_type.value());
        if (elem.tracer_type == validation::THIRD_BODY)
        {
          s.SetThirdBody();
        }
      }
      for (auto& unknown : elem.unknown_properties)
      {
        if (IsInt(unknown.second))
        {
          s.SetProperty(unknown.first, std::stoi(unknown.second));
        }
        else if (IsFloatingPoint(unknown.second))
        {
          s.SetProperty(unknown.first, std::stod(unknown.second));
        }
        else if (IsBool(unknown.second))
        {
          s.SetProperty(unknown.first, unknown.second == "true");
        }
        else
        {
          s.SetProperty(unknown.first, unknown.second);
        }
      }
      gas_phase.species_.push_back(s);
    }
    chemistry.system.gas_phase_ = gas_phase;
  }

  std::vector<micm::Species> reaction_components_to_reactants(
      const std::vector<mechanism_configuration::v0::types::ReactionComponent>& components,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    std::vector<micm::Species> species;
    for (const auto& component : components)
    {
      for (int i = 0; i < component.coefficient; i++)
      {
        species.push_back(species_map[component.species_name]);
      }
    }
    return species;
  }

  std::vector<std::pair<micm::Species, double>> reaction_components_to_products(
      const std::vector<mechanism_configuration::v0::types::ReactionComponent>& components,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    std::vector<std::pair<micm::Species, double>> species;
    for (const auto& component : components)
    {
      species.push_back({ species_map[component.species_name], component.coefficient });
    }
    return species;
  }

  void convert_arrhenius(
      Chemistry& chemistry,
      const std::vector<mechanism_configuration::v0::types::Arrhenius>& arrhenius,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    for (const auto& reaction : arrhenius)
    {
      micm::ArrheniusRateConstantParameters parameters;
      parameters.A_ = reaction.A;
      parameters.B_ = reaction.B;
      parameters.C_ = reaction.C;
      parameters.D_ = reaction.D;
      parameters.E_ = reaction.E;
      micm::ArrheniusRateConstant rate_constant(parameters);
      auto reactants = reaction_components_to_reactants(reaction.reactants, species_map);
      auto products = reaction_components_to_products(reaction.products, species_map);
      chemistry.processes.push_back(micm::Process(
          reactants, products, std::make_unique<micm::ArrheniusRateConstant>(rate_constant), chemistry.system.gas_phase_));
    }
  }

  void convert_branched(
      Chemistry& chemistry,
      const std::vector<mechanism_configuration::v0::types::Branched>& branched,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    for (const auto& reaction : branched)
    {
      auto reactants = reaction_components_to_reactants(reaction.reactants, species_map);
      auto alkoxy_products = reaction_components_to_products(reaction.alkoxy_products, species_map);
      auto nitrate_products = reaction_components_to_products(reaction.nitrate_products, species_map);

      micm::BranchedRateConstantParameters parameters;
      parameters.X_ = reaction.X;
      parameters.Y_ = reaction.Y;
      parameters.a0_ = reaction.a0;
      parameters.n_ = reaction.n;

      // Alkoxy branch
      parameters.branch_ = micm::BranchedRateConstantParameters::Branch::Alkoxy;
      chemistry.processes.push_back(micm::Process(
          reactants,
          alkoxy_products,
          std::make_unique<micm::BranchedRateConstant>(parameters),
          chemistry.system.gas_phase_));

      // Nitrate branch
      parameters.branch_ = micm::BranchedRateConstantParameters::Branch::Nitrate;
      chemistry.processes.push_back(micm::Process(
          reactants,
          nitrate_products,
          std::make_unique<micm::BranchedRateConstant>(parameters),
          chemistry.system.gas_phase_));
    }
  }

  void convert_user_defined(
      Chemistry& chemistry,
      const std::vector<mechanism_configuration::v0::types::UserDefined>& user_defined,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    for (const auto& reaction : user_defined)
    {
      auto reactants = reaction_components_to_reactants(reaction.reactants, species_map);
      auto products = reaction_components_to_products(reaction.products, species_map);
      micm::UserDefinedRateConstantParameters parameters;
      parameters.scaling_factor_ = reaction.scaling_factor;
      parameters.label_ = reaction.name;
      chemistry.processes.push_back(micm::Process(
          reactants, products, std::make_unique<micm::UserDefinedRateConstant>(parameters), chemistry.system.gas_phase_));
    }
  }

  void convert_surface(
      Chemistry& chemistry,
      const std::vector<mechanism_configuration::v0::types::Surface>& surface,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    for (const auto& reaction : surface)
    {
      auto reactants = reaction_components_to_reactants({ reaction.gas_phase_species }, species_map);
      auto products = reaction_components_to_products(reaction.gas_phase_products, species_map);
      micm::SurfaceRateConstantParameters parameters;
      parameters.reaction_probability_ = reaction.reaction_probability;
      parameters.label_ = reaction.name;
      parameters.species_ = species_map[reaction.gas_phase_species.species_name];
      chemistry.processes.push_back(micm::Process(
          reactants, products, std::make_unique<micm::SurfaceRateConstant>(parameters), chemistry.system.gas_phase_));
    }
  }

  void convert_troe(
      Chemistry& chemistry,
      const std::vector<mechanism_configuration::v0::types::Troe>& troe,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    for (const auto& reaction : troe)
    {
      auto reactants = reaction_components_to_reactants(reaction.reactants, species_map);
      auto products = reaction_components_to_products(reaction.products, species_map);
      micm::TroeRateConstantParameters parameters;
      parameters.k0_A_ = reaction.k0_A;
      parameters.k0_B_ = reaction.k0_B;
      parameters.k0_C_ = reaction.k0_C;
      parameters.kinf_A_ = reaction.kinf_A;
      parameters.kinf_B_ = reaction.kinf_B;
      parameters.kinf_C_ = reaction.kinf_C;
      parameters.Fc_ = reaction.Fc;
      parameters.N_ = reaction.N;
      chemistry.processes.push_back(micm::Process(
          reactants, products, std::make_unique<micm::TroeRateConstant>(parameters), chemistry.system.gas_phase_));
    }
  }

  void convert_ternary_chemical_activation(
      Chemistry& chemistry,
      const std::vector<mechanism_configuration::v0::types::TernaryChemicalActivation>& ternary_chemical_activation,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    for (const auto& reaction : ternary_chemical_activation)
    {
      auto reactants = reaction_components_to_reactants(reaction.reactants, species_map);
      auto products = reaction_components_to_products(reaction.products, species_map);
      micm::TernaryChemicalActivationRateConstantParameters parameters;
      parameters.k0_A_ = reaction.k0_A;
      parameters.k0_B_ = reaction.k0_B;
      parameters.k0_C_ = reaction.k0_C;
      parameters.kinf_A_ = reaction.kinf_A;
      parameters.kinf_B_ = reaction.kinf_B;
      parameters.kinf_C_ = reaction.kinf_C;
      parameters.Fc_ = reaction.Fc;
      parameters.N_ = reaction.N;
      chemistry.processes.push_back(micm::Process(
          reactants,
          products,
          std::make_unique<micm::TernaryChemicalActivationRateConstant>(parameters),
          chemistry.system.gas_phase_));
    }
  }

  void convert_tunneling(
      Chemistry& chemistry,
      const std::vector<mechanism_configuration::v0::types::Tunneling>& tunneling,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    for (const auto& reaction : tunneling)
    {
      auto reactants = reaction_components_to_reactants(reaction.reactants, species_map);
      auto products = reaction_components_to_products(reaction.products, species_map);
      micm::TunnelingRateConstantParameters parameters;
      parameters.A_ = reaction.A;
      parameters.B_ = reaction.B;
      parameters.C_ = reaction.C;
      chemistry.processes.push_back(micm::Process(
          reactants, products, std::make_unique<micm::TunnelingRateConstant>(parameters), chemistry.system.gas_phase_));
    }
  }

  Chemistry ParserV0(const mechanism_configuration::ParserResult<>& result)
  {
    using V0 = mechanism_configuration::v0::types::Mechanism;
    V0* v0_mechanism = dynamic_cast<V0*>(result.mechanism.get());
    Chemistry chemistry{};
    if (!v0_mechanism)
    {
      throw std::system_error(make_error_code(MusicaParseErrc::FailedToCastToVersion), "Failed to cast to V0");
    }
    else
    {
      convert_species(chemistry, v0_mechanism->species);
      std::unordered_map<std::string, micm::Species> species_map;
      for (const auto& species : chemistry.system.gas_phase_.species_)
      {
        species_map[species.name_] = species;
      }
      convert_arrhenius(chemistry, v0_mechanism->reactions.arrhenius, species_map);
      convert_branched(chemistry, v0_mechanism->reactions.branched, species_map);
      convert_user_defined(chemistry, v0_mechanism->reactions.user_defined, species_map);
      convert_surface(chemistry, v0_mechanism->reactions.surface, species_map);
      convert_troe(chemistry, v0_mechanism->reactions.troe, species_map);
      convert_ternary_chemical_activation(chemistry, v0_mechanism->reactions.ternary_chemical_activation, species_map);
      convert_tunneling(chemistry, v0_mechanism->reactions.tunneling, species_map);
    }

    return chemistry;
  }
}  // namespace musica
