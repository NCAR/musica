
// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/utils/error_handler.hpp>
#include <musica/utils/string.hpp>
#ifdef MUSICA_USE_MICM
  #include <micm/util/vector_matrix.hpp>
#endif

#include <cstddef>

#ifdef __cplusplus

namespace musica
{

  extern "C"
  {
#endif

    /// @brief Vector dimension for Vector-ordered matrices
    #ifdef MUSICA_USE_MICM
      const size_t MUSICA_VECTOR_SIZE = MICM_DEFAULT_VECTOR_SIZE;
    #else
      const size_t MUSICA_VECTOR_SIZE = 0;
    #endif

    /// @brief A set of configuration data
    struct Configuration
    {
      void* data_ = nullptr;  // Opaque pointer to YAML::Node (implementation detail)
    };

    /// @brief Options for mapping between indices in two arrays
    enum IndexMappingOptions
    {
      UndefinedMapping = 0,  // Undefined mapping
      MapAny = 1,            // Map any pair of source and target elements that exists
      MapAll = 2             // Map every pair of source and target elements and fail if any are missing
    };

    /// @brief A struct to represent a mapping between a string and an index
    struct Mapping
    {
      String name_;
      std::size_t index_;
    };

    /// @brief A struct to represent an array of Mappings
    struct Mappings
    {
      Mapping* mappings_ = nullptr;
      std::size_t size_ = 0;
    };

    /// @brief A struct to represent the mapping between indices in two arrays
    struct IndexMapping
    {
      std::size_t source_;
      std::size_t target_;
      double scale_factor_ = 1.0;  // Scaling factor applied to the source data
    };

    /// @brief A struct to represent an array of IndexMappings
    struct IndexMappings
    {
      IndexMapping* mappings_ = nullptr;
      std::size_t size_ = 0;
    };

    /// @brief Loads a set of configuration data from a string
    /// @param data The string to load [input]
    /// @param configuration The Configuration [output]
    /// @param error The Error to populate if the data cannot be loaded [output]
    void LoadConfigurationFromString(const char* data, Configuration* configuration, Error* error);

    /// @brief Loads a set of configuration data from a file
    /// @param filename The file to load [input]
    /// @param configuration The Configuration [output]
    /// @param error The Error to populate if the data cannot be loaded [output]
    void LoadConfigurationFromFile(const char* filename, Configuration* configuration, Error* error);

    /// @brief Deletes a Configuration
    /// @param config The Configuration to delete
    void DeleteConfiguration(Configuration* config);

    /// @brief Allocates an array of Mappings
    /// @param size The size of the array
    /// @return The array of Mappings
    Mapping* AllocateMappingArray(const std::size_t size);

    /// @brief Allocate a new Mappings struct
    /// @param size The size of the Mappings [input]
    /// @param mapping The Mappings [output]
    void CreateMappings(std::size_t size, Mappings* mapping);

    /// @brief Finds the index of a Mapping by name
    /// @param mappings The array of Mappings
    /// @param name The name to search for
    /// @param error The Error to populate if the Mapping is not found
    /// @return The index of the Mapping
    std::size_t FindMappingIndex(const Mappings mappings, const char* name, Error* error);

    /// @brief Creates a set of index mappings
    /// @param configuration The Configuration containing the mappings [input]
    /// @param map_options The options for mapping [input]
    /// @param source The source array of name-index Mappings [input]
    /// @param target The target array of name-index Mappings [input]
    /// @param index_mapping The array of IndexMappings [output]
    /// @param error The Error to populate if a Mapping is not found [output]
    void CreateIndexMappings(
        const Configuration configuration,
        const IndexMappingOptions map_options,
        const Mappings source,
        const Mappings target,
        IndexMappings* index_mapping,
        Error* error);

    /// @brief Returns the number of elements in an IndexMappings container
    /// @param mappings The IndexMappings container
    /// @return The number of elements
    std::size_t GetIndexMappingsSize(const IndexMappings mappings);

    /// @brief Copies data from one array to another using IndexMappings
    /// @param mappings The array of IndexMappings
    /// @param source The source array
    /// @param target The target array
    void CopyData(const IndexMappings mappings, const double* source, double* target);

    /// @brief Deletes a Mapping
    /// @param mapping The Mapping to delete
    void DeleteMapping(Mapping* mapping);

    /// @brief Deletes an array of Mappings
    /// @param mappings The array of Mappings to delete
    void DeleteMappings(Mappings* mappings);

    /// @brief Deletes an array of IndexMappings
    /// @param mappings The array of IndexMappings to delete
    void DeleteIndexMappings(IndexMappings* mappings);

    /// @brief Get the MUSICA version
    /// @param musica_version MUSICA version [output]
    void MusicaVersion(String* musica_version);

#ifdef __cplusplus
  }

  /// @brief Creates a Mapping from a name and index
  /// @param name The name of the Mapping
  /// @param index The index of the Mapping
  /// @return The Mapping
  void ToMapping(const char* name, std::size_t index, Mapping* mapping);

#endif

}  // namespace musica