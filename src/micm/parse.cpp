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

#include <mechanism_configuration/v0/types.hpp>
#include <mechanism_configuration/v0/validation.hpp>

namespace musica
{
  namespace v0
  {
    void convert_species(Chemistry& chemistry, const std::vector<mechanism_configuration::v0::types::Species>& species)
    {
      using namespace mechanism_configuration::v0;
      micm::Phase gas_phase;
      for (const auto& elem : species)
      {
        micm::Species s;
        s.name_ = elem.name;
        s.SetProperty(validation::MOL_WEIGHT, elem.molecular_weight);
        s.SetProperty(validation::DIFFUSION_COEFF, elem.diffusion_coefficient);
        s.SetProperty(validation::THIRD_BODY, elem.third_body);
        s.SetProperty(validation::ABS_TOLERANCE, elem.absolute_tolerance);
        s.SetProperty(validation::TRACER_TYPE, elem.tracer_type);
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
            reactants, alkoxy_products, std::make_unique<micm::BranchedRateConstant>(parameters), chemistry.system.gas_phase_));

        // Nitrate branch
        parameters.branch_ = micm::BranchedRateConstantParameters::Branch::Nitrate;
        chemistry.processes.push_back(micm::Process(
            reactants, nitrate_products, std::make_unique<micm::BranchedRateConstant>(parameters), chemistry.system.gas_phase_));
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

    void convert_processes(
        Chemistry& chemistry,
        const mechanism_configuration::v0::types::Mechanism& mechanism,
        std::unordered_map<std::string, micm::Species>& species_map)
    {
      convert_arrhenius(chemistry, mechanism.reactions.arrhenius, species_map);
      convert_branched(chemistry, mechanism.reactions.branched, species_map);
    }
  }  // namespace v0

  Chemistry ParserV0(const mechanism_configuration::ParserResult<>& result, Error* error)
  {
    using V0 = mechanism_configuration::v0::types::Mechanism;
    V0* v0_mechanism = dynamic_cast<V0*>(result.mechanism.get());
    Chemistry chemistry{};
    if (!v0_mechanism)
    {
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_CONFIG_PARSE_FAILED, "Failed to cast to V0");
    }
    else
    {
      v0::convert_species(chemistry, v0_mechanism->species);
      std::unordered_map<std::string, micm::Species> species_map;
      for (const auto& species : chemistry.system.gas_phase_.species_)
      {
        species_map[species.name_] = species;
      }
      v0::convert_processes(chemistry, *v0_mechanism, species_map);
    }

    return chemistry;
  }

  Chemistry ParserV1(const mechanism_configuration::ParserResult<>& result, Error* error)
  {
    return Chemistry{};
  }

}  // namespace musica