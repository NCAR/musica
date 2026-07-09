#include <musica/configuration/parse.hpp>
#include <musica/configuration/read_mechanism.hpp>
#include <musica/micm/lambda_callback.hpp>
#include <musica/utils/error_code.hpp>

#include <mechanism_configuration/parse.hpp>
#include <mechanism_configuration/types/reactions.hpp>
#include <mechanism_configuration/types/species.hpp>
#include <micm/Process.hpp>
#include <micm/System.hpp>
#include <micm/process/rate_constant/lambda_rate_constant.hpp>

#include <sstream>

using namespace mechanism_configuration;
// used to come from mechanism configuration's validation, but that is now a private header
inline constexpr std::string_view molecular_weight = "molecular weight [kg mol-1]";

namespace musica
{
  Chemistry ReadConfiguration(const std::string& config_path)
  {
    return ConvertMechanism(ReadMechanism(config_path));
  }

  Chemistry ReadConfigurationFromString(const std::string& json_or_yaml_string)
  {
    return ConvertMechanism(ReadMechanismFromString(json_or_yaml_string));
  }

  bool IsBool(const std::string& value)
  {
    return (value == "true" || value == "false");
  }

  bool IsInt(const std::string& value)
  {
    std::istringstream iss(value);
    int result;
    return (iss >> result >> std::ws).eof() && !value.empty();
  }

  bool IsFloatingPoint(const std::string& value)
  {
    std::istringstream iss(value);
    double result;
    return (iss >> result >> std::ws).eof() && !value.empty();
  }

