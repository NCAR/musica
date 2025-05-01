#include <musica/micm/parse.hpp>

#include <micm/process/arrhenius_rate_constant.hpp>
#include <micm/process/branched_rate_constant.hpp>
#include <micm/process/process.hpp>
#include <micm/process/surface_rate_constant.hpp>
#include <micm/process/ternary_chemical_activation_rate_constant.hpp>
#include <micm/process/troe_rate_constant.hpp>
#include <micm/process/tunneling_rate_constant.hpp>
#include <micm/process/user_defined_rate_constant.hpp>
#include <micm/system/phase.hpp>
#include <micm/system/species.hpp>

#include <mechanism_configuration/v1/types.hpp>
#include <mechanism_configuration/v1/validation.hpp>

namespace musica
{
  std::vector<micm::Species> convert_species(const std::vector<mechanism_configuration::v1::types::Species>& species)
  {
    using namespace mechanism_configuration::v1;
    micm::Phase gas_phase;
    std::vector<micm::Species> micm_species;
    for (const auto& elem : species)
    {
      micm::Species s;
      s.name_ = elem.name;

      if (elem.molecular_weight.has_value())
      {
        s.SetProperty(validation::molecular_weight, elem.molecular_weight.value());
      }
      if (elem.diffusion_coefficient.has_value())
      {
        s.SetProperty(validation::diffusion_coefficient, elem.diffusion_coefficient.value());
      }
      if (elem.absolute_tolerance.has_value())
      {
        s.SetProperty(validation::absolute_tolerance, elem.absolute_tolerance.value());
      }
      if (elem.tracer_type.has_value())
      {
        s.SetProperty(validation::tracer_type, elem.tracer_type.value());
        if (elem.tracer_type == validation::third_body)
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

      micm_species.push_back(s);
    }

    return micm_species;
  }

  std::vector<micm::Species> collect_species(
      std::vector<std::string> species_names,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    std::vector<micm::Species> species;
    for (const auto& species_name : species_names)
    {
      species.push_back(species_map[species_name]);
    }
    return species;
  }

  std::vector<micm::Phase> convert_phases(
      const std::vector<mechanism_configuration::v1::types::Phase>& phases,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    std::vector<micm::Phase> micm_phases;
    for (const auto& phase : phases)
    {
      micm::Phase micm_phase;
      micm_phase.name_ = phase.name;
      micm_phase.species_ = collect_species(phase.species, species_map);
      micm_phases.push_back(micm_phase);
    }
    return micm_phases;
  }

  std::vector<micm::Species> reaction_components_to_reactants(
      const std::vector<mechanism_configuration::v1::types::ReactionComponent>& components,
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
      const std::vector<mechanism_configuration::v1::types::ReactionComponent>& components,
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
      const std::vector<mechanism_configuration::v1::types::Arrhenius>& arrhenius,
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
      const std::vector<mechanism_configuration::v1::types::Branched>& branched,
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

  void convert_surface(
      Chemistry& chemistry,
      const std::vector<mechanism_configuration::v1::types::Surface>& surface,
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
      const std::vector<mechanism_configuration::v1::types::Troe>& troe,
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

  void convert_tunneling(
      Chemistry& chemistry,
      const std::vector<mechanism_configuration::v1::types::Tunneling>& tunneling,
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

  // Helper traits to detect the presence of reactants and products members
  template<typename T, typename = void>
  struct has_reactants : std::false_type
  {
  };

  template<typename T>
  struct has_reactants<T, std::void_t<decltype(std::declval<T>().reactants)>> : std::true_type
  {
  };

  template<typename T, typename = void>
  struct has_products : std::false_type
  {
  };

  template<typename T>
  struct has_products<T, std::void_t<decltype(std::declval<T>().products)>> : std::true_type
  {
  };

  template<typename T>
  void convert_user_defined(
      Chemistry& chemistry,
      const std::vector<T>& user_defined,
      std::unordered_map<std::string, micm::Species>& species_map,
      std::string prefix = "")
  {
    for (const auto& reaction : user_defined)
    {
      std::vector<micm::Species> reactants{};
      std::vector<std::pair<micm::Species, double>> products{};

      if constexpr (has_reactants<T>::value)
      {
        reactants = reaction_components_to_reactants(reaction.reactants, species_map);
      }
      if constexpr (has_products<T>::value)
      {
        products = reaction_components_to_products(reaction.products, species_map);
      }

      micm::UserDefinedRateConstantParameters parameters;
      parameters.scaling_factor_ = reaction.scaling_factor;
      parameters.label_ = prefix + reaction.name;
      chemistry.processes.push_back(micm::Process(
          reactants, products, std::make_unique<micm::UserDefinedRateConstant>(parameters), chemistry.system.gas_phase_));
    }
  }

  Chemistry ConvertV1Mechanism(const mechanism_configuration::v1::types::Mechanism& v1_mechanism)
  {
    Chemistry chemistry{};
    auto species = convert_species(v1_mechanism.species);
    std::unordered_map<std::string, micm::Species> species_map;
    for (const auto& species : species)
    {
      species_map[species.name_] = species;
    }
    auto phases = convert_phases(v1_mechanism.phases, species_map);
    micm::Phase& gas_phase = chemistry.system.gas_phase_;
    for (const auto& phase : phases)
    {
      if (phase.name_ == "gas")
      {
        gas_phase = phase;
      }
      else
      {
        chemistry.system.phases_[phase.name_] = phase;
      }
    }
    convert_arrhenius(chemistry, v1_mechanism.reactions.arrhenius, species_map);
    convert_branched(chemistry, v1_mechanism.reactions.branched, species_map);
    convert_surface(chemistry, v1_mechanism.reactions.surface, species_map);
    convert_troe(chemistry, v1_mechanism.reactions.troe, species_map);
    convert_tunneling(chemistry, v1_mechanism.reactions.tunneling, species_map);
    convert_user_defined(chemistry, v1_mechanism.reactions.photolysis, species_map, "PHOTO.");
    convert_user_defined(chemistry, v1_mechanism.reactions.emission, species_map, "EMIS.");
    convert_user_defined(chemistry, v1_mechanism.reactions.first_order_loss, species_map, "LOSS.");
    convert_user_defined(chemistry, v1_mechanism.reactions.user_defined, species_map, "USER.");
    return chemistry;
  }

  Chemistry ParserV1(const mechanism_configuration::ParserResult<>& result)
  {
    using V1 = mechanism_configuration::v1::types::Mechanism;
    V1* v1_mechanism = dynamic_cast<V1*>(result.mechanism.get());
    if (!v1_mechanism)
      throw std::system_error(make_error_code(MusicaParseErrc::FailedToCastToVersion), "Failed to cast to V1");
    return ConvertV1Mechanism(*v1_mechanism);
  }

}  // namespace musica
