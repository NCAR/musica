// Copyright (C) 2023-2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <musica/tuvx/radiator.hpp>
#include <musica/util.hpp>

#include <memory>
#include <string>
#include <vector>

namespace musica
{

  /// @brief Radiator map used to access radiator information in tuvx
  class RadiatorMap
  {
   public:
    RadiatorMap(void *radiator_map)
        : radiator_map_(radiator_map),
          owns_radiator_map_(false)
    {
    }

    /// @brief Creates radiator map
    /// @param error Error to indicate success or failure
    RadiatorMap(Error *error);

    ~RadiatorMap();

    /// @brief Adds a radiator to the radiator map
    /// @param radiator Radiator to add
    /// @param error Error to indicate success or failure
    void AddRadiator(Radiator *radiator, Error *error);

    /// @brief Returns a radiator. For now, this calls the interal tuvx fortran api, but will allow the change to c++ later
    /// on to be transparent to downstream projects
    /// @param radiator_name Radiator name
    /// @param error Error to indicate success or failure
    /// @return Radiator
    Radiator *GetRadiator(const char *radiator_name, Error *error);

    /// @brief Returns a radiator based on its index in the map
    /// @param index Index of the radiator we want
    /// @param error Error to indicate success or failure
    /// @return Radiator
    Radiator *GetRadiatorByIndex(std::size_t index, Error *error);

    /// @brief Removes a radiator from the map by name
    /// @param radiator_name Radiator name
    /// @param error Error to indicate success or failure
    void RemoveRadiator(const char *radiator_name, Error *error);

    /// @brief Removes a radiator from the map by index
    /// @param index Index of the radiator to remove
    /// @param error Error to indicate success or failure
    void RemoveRadiatorByIndex(std::size_t index, Error *error);

    /// @brief Gets the number of radiators in the map
    /// @param error Error to indicate success or failure
    /// @return Number of radiators in the map
    std::size_t GetNumberOfRadiators(Error *error);

   private:
    void *radiator_map_;
    bool owns_radiator_map_;

    friend class TUVX;
  };

#ifdef __cplusplus
  extern "C"
  {
#endif

    // The external C API for TUVX
    // callable by wrappers in other languages

    /// @brief Creates radiator map
    /// @param error Error to indicate success or failure
    /// @return Radiator map
    RadiatorMap *CreateRadiatorMap(Error *error);

    /// @brief Deletes radiator map
    /// @param radiator_map Radiator map to delete
    /// @param error Error to indicate success or failure
    void DeleteRadiatorMap(RadiatorMap *radiator_map, Error *error);

    /// @brief Adds a radiator to the radiator map
    /// @param radiator_map Radiator map to add the radiator to
    /// @param radiator Radiator to add
    /// @param error Error to indicate success or failure
    void AddRadiator(RadiatorMap *radiator_map, Radiator *radiator, Error *error);

    /// @brief Returns a radiator from the radiator map
    /// @param radiator_map Radiator map to get the radiator from
    /// @param radiator_name Radiator name
    /// @param error Error to indicate success or failure
    /// @return The radiator pointer, or nullptr if the radiator is not found
    Radiator *GetRadiator(RadiatorMap *radiator_map, const char *radiator_name, Error *error);

    /// @brief Returns a radiator from the radiator map by index
    /// @param radiator_map Radiator map to get the radiator from
    /// @param index Index of the radiator we want
    /// @param error Error to indicate success or failure
    /// @return The radiator pointer, or nullptr if the radiator is not found
    Radiator *GetRadiatorByIndex(RadiatorMap *radiator_map, std::size_t index, Error *error);

    /// @brief Removes a radiator from the radiator map by name
    /// @param radiator_map Radiator map to remove the radiator from
    /// @param radiator_name Radiator name
    /// @param error Error to indicate success or failure
    void RemoveRadiator(RadiatorMap *radiator_map, const char *radiator_name, Error *error);

    /// @brief Removes a radiator from the radiator map by index
    /// @param radiator_map Radiator map to remove the radiator from
    /// @param index Index of the radiator to remove
    /// @param error Error to indicate success or failure
    void RemoveRadiatorByIndex(RadiatorMap *radiator_map, std::size_t index, Error *error);

    /// @brief Gets the number of radiators in the radiator map
    /// @param radiator_map Radiator map to get the number of radiators from
    /// @param error Error to indicate success or failure
    /// @return Number of radiators in the radiator map
    std::size_t GetNumberOfRadiators(RadiatorMap *radiator_map, Error *error);

    // INTERNAL USE. If tuvx ever gets rewritten in C++, these functions will
    // go away but the C API will remain the same and downstream projects (like CAM-SIMA) will
    // not need to change
    void *InternalCreateRadiatorMap(int *error_code);
    void InternalDeleteRadiatorMap(void *radiator_map, int *error_code);
    void InternalAddRadiator(void *radiator_map, void *radiator, int *error_code);
    void *
    InternalGetRadiator(void *radiator_map, const char *radiator_name, std::size_t radiator_name_length, int *error_code);
    void *InternalGetRadiatorByIndex(void *radiator_map, std::size_t index, int *error_code);
    void *InternalGetRadiatorUpdaterFromMap(void *radiator_map, void *radiator, int *error_code);
    void
    InternalRemoveRadiator(void *radiator_map, const char *radiator_name, std::size_t radiator_name_length, int *error_code);
    void InternalRemoveRadiatorByIndex(void *radiator_map, std::size_t index, int *error_code);
    std::size_t InternalGetNumberOfRadiators(void *radiator_map, int *error_code);

#ifdef __cplusplus
  }
#endif

}  // namespace musica
