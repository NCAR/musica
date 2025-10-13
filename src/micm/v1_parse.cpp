#include <musica/micm/parse.hpp>

#include <micm/Process.hpp>
#include <micm/System.hpp>

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
      if (elem.constant_concentration.has_value())
      {
        auto constant_concentration = elem.constant_concentration.value();
        s.parameterize_ = [constant_concentration](const micm::Conditions& c) { return constant_concentration; };
      }
      if (elem.constant_mixing_ratio.has_value())
      {
        auto constant_mixing_ratio = elem.constant_mixing_ratio.value();
        s.parameterize_ = [constant_mixing_ratio](const micm::Conditions& c)
        { return c.air_density_ * constant_mixing_ratio; };
      }
      if (elem.is_third_body.value_or(false))
      {
        s.SetThirdBody();
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

  std::vector<micm::Phase> convert_phases(
      const std::vector<mechanism_configuration::v1::types::Phase>& phases,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    std::vector<micm::Phase> micm_phases;
    for (const auto& phase : phases)
    {
      std::vector<micm::PhaseSpecies> phase_species_list;

      for (const auto& phase_species : phase.species)
      {
        micm::PhaseSpecies micm_phase_species(species_map[phase_species.name]);

        if (phase_species.diffusion_coefficient.has_value())
        {
          micm_phase_species.SetDiffusionCoefficient(phase_species.diffusion_coefficient.value());
        }
        phase_species_list.emplace_back(micm_phase_species);
      }

      micm_phases.emplace_back(micm::Phase(phase.name, phase_species_list));
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

  std::vector<micm::Yield> reaction_components_to_products(
      const std::vector<mechanism_configuration::v1::types::ReactionComponent>& components,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    std::vector<micm::Yield> yields;
    for (const auto& component : components)
    {
      yields.push_back({ species_map[component.species_name], component.coefficient });
    }
    return yields;
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
      auto reactants = reaction_components_to_reactants(reaction.reactants, species_map);
      auto products = reaction_components_to_products(reaction.products, species_map);
      chemistry.processes.push_back(micm::ChemicalReactionBuilder()
                                        .SetReactants(reactants)
                                        .SetProducts(products)
                                        .SetRateConstant(micm::ArrheniusRateConstant(parameters))
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());
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
      chemistry.processes.push_back(micm::ChemicalReactionBuilder()
                                        .SetReactants(reactants)
                                        .SetProducts(alkoxy_products)
                                        .SetRateConstant(micm::BranchedRateConstant(parameters))
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());

      // Nitrate branch
      parameters.branch_ = micm::BranchedRateConstantParameters::Branch::Nitrate;
      chemistry.processes.push_back(micm::ChemicalReactionBuilder()
                                        .SetReactants(reactants)
                                        .SetProducts(nitrate_products)
                                        .SetRateConstant(micm::BranchedRateConstant(parameters))
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());
    }
  }

  void convert_surface(
      Chemistry& chemistry,
      const std::vector<mechanism_configuration::v1::types::Surface>& surface,
      std::unordered_map<std::string, micm::Species>& species_map,
      const micm::Phase& gas_phase,
      const std::string& prefix)
  {
    for (const auto& reaction : surface)
    {
      auto reactants = reaction_components_to_reactants({ reaction.gas_phase_species }, species_map);
      auto products = reaction_components_to_products(reaction.gas_phase_products, species_map);

      auto& phase_species_list = gas_phase.phase_species_;
      auto it = std::find_if(
          phase_species_list.begin(),
          phase_species_list.end(),
          [&reaction](const micm::PhaseSpecies& ps)
          { return ps.species_.name_ == reaction.gas_phase_species.species_name; });

      if (it == phase_species_list.end())
      {
        throw std::system_error(
            make_error_code(MusicaParseErrc::ParsingFailed),
            "Species '" + reaction.gas_phase_species.species_name + "' for surface reaction in gas phase is not found\n");
      }

      size_t surface_reaction_species_index = std::distance(phase_species_list.begin(), it);
      micm::SurfaceRateConstantParameters parameters{ .label_ = prefix + reaction.name,
                                                      .phase_species_ = phase_species_list[surface_reaction_species_index],
                                                      .reaction_probability_ = reaction.reaction_probability };

      chemistry.processes.push_back(micm::ChemicalReactionBuilder()
                                        .SetReactants(reactants)
                                        .SetProducts(products)
                                        .SetRateConstant(micm::SurfaceRateConstant(parameters))
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());
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
      chemistry.processes.push_back(micm::ChemicalReactionBuilder()
                                        .SetReactants(reactants)
                                        .SetProducts(products)
                                        .SetRateConstant(micm::TroeRateConstant(parameters))
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());
    }
  }

  void convert_ternary_chemical_activation(
      Chemistry& chemistry,
      const std::vector<mechanism_configuration::v1::types::TernaryChemicalActivation>& ternary,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    for (const auto& reaction : ternary)
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
      chemistry.processes.push_back(micm::ChemicalReactionBuilder()
                                        .SetReactants(reactants)
                                        .SetProducts(products)
                                        .SetRateConstant(micm::TernaryChemicalActivationRateConstant(parameters))
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());
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
      chemistry.processes.push_back(micm::ChemicalReactionBuilder()
                                        .SetReactants(reactants)
                                        .SetProducts(products)
                                        .SetRateConstant(micm::TunnelingRateConstant(parameters))
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());
    }
  }

  void convert_taylor_series(
      Chemistry& chemistry,
      const std::vector<mechanism_configuration::v1::types::TaylorSeries>& taylor_series,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    for (const auto& reaction : taylor_series)
    {
      auto reactants = reaction_components_to_reactants(reaction.reactants, species_map);
      auto products = reaction_components_to_products(reaction.products, species_map);
      micm::TaylorSeriesRateConstantParameters parameters;
      parameters.A_ = reaction.A;
      parameters.B_ = reaction.B;
      parameters.C_ = reaction.C;
      parameters.D_ = reaction.D;
      parameters.E_ = reaction.E;
      parameters.coefficients_ = reaction.taylor_coefficients;
      chemistry.processes.push_back(micm::ChemicalReactionBuilder()
                                        .SetReactants(reactants)
                                        .SetProducts(products)
                                        .SetRateConstant(micm::TaylorSeriesRateConstant(parameters))
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());
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
      std::vector<micm::Yield> products{};

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
      chemistry.processes.push_back(micm::ChemicalReactionBuilder()
                                        .SetReactants(reactants)
                                        .SetProducts(products)
                                        .SetRateConstant(micm::UserDefinedRateConstant(parameters))
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());
    }
  }

  Chemistry ConvertV1Mechanism(const mechanism_configuration::v1::types::Mechanism& v1_mechanism, bool ignore_non_gas_phases)
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
      else if (!ignore_non_gas_phases)
      {
        chemistry.system.phases_[phase.name_] = phase;
      }
    }
    convert_arrhenius(chemistry, v1_mechanism.reactions.arrhenius, species_map);
    convert_branched(chemistry, v1_mechanism.reactions.branched, species_map);
    convert_surface(chemistry, v1_mechanism.reactions.surface, species_map, gas_phase, "SURF.");
    convert_taylor_series(chemistry, v1_mechanism.reactions.taylor_series, species_map);
    convert_troe(chemistry, v1_mechanism.reactions.troe, species_map);
    convert_ternary_chemical_activation(chemistry, v1_mechanism.reactions.ternary_chemical_activation, species_map);
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
