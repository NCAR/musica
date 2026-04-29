// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/utils/util.hpp>
#include <musica/version.hpp>

#include <yaml-cpp/yaml.h>

#include <cstddef>
#include <cstring>

namespace musica
{

  void LoadConfigurationFromString(const char* data, Configuration* configuration, Error* error)
  {
    DeleteError(error);
    try
    {
      configuration->data_ = static_cast<void*>(new YAML::Node(YAML::Load(data)));
      NoError(error);
    }
    catch (const std::exception& e)
    {
      configuration->data_ = nullptr;
      ToError(MUSICA_ERROR_CATEGORY, MUSICA_PARSE_ERROR_CODE_PARSING_FAILED, e.what(), MUSICA_SEVERITY_ERROR, error);
    }
  }

  void LoadConfigurationFromFile(const char* filename, Configuration* configuration, Error* error)
  {
    DeleteError(error);
    try
    {
      configuration->data_ = static_cast<void*>(new YAML::Node(YAML::LoadFile(filename)));
      NoError(error);
    }
    catch (const std::exception& e)
    {
      configuration->data_ = nullptr;
      ToError(MUSICA_ERROR_CATEGORY, MUSICA_PARSE_ERROR_CODE_PARSING_FAILED, e.what(), MUSICA_SEVERITY_ERROR, error);
    }
  }

  void DeleteConfiguration(Configuration* config)
  {
    if (config->data_ != nullptr)
      delete static_cast<YAML::Node*>(config->data_);
    config->data_ = nullptr;
  }

  Mapping* AllocateMappingArray(const std::size_t size)
  {
    return new Mapping[size];
  }

  void CreateMappings(std::size_t size, Mappings* mapping)
  {
    mapping->mappings_ = new Mapping[size];
    mapping->size_ = size;
  }

  std::size_t FindMappingIndex(const Mappings mappings, const char* name, Error* error)
  {
    DeleteError(error);
    for (std::size_t i = 0; i < mappings.size_; i++)
    {
      if (std::strcmp(mappings.mappings_[i].name_.value_, name) == 0)
      {
        NoError(error);
        return mappings.mappings_[i].index_;
      }
    }
    std::string const msg = "Mapping element '" + std::string(name) + "' not found";
    ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_MAPPING_NOT_FOUND, msg.c_str(), MUSICA_SEVERITY_ERROR, error);
    return 0;
  }

  void CreateIndexMappings(
      const Configuration configuration,
      const IndexMappingOptions map_options,
      const Mappings source,
      const Mappings target,
      IndexMappings* index_mapping,
      Error* error)
  {
    DeleteError(error);
    if (configuration.data_ == nullptr)
    {
      ToError(
        MUSICA_ERROR_CATEGORY, 
        MUSICA_ERROR_CODE_UNKNOWN, 
        "Invalid configuration", 
        MUSICA_SEVERITY_ERROR, 
        error);
      return;
    }
    YAML::Node* yaml_data = static_cast<YAML::Node*>(configuration.data_);
    std::size_t const size = yaml_data->size();
    std::vector<IndexMapping> mappings;
    index_mapping->size_ = 0;
    if (map_options == IndexMappingOptions::UndefinedMapping)
    {
      ToError(
          MUSICA_ERROR_CATEGORY,
          MUSICA_ERROR_CODE_MAPPING_OPTIONS_UNDEFINED,
          "Mapping options are undefined",
          MUSICA_SEVERITY_ERROR,
          error);
      return;
    }
    for (std::size_t i = 0; i < size; i++)
    {
      const YAML::Node& node = (*yaml_data)[i];
      std::string const source_name = node["source"].as<std::string>();
      std::string const target_name = node["target"].as<std::string>();
      std::size_t const source_index = FindMappingIndex(source, source_name.c_str(), error);
      if (error->code_ == MUSICA_ERROR_CODE_MAPPING_NOT_FOUND)
      {
        if (map_options == IndexMappingOptions::MapAll)
        {
          return;
        }
        else
        {
          DeleteError(error);
          NoError(error);
          continue;
        }
      }
      else if (!IsSuccess(*error))
      {
        return;
      }
      std::size_t const target_index = FindMappingIndex(target, target_name.c_str(), error);
      if (error->code_ == MUSICA_ERROR_CODE_MAPPING_NOT_FOUND)
      {
        if (map_options == IndexMappingOptions::MapAll)
        {
          return;
        }
        else
        {
          DeleteError(error);
          NoError(error);
          continue;
        }
      }
      else if (!IsSuccess(*error))
      {
        return;
      }
      double scale_factor = 1.0;
      if (node["scale factor"].IsDefined())
      {
        scale_factor = node["scale factor"].as<double>();
      }
      mappings.push_back({ source_index, target_index, scale_factor });
    }
    index_mapping->mappings_ = new IndexMapping[mappings.size()];
    index_mapping->size_ = mappings.size();
    for (std::size_t i = 0; i < mappings.size(); i++)
    {
      index_mapping->mappings_[i] = mappings[i];
    }
  }

  std::size_t GetIndexMappingsSize(const IndexMappings mappings)
  {
    return mappings.size_;
  }

  void CopyData(const IndexMappings mappings, const double* source, double* target)
  {
    for (std::size_t i = 0; i < mappings.size_; i++)
    {
      target[mappings.mappings_[i].target_] = source[mappings.mappings_[i].source_] * mappings.mappings_[i].scale_factor_;
    }
  }

  void DeleteMapping(Mapping* mapping)
  {
    DeleteString(&(mapping->name_));
  }

  void DeleteMappings(Mappings* mappings)
  {
    if (mappings->mappings_ == nullptr)
      return;
    for (std::size_t i = 0; i < mappings->size_; i++)
    {
      DeleteMapping(&(mappings->mappings_[i]));
    }
    delete[] mappings->mappings_;
    mappings->mappings_ = nullptr;
    mappings->size_ = 0;
  }

  void DeleteIndexMappings(IndexMappings* mappings)
  {
    if (mappings->mappings_ == nullptr)
      return;

    delete[] mappings->mappings_;
    mappings->mappings_ = nullptr;
    mappings->size_ = 0;
  }

  void MusicaVersion(String* musica_version)
  {
    CreateString(GetMusicaVersion(), musica_version);
  }

  void ToMapping(const char* name, std::size_t index, Mapping* mapping)
  {
    CreateString(name, &mapping->name_);
    mapping->index_ = index;
  }
}  // namespace musica
