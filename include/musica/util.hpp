
// Copyright (C) 2023-2025 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/error.hpp>
#ifdef MUSICA_USE_MICM
  #include <micm/util/vector_matrix.hpp>
#endif

#include <cstddef>

#ifdef __cplusplus
  #include <yaml-cpp/yaml.h>

  #include <system_error>

namespace musica
{

  extern "C"
  {
    typedef YAML::Node Yaml;
#endif

    /// @brief Vector dimension for Vector-ordered matrices
#ifdef MUSICA_USE_MICM
    const size_t MUSICA_VECTOR_SIZE = MICM_DEFAULT_VECTOR_SIZE;
#else
const size_t MUSICA_VECTOR_SIZE = 0;
#endif

    /// @brief Options for mapping between indices in two arrays
    enum IndexMappingOptions
    {
      UndefinedMapping = 0,  // Undefined mapping
      MapAny = 1,            // Map any pair of source and target elements that exists
      MapAll = 2             // Map every pair of source and target elements and fail if any are missing
    };

    /// @brief A struct to represent a string
    struct String
    {
      char* value_ = nullptr;
      std::size_t size_ = 0;
    };

    /// @brief A struct to describe failure conditions
    struct Error
    {
      int code_ = 0;
      String category_;
      String message_;
    };

    /// @brief A set of configuration data
    struct Configuration
    {
      Yaml* data_ = nullptr;
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

    /// @brief Casts a char* to a String
    /// @param value The char* to cast
    /// @return The casted String
    String CreateString(const char* value);

    /// @brief Deletes a String
    /// @param str The String to delete
    void DeleteString(String* str);

    /// @brief Creates an Error indicating no error
    /// @return The Error
    Error NoError();

    /// @brief Creates an Error from a category, code, and message
    /// @param category The category of the Error
    /// @param code The code of the Error
    /// @param message The message of the Error
    /// @return The Error
    Error ToError(const char* category, int code, const char* message);

    /// @brief Loads a set of configuration data from a string
    /// @param data The string to load
    /// @param error The Error to populate if the data cannot be loaded
    /// @return The Configuration
    Configuration LoadConfigurationFromString(const char* data, Error* error);

    /// @brief Loads a set of configuration data from a file
    /// @param filename The file to load
    /// @param error The Error to populate if the data cannot be loaded
    /// @return The Configuration
    Configuration LoadConfigurationFromFile(const char* filename, Error* error);

    /// @brief Allocates an array of Mappings
    /// @param size The size of the array
    /// @return The array of Mappings
    Mapping* AllocateMappingArray(const std::size_t size);

    /// @brief Allocate a new Mappings struct
    /// @param size The size of the Mappings
    /// @return The Mappings
    Mappings CreateMappings(std::size_t size);

    /// @brief Finds the index of a Mapping by name
    /// @param mappings The array of Mappings
    /// @param name The name to search for
    /// @param error The Error to populate if the Mapping is not found
    /// @return The index of the Mapping
    std::size_t FindMappingIndex(const Mappings mappings, const char* name, Error* error);

    /// @brief Creates a set of index mappings
    /// @param configuration The Configuration containing the mappings
    /// @param map_options The options for mapping
    /// @param source The source array of name-index Mappings
    /// @param target The target array of name-index Mappings
    /// @param error The Error to populate if a Mapping is not found
    /// @return The array of IndexMappings
    IndexMappings CreateIndexMappings(
        const Configuration configuration,
        const IndexMappingOptions map_options,
        Mappings source,
        const Mappings target,
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

    /// @brief Deletes an Error
    /// @param error The Error to delete
    void DeleteError(Error* error);

    /// @brief Deletes a Configuration
    /// @param config The Configuration to delete
    void DeleteConfiguration(Configuration* config);

    /// @brief Deletes a Mapping
    /// @param mapping The Mapping to delete
    void DeleteMapping(Mapping* mapping);

    /// @brief Deletes an array of Mappings
    /// @param mappings The array of Mappings to delete
    void DeleteMappings(Mappings* mappings);

    /// @brief Deletes an IndexMapping
    /// @param mapping The IndexMapping to delete
    void DeleteIndexMapping(IndexMapping* mapping);

    /// @brief Deletes an array of IndexMappings
    /// @param mappings The array of IndexMappings to delete
    void DeleteIndexMappings(IndexMappings* mappings);

#ifdef __cplusplus
  }
  /// @brief Creates an Error from a category and code
  /// @param category The category of the Error
  /// @param code The code of the Error
  /// @return The Error
  Error ToError(const char* category, int code);

  /// @brief Creates an Error from syd::system_error
  /// @param e The std::system_error to convert
  /// @return The Error
  Error ToError(const std::system_error& e);

  /// @brief Checks for success
  /// @param error The Error to check
  /// @return True if the Error is successful, false otherwise
  bool IsSuccess(const Error& error);

  /// @brief Checks for a specific error
  /// @param error The Error to check
  /// @param category The category of the Error
  /// @param code The code of the Error
  /// @return True if the Error matches the category and code, false otherwise
  bool IsError(const Error& error, const char* category, int code);

  /// @brief Overloads the equality operator for Error types
  /// @param lhs The left-hand side Error
  /// @param rhs The right-hand side Error
  /// @return True if the Errors are equal, false otherwise
  bool operator==(const Error& lhs, const Error& rhs);

  /// @brief Overloads the inequality operator for Error types
  /// @param lhs The left-hand side Error
  /// @param rhs The right-hand side Error
  /// @return True if the Errors are not equal, false otherwise
  bool operator!=(const Error& lhs, const Error& rhs);

  /// @brief Creates a Mapping from a name and index
  /// @param name The name of the Mapping
  /// @param index The index of the Mapping
  /// @return The Mapping
  Mapping ToMapping(const char* name, std::size_t index);

#endif

}  // namespace musica