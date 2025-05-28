// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/util.hpp>

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

  String* CreateString(const char* value)
  {
    auto* str = new String;
    str->size_ = std::strlen(value);
    str->value_ = new char[str->size_ + 1];
    std::strcpy(str->value_, value);
    return str;
  }

  void DeleteString(String* str)
  {
    if (str->value_ != nullptr)
      delete[] str->value_;
    str->value_ = nullptr;
    str->size_ = 0;
  }

  Error* NoError()
  {
    return new Error{0, *CreateString(""), *CreateString("Success")};
  }

  Error* ToError(const char* category, int code)
  {
    return ToError(category, code, "");
  }

  Error* ToError(const char* category, int code, const char* message)
  {
    Error* error = new Error;
    error->code_ = code;
    String* category_str = CreateString(category);
    String* message_str  = CreateString(message);
    error->code_ = code;
    error->category_ = *category_str;
    error->message_  = *message_str;

    return error;
  }

  Error* ToError(const std::system_error& e)
  {
    Error* temp = ToError(e.code().category().name(), e.code().value(), e.what());
    Error* result = new Error(*temp);
    DeleteError(temp);
    return result;
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

  Configuration* LoadConfigurationFromString(const char* data, Error* error)
  {
    DeleteError(error);
    auto* config = new Configuration;
    try
    {
      config->data_ = new YAML::Node(YAML::Load(data));
      Error* temp = NoError();
      *error = *temp;
      DeleteError(temp);
    }
    catch (const std::exception& e)
    {
      config->data_ = nullptr;
      Error* temp = ToError(MUSICA_ERROR_CATEGORY, MUSICA_PARSE_PARSING_FAILED, e.what());
      *error = *temp;
      DeleteError(temp);
    }
    return config;
  }

  Configuration* LoadConfigurationFromFile(const char* filename, Error* error)
  {
    DeleteError(error);
    auto* config = new Configuration;
    try
    {
      config->data_ = new YAML::Node(YAML::LoadFile(filename));
      Error* temp = NoError();
      *error = *temp;
      DeleteError(temp);
    }
    catch (const std::exception& e)
    {
      config->data_ = nullptr;
      Error* temp = ToError(MUSICA_ERROR_CATEGORY, MUSICA_PARSE_PARSING_FAILED, e.what());
      *error = *temp;
      DeleteError(temp);
    }
    return config;
  }

  void DeleteConfiguration(Configuration* config)
  {
    if (config->data_ != nullptr)
      delete config->data_;
    config->data_ = nullptr;
  }

  Mapping ToMapping(const char* name, std::size_t index)
  {
    Mapping mapping;
    mapping.name_ = *CreateString(name);
    mapping.index_ = index;
    return mapping;
  }

  Mapping* AllocateMappingArray(const std::size_t size)
  {
    return new Mapping[size];
  }

  Mappings* CreateMappings(std::size_t size)
  {
    auto* mappings = new Mappings;
    mappings->mappings_ = new Mapping[size];
    mappings->size_ = size;
    return mappings;
  }

  std::size_t FindMappingIndex(const Mappings mappings, const char* name, Error* error)
  {
    DeleteError(error);
    for (std::size_t i = 0; i < mappings.size_; i++)
    {
      if (std::strcmp(mappings.mappings_[i].name_.value_, name) == 0)
      {
        Error* temp = NoError();
        *error = *temp;
        DeleteError(temp);
        return mappings.mappings_[i].index_;
      }
    }
    std::string msg = "Mapping element '" + std::string(name) + "' not found";
    Error* temp = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_MAPPING_NOT_FOUND, msg.c_str());
    *error = *temp;
    DeleteError(temp);    
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

  IndexMappings* CreateIndexMappings(
      const Configuration configuration,
      const IndexMappingOptions map_options,
      const Mappings source,
      const Mappings target,
      Error* error)
  {
    DeleteError(error);
    auto* index_mappings = new IndexMappings;
    index_mappings->mappings_ = nullptr;
    index_mappings->size_ = 0;

    if (map_options == IndexMappingOptions::UndefinedMapping)
    {
      Error* temp = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_MAPPING_OPTIONS_UNDEFINED,
                      "Mapping options are undefined");
      *error = *temp;
      DeleteError(temp);
      return index_mappings;
    }

    std::vector<IndexMapping> mappings;

    for (const auto& node : *configuration.data_)
    {
      std::string source_name = node["source"].as<std::string>();
      std::string target_name = node["target"].as<std::string>();

      std::size_t source_index = FindMappingIndex(source, source_name.c_str(), error);
      if (error->code_ == MUSICA_ERROR_CODE_MAPPING_NOT_FOUND)
      {
        if (map_options == IndexMappingOptions::MapAll)
          return index_mappings;
        DeleteError(error);
        Error* temp = NoError();
        *error = *temp;
        DeleteError(temp);
        continue;
      }
      else if (!IsSuccess(*error))
        return index_mappings;

      std::size_t target_index = FindMappingIndex(target, target_name.c_str(), error);
      if (error->code_ == MUSICA_ERROR_CODE_MAPPING_NOT_FOUND)
      {
        if (map_options == IndexMappingOptions::MapAll)
          return index_mappings;
        DeleteError(error);
        Error* temp = NoError();
        *error = *temp;
        DeleteError(temp);
        continue;
      }
      else if (!IsSuccess(*error))
        return index_mappings;

      double scale = node["scale factor"].IsDefined() ? node["scale factor"].as<double>() : 1.0;
      mappings.emplace_back(IndexMapping{ source_index, target_index, scale });
    }

    index_mappings->size_ = mappings.size();
    index_mappings->mappings_ = new IndexMapping[mappings.size()];
    std::copy(mappings.begin(), mappings.end(), index_mappings->mappings_);

    return index_mappings;
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

  void SetError(Error* error, int code, const char* category_str, const char* message_str)
  {
    if (!error) return;
    DeleteError(error);
    error->code_ = code;
    String* category = CreateString(category_str);
    String* message = CreateString(message_str);

    error->category_ = *category;  // dereference pointer to copy struct
    error->message_ = *message;
  }

}  // namespace musica