  std::vector<micm::Species> convert_species(const std::vector<types::Species>& species)
  {
    using namespace mechanism_configuration;
    micm::Phase const gas_phase;
    std::vector<micm::Species> micm_species;
    for (const auto& elem : species)
    {
      micm::Species s;
      s.name_ = elem.name;

      if (elem.molecular_weight.has_value())
      {
        s.SetProperty(std::string(molecular_weight), elem.molecular_weight.value());
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
      if (elem.tracer_type.has_value())
      {
        if (elem.tracer_type.value() == "THIRD_BODY")
        {
          s.SetThirdBody();
        }
        else
        {
          s.SetProperty("tracer type", elem.tracer_type.value());
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

  std::vector<micm::Phase> convert_phases(
      const std::vector<types::Phase>& phases,
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
      const std::vector<types::ReactionComponent>& components,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    std::vector<micm::Species> species;
    for (const auto& component : components)
    {
      for (int i = 0; i < component.coefficient; i++)
      {
        species.push_back(species_map[component.name]);
      }
    }
    return species;
  }

  std::vector<micm::Species> reaction_components_to_reactants(
      const types::ReactionComponent& component,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    return reaction_components_to_reactants(std::vector<types::ReactionComponent>{ component }, species_map);
  }

  std::vector<micm::StoichSpecies> reaction_components_to_products(
      const std::vector<types::ReactionComponent>& components,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    std::vector<micm::StoichSpecies> yields;
    for (const auto& component : components)
    {
      yields.push_back({ species_map[component.name], component.coefficient });
    }
    return yields;
  }

  void convert_arrhenius(
      Chemistry& chemistry,
      const std::vector<types::Arrhenius>& arrhenius,
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
                                        .SetRateConstant(parameters)
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());
    }
  }

  void convert_branched(
      Chemistry& chemistry,
      const std::vector<types::Branched>& branched,
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
                                        .SetRateConstant(parameters)
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());

      // Nitrate branch
      parameters.branch_ = micm::BranchedRateConstantParameters::Branch::Nitrate;
      chemistry.processes.push_back(micm::ChemicalReactionBuilder()
                                        .SetReactants(reactants)
                                        .SetProducts(nitrate_products)
                                        .SetRateConstant(parameters)
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());
    }
  }

  void convert_surface(
      Chemistry& chemistry,
      const std::vector<types::Surface>& surface,
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
          [&reaction](const micm::PhaseSpecies& ps) { return ps.species_.name_ == reaction.gas_phase_species.name; });

      if (it == phase_species_list.end())
      {
        throw musica::Exception(
            musica::ParseErrorCode::ParsingFailed,
            "Species '" + reaction.gas_phase_species.name + "' for surface reaction in gas phase is not found\n");
      }

      size_t const surface_reaction_species_index = std::distance(phase_species_list.begin(), it);
      micm::SurfaceRateConstantParameters const parameters{ .label_ = prefix + reaction.name,
                                                            .phase_species_ =
                                                                phase_species_list[surface_reaction_species_index],
                                                            .reaction_probability_ = reaction.reaction_probability };

      chemistry.processes.push_back(micm::ChemicalReactionBuilder()
                                        .SetReactants(reactants)
                                        .SetProducts(products)
                                        .SetRateConstant(parameters)
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());
    }
  }

  void convert_troe(
      Chemistry& chemistry,
      const std::vector<types::Troe>& troe,
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
                                        .SetRateConstant(parameters)
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());
    }
  }

  void convert_ternary_chemical_activation(
      Chemistry& chemistry,
      const std::vector<types::TernaryChemicalActivation>& ternary,
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
                                        .SetRateConstant(parameters)
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());
    }
  }

  void convert_tunneling(
      Chemistry& chemistry,
      const std::vector<types::Tunneling>& tunneling,
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
                                        .SetRateConstant(parameters)
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());
    }
  }

  void convert_taylor_series(
      Chemistry& chemistry,
      const std::vector<types::TaylorSeries>& taylor_series,
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
      if (reaction.taylor_coefficients.size() > micm::TaylorSeriesRateConstantParameters::MAX_COEFFICIENTS)
      {
        throw musica::Exception(
            musica::ParseErrorCode::ParsingFailed,
            "Number of Taylor series coefficients for reaction '" + reaction.name + "' exceeds the maximum supported (" +
                std::to_string(micm::TaylorSeriesRateConstantParameters::MAX_COEFFICIENTS) + ").");
      }
      std::copy(reaction.taylor_coefficients.begin(), reaction.taylor_coefficients.end(), parameters.coefficients_);
      chemistry.processes.push_back(micm::ChemicalReactionBuilder()
                                        .SetReactants(reactants)
                                        .SetProducts(products)
                                        .SetRateConstant(parameters)
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

  void convert_lambda_rate_constants(
      Chemistry& chemistry,
      const std::vector<types::LambdaRateConstant>& reactions,
      std::unordered_map<std::string, micm::Species>& species_map)
  {
    for (const auto& reaction : reactions)
    {
      const std::string label = "Lambda." + reaction.name;
      auto reactants = reaction_components_to_reactants(reaction.reactants, species_map);
      auto products = reaction_components_to_products(reaction.products, species_map);

      // The lambda_function field stores a raw string from the config (which
      // may be a C++ lambda expression or a JS placeholder).  At runtime MICM
      // never evaluates that string; instead it calls InvokeLambdaCallback
      // which dispatches to whichever JS function was registered via
      // SetLambdaRateCallback.
      micm::LambdaRateConstantParameters params;
      params.label_ = label;
      params.lambda_function_ = [label](const micm::Conditions& c) { return musica::InvokeLambdaCallback(label, c); };

      chemistry.processes.push_back(micm::ChemicalReactionBuilder()
                                        .SetReactants(reactants)
                                        .SetProducts(products)
                                        .SetRateConstant(params)
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());
    }
  }

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
      std::vector<micm::StoichSpecies> products{};

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
                                        .SetRateConstant(parameters)
                                        .SetPhase(chemistry.system.gas_phase_)
                                        .Build());
    }
  }

  Chemistry ConvertMechanism(const Mechanism& mechanism)
  {
    Chemistry chemistry{};
    auto species = convert_species(mechanism.species);
    std::unordered_map<std::string, micm::Species> species_map;
    for (const auto& species : species)
    {
      species_map[species.name_] = species;
    }
    auto phases = convert_phases(mechanism.phases, species_map);
    micm::Phase& gas_phase = chemistry.system.gas_phase_;
    for (const auto& phase : phases)
    {
      if (phase.name_ == "gas")
      {
        gas_phase = phase;
      }
    }
    convert_arrhenius(chemistry, mechanism.reactions.arrhenius, species_map);
    convert_branched(chemistry, mechanism.reactions.branched, species_map);
    convert_surface(chemistry, mechanism.reactions.surface, species_map, gas_phase, "SURF.");
    convert_taylor_series(chemistry, mechanism.reactions.taylor_series, species_map);
    convert_troe(chemistry, mechanism.reactions.troe, species_map);
    convert_ternary_chemical_activation(chemistry, mechanism.reactions.ternary_chemical_activation, species_map);
    convert_tunneling(chemistry, mechanism.reactions.tunneling, species_map);
    convert_user_defined(chemistry, mechanism.reactions.photolysis, species_map, "PHOTO.");
    convert_user_defined(chemistry, mechanism.reactions.emission, species_map, "EMIS.");
    convert_user_defined(chemistry, mechanism.reactions.first_order_loss, species_map, "LOSS.");
    convert_user_defined(chemistry, mechanism.reactions.user_defined, species_map, "USER.");
    convert_lambda_rate_constants(chemistry, mechanism.reactions.lambda_rate_constant, species_map);
    return chemistry;
  }

}  // namespace musica