// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/miem/parse.hpp>
#include <musica/utils/error_code.hpp>

#include <miem/species_map.hpp>

#include <filesystem>
#include <unordered_map>

namespace musica
{
  namespace mc_types = mechanism_configuration::types;

  namespace
  {
    miem::SourceMode ConvertSourceMode(mc_types::SourceMode mode)
    {
      switch (mode)
      {
        case mc_types::SourceMode::Offline: return miem::SourceMode::Offline;
      }
    }

    miem::SourceType ConvertSourceType(mc_types::SourceType type)
    {
      switch (type)
      {
        case mc_types::SourceType::Anthropogenic: return miem::SourceType::Anthropogenic;
        case mc_types::SourceType::Fire: return miem::SourceType::Fire;
        case mc_types::SourceType::Biogenic: return miem::SourceType::Biogenic;
        case mc_types::SourceType::Dust: return miem::SourceType::Dust;
        case mc_types::SourceType::SeaSalt: return miem::SourceType::SeaSalt;
        case mc_types::SourceType::Lightning: return miem::SourceType::Lightning;
      }
    }

    miem::TemporalInterpolation ConvertTemporalInterpolation(mc_types::TemporalInterpolation interpolation)
    {
      switch (interpolation)
      {
        case mc_types::TemporalInterpolation::Linear: return miem::TemporalInterpolation::Linear;
        case mc_types::TemporalInterpolation::Nearest: return miem::TemporalInterpolation::Nearest;
        case mc_types::TemporalInterpolation::None: return miem::TemporalInterpolation::None;
      }
    }

    miem::VerticalInjection ConvertVerticalInjection(mc_types::VerticalInjection injection)
    {
      switch (injection)
      {
        case mc_types::VerticalInjection::Surface: return miem::VerticalInjection::Surface;
      }
    }

    miem::RegriddingType ConvertRegriddingType(mc_types::RegriddingType type)
    {
      switch (type)
      {
        case mc_types::RegriddingType::None: return miem::RegriddingType::None;
      }
    }

    miem::SpeciesMap ConvertSpeciesMap(const mc_types::SpeciesMap& species_map)
    {
      miem::SpeciesMap converted{};
      for (const auto& mapping : species_map.mappings)
      {
        converted.AddMapping(mapping.inventory_species, mapping.mechanism_species, mapping.scaling_factor);
      }
      return converted;
    }

    miem::Source ConvertSource(
        const mc_types::SourceDescriptor& source,
        const std::unordered_map<std::string, const mc_types::Inventory*>& inventories_by_name,
        const std::unordered_map<std::string, const mc_types::SpeciesMap*>& species_maps_by_name)
    {
      auto inventory_it = inventories_by_name.find(source.inventory);
      if (inventory_it == inventories_by_name.end())
      {
        throw musica::Exception(
            musica::MiemErrorCode::UnresolvedReference,
            "Emission source '" + source.name + "' references unknown inventory '" + source.inventory + "'");
      }

      auto species_map_it = species_maps_by_name.find(source.species_map);
      if (species_map_it == species_maps_by_name.end())
      {
        throw musica::Exception(
            musica::MiemErrorCode::UnresolvedReference,
            "Emission source '" + source.name + "' references unknown species map '" + source.species_map + "'");
      }

      const mc_types::Inventory& inventory = *inventory_it->second;

      miem::Source converted{};
      converted.name_ = source.name;
      converted.mode_ = ConvertSourceMode(source.mode);
      converted.type_ = ConvertSourceType(source.type);
      converted.file_pattern_ = (std::filesystem::path(inventory.directory) / inventory.file_pattern).string();
      converted.convention_ = inventory.convention;
      converted.species_map_ = ConvertSpeciesMap(*species_map_it->second);
      converted.temporal_interpolation_ = ConvertTemporalInterpolation(source.temporal_interpolation);
      converted.vertical_injection_ = ConvertVerticalInjection(source.vertical_injection);
      converted.category_ = source.category;
      converted.hierarchy_ = source.hierarchy;
      converted.scaling_factor_ = source.scaling_factor;
      converted.sector_ = source.sector;

      return converted;
    }
  }  // namespace

  Emissions ConvertEmissions(const mechanism_configuration::Mechanism& mechanism)
  {
    if (!mechanism.emissions.has_value())
    {
      throw musica::Exception(musica::MiemErrorCode::MissingEmissionsSection, "Mechanism has no emissions section");
    }

    const mc_types::EmissionsConfig& config = *mechanism.emissions;

    std::unordered_map<std::string, const mc_types::Inventory*> inventories_by_name;
    for (const auto& inventory : config.inventories)
    {
      inventories_by_name.emplace(inventory.name, &inventory);
    }

    std::unordered_map<std::string, const mc_types::SpeciesMap*> species_maps_by_name;
    for (const auto& species_map : config.species_maps)
    {
      species_maps_by_name.emplace(species_map.name, &species_map);
    }

    Emissions emissions{};
    emissions.sources.reserve(config.sources.size());
    for (const auto& source : config.sources)
    {
      emissions.sources.push_back(ConvertSource(source, inventories_by_name, species_maps_by_name));
    }
    emissions.regridding.type_ = ConvertRegriddingType(config.regridding.type);

    return emissions;
  }
}  // namespace musica
