// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/util.hpp>
#include <musica/version.hpp>

#include <cstddef>
#include <cstring>
#include <iostream>

namespace
{
  struct Yaml
  {
    YAML::Node node_;
  };
}  // namespace

namespace musica
{

  void CreateString(const char* value, String* str)
  {
    str->size_ = std::strlen(value);
    str->value_ = new char[str->size_ + 1];
    std::memcpy(str->value_, value, str->size_ + 1);
  }

  void DeleteString(String* str)
  {
    if (str->value_ != nullptr)
      delete[] str->value_;
    str->value_ = nullptr;
    str->size_ = 0;
  }

  void NoError(Error* error)
  {
    ToError("", 0, "Success", error);
  }

  void ToError(const char* category, int code, Error* error)
  {
    ToError(category, code, "", error);
  }

  void ToError(const char* category, int code, const char* message, Error* error)
  {
    error->code_ = code;
    CreateString(category, &error->category_);
    CreateString(message, &error->message_);
  }

  void ToError(const std::system_error& e, Error* error)
  {
    ToError(e.code().category().name(), e.code().value(), e.what(), error);
  }

  bool IsSuccess(const Error& error)
  {
    return error.code_ == 0;
  }

  bool IsError(const Error& error, const char* category, int code)
  {
    return error.code_ == code && (error.category_.value_ == nullptr && category == nullptr) ||
           std::strcmp(error.category_.value_, category) == 0;
  }

  void DeleteError(Error* error)
  {
    DeleteString(&(error->category_));
    DeleteString(&(error->message_));
  }

  bool operator==(const Error& lhs, const Error& rhs)
  {
    if (lhs.code_ == 0 && rhs.code_ == 0)
    {
      return true;
    }
    return lhs.code_ == rhs.code_ && (lhs.category_.value_ == nullptr && rhs.category_.value_ == nullptr) ||
           std::strcmp(lhs.category_.value_, rhs.category_.value_) == 0;
  }

  bool operator!=(const Error& lhs, const Error& rhs)
  {
    return !(lhs == rhs);
  }

  void LoadConfigurationFromString(const char* data, Configuration* configuration, Error* error)
  {
    DeleteError(error);
    try
    {
      configuration->data_ = new YAML::Node(YAML::Load(data));
      NoError(error);
    }
    catch (const std::exception& e)
    {
      configuration->data_ = nullptr;
      ToError(MUSICA_ERROR_CATEGORY, MUSICA_PARSE_PARSING_FAILED, e.what(), error);
    }
  }

  void LoadConfigurationFromFile(const char* filename, Configuration* configuration, Error* error)
  {
    DeleteError(error);
    try
    {
      configuration->data_ = new YAML::Node(YAML::LoadFile(filename));
      NoError(error);
    }
    catch (const std::exception& e)
    {
      configuration->data_ = nullptr;
      ToError(MUSICA_ERROR_CATEGORY, MUSICA_PARSE_PARSING_FAILED, e.what(), error);
    }
  }

  void DeleteConfiguration(Configuration* config)
  {
    if (config->data_ != nullptr)
      delete config->data_;
    config->data_ = nullptr;
  }

  void ToMapping(const char* name, std::size_t index, Mapping* mapping)
  {
    CreateString(name, &mapping->name_);
    mapping->index_ = index;
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
    ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_MAPPING_NOT_FOUND, msg.c_str(), error);
    return 0;
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
    std::size_t const size = configuration.data_->size();
    std::vector<IndexMapping> mappings;
    index_mapping->size_ = 0;
    if (map_options == IndexMappingOptions::UndefinedMapping)
    {
      ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_MAPPING_OPTIONS_UNDEFINED, "Mapping options are undefined", error);
      return;
    }
    for (std::size_t i = 0; i < size; i++)
    {
      const YAML::Node& node = (*configuration.data_)[i];
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
    return;
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

  void DeleteIndexMapping(IndexMapping* mapping)
  {
    // Nothing to do
  }

  void DeleteIndexMappings(IndexMappings* mappings)
  {
    if (mappings->mappings_ == nullptr)
      return;
    for (std::size_t i = 0; i < mappings->size_; i++)
    {
      DeleteIndexMapping(&(mappings->mappings_[i]));
    }
    delete[] mappings->mappings_;
  }

  void MusicaVersion(String* musica_version)
  {
    CreateString(GetMusicaVersion(), musica_version);
  }

}  // namespace musica
