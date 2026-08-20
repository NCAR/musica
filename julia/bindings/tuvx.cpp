// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// TUV-x bindings for the Julia interface. This translation unit is compiled into
// libmusica_julia only when MUSICA is built with TUV-x enabled (see
// julia/CMakeLists.txt), so it needs no preprocessor guards — it registers
// itself with the module via the self-registration registry (registration.hpp).
#include "musica/tuvx/tuvx.hpp"

#include "jlcxx/jlcxx.hpp"
#include "musica/tuvx/grid.hpp"
#include "musica/tuvx/grid_map.hpp"
#include "musica/tuvx/profile.hpp"
#include "musica/tuvx/profile_map.hpp"
#include "musica/tuvx/radiator.hpp"
#include "musica/tuvx/radiator_map.hpp"
#include "registration.hpp"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
  /// @brief Throws a Julia-visible exception when a MUSICA call failed.
  ///
  /// jlcxx translates a C++ exception into a Julia ErrorException, so the Julia
  /// layer needs no separate error type.
  void check_error(musica::Error& error, const char* context)
  {
    if (musica::IsSuccess(error))
    {
      musica::DeleteError(&error);
      return;
    }
    std::string message = std::string(context) + ": " + std::string(error.message_.value_);
    musica::DeleteError(&error);
    throw std::runtime_error(message);
  }

  /// @brief Reports a failure during finalization without throwing.
  ///
  /// Julia runs finalizers on its own schedule, where an exception is unhelpful
  /// and may be raised at an arbitrary point in the program.
  void report_error(musica::Error& error, const char* context)
  {
    if (!musica::IsSuccess(error))
      std::cerr << context << ": " << error.message_.value_ << std::endl;
    musica::DeleteError(&error);
  }

  /// @brief Throws a Julia-visible exception when a pointer from Julia is null.
  ///
  /// A CxxPtr is not guaranteed non-null the way a Julia-level reference would
  /// be, so every method taking a raw pointer checks it before dereferencing.
  template<typename T>
  void check_not_null(T* ptr, const char* type_name)
  {
    if (!ptr)
      throw std::runtime_error(std::string(type_name) + " pointer is null");
  }

  /// @brief Extracts the names from a Mappings struct (does not free it).
  std::vector<std::string> mapping_names(const musica::Mappings& mappings)
  {
    std::vector<std::string> names;
    names.reserve(mappings.size_);
    for (std::size_t i = 0; i < mappings.size_; ++i)
      names.emplace_back(mappings.mappings_[i].name_.value_, mappings.mappings_[i].name_.size_);
    return names;
  }

  /// @brief Extracts the indices from a Mappings struct (does not free it).
  std::vector<int64_t> mapping_indices(const musica::Mappings& mappings)
  {
    std::vector<int64_t> indices;
    indices.reserve(mappings.size_);
    for (std::size_t i = 0; i < mappings.size_; ++i)
      indices.push_back(static_cast<int64_t>(mappings.mappings_[i].index_));
    return indices;
  }

  void register_tuvx(jlcxx::Module& mod)
  {
    // ── Version ──────────────────────────────────────────────────────────
    // Calls into the TUV-x Fortran library.
    mod.method("get_tuvx_version", []() { return musica::TUVX::GetVersion(); });

    // ── Opaque types ─────────────────────────────────────────────────────
    mod.add_type<musica::Grid>("CppGrid");
    mod.add_type<musica::GridMap>("CppGridMap");
    mod.add_type<musica::Profile>("CppProfile");
    mod.add_type<musica::ProfileMap>("CppProfileMap");
    mod.add_type<musica::Radiator>("CppRadiator");
    mod.add_type<musica::RadiatorMap>("CppRadiatorMap");
    mod.add_type<musica::TUVX>("CppTUVX");

    // ── Grid creation / deletion ─────────────────────────────────────────
    mod.method(
        "cpp_create_grid",
        [](const std::string& name, const std::string& units, int64_t num_sections)
        {
          if (num_sections <= 0)
            throw std::runtime_error("num_sections must be greater than 0");
          musica::Error error;
          musica::Grid* grid =
              musica::CreateGrid(name.c_str(), units.c_str(), static_cast<std::size_t>(num_sections), &error);
          // CreateGrid returns the object even when it sets an error, so delete
          // the partly built grid before the exception leaves this function.
          if (!musica::IsSuccess(error))
          {
            musica::Error delete_error;
            musica::DeleteGrid(grid, &delete_error);
            musica::DeleteError(&delete_error);
          }
          check_error(error, "Error creating grid");
          if (!grid)
            throw std::runtime_error("Grid creation returned null pointer");
          return grid;
        });

    mod.method(
        "cpp_delete_grid",
        [](musica::Grid* grid)
        {
          if (!grid)
            return;
          musica::Error error;
          musica::DeleteGrid(grid, &error);
          report_error(error, "Error deleting Grid");
        });

    // ── Grid accessors ───────────────────────────────────────────────────
    mod.method(
        "cpp_grid_name",
        [](musica::Grid* grid)
        {
          check_not_null(grid, "Grid");
          musica::Error error;
          std::string name = grid->GetName(&error);
          check_error(error, "Error getting grid name");
          return name;
        });

    mod.method(
        "cpp_grid_units",
        [](musica::Grid* grid)
        {
          check_not_null(grid, "Grid");
          musica::Error error;
          std::string units = grid->GetUnits(&error);
          check_error(error, "Error getting grid units");
          return units;
        });

    mod.method(
        "cpp_grid_num_sections",
        [](musica::Grid* grid)
        {
          check_not_null(grid, "Grid");
          musica::Error error;
          std::size_t num_sections = grid->GetNumberOfSections(&error);
          check_error(error, "Error getting number of grid sections");
          return static_cast<int64_t>(num_sections);
        });

    // The two pointer functions return the address of the TUV-x array as an
    // integer. Julia wraps that address with unsafe_wrap to get a zero-copy
    // view for READING. An integer avoids any dependence on how jlcxx maps a
    // raw double* into Julia. Writes must go through the cpp_grid_set_*!
    // methods below instead of through this view: TUV-x tracks some derived
    // state (e.g. a profile's exo layer density, folded into its top layer)
    // outside the raw buffer, and only the proper SetX call keeps it in
    // sync — a raw pointer write bypasses that bookkeeping silently.
    mod.method(
        "cpp_grid_edges_pointer",
        [](musica::Grid* grid)
        {
          check_not_null(grid, "Grid");
          musica::Error error;
          double* edges = grid->GetEdgesPointer(&error);
          check_error(error, "Error getting grid edges pointer");
          if (!edges)
            throw std::runtime_error("Grid edges pointer is null");
          return static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(edges));
        });

    mod.method(
        "cpp_grid_midpoints_pointer",
        [](musica::Grid* grid)
        {
          check_not_null(grid, "Grid");
          musica::Error error;
          double* midpoints = grid->GetMidpointsPointer(&error);
          check_error(error, "Error getting grid midpoints pointer");
          if (!midpoints)
            throw std::runtime_error("Grid midpoints pointer is null");
          return static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(midpoints));
        });

    mod.method(
        "cpp_grid_set_edges!",
        [](musica::Grid* grid, jlcxx::ArrayRef<double> values)
        {
          check_not_null(grid, "Grid");
          musica::Error error;
          grid->SetEdges(values.data(), values.size(), &error);
          check_error(error, "Error setting grid edges");
        });

    mod.method(
        "cpp_grid_set_midpoints!",
        [](musica::Grid* grid, jlcxx::ArrayRef<double> values)
        {
          check_not_null(grid, "Grid");
          musica::Error error;
          grid->SetMidpoints(values.data(), values.size(), &error);
          check_error(error, "Error setting grid midpoints");
        });

    // ── GridMap creation / deletion ──────────────────────────────────────
    mod.method(
        "cpp_create_grid_map",
        []()
        {
          musica::Error error;
          musica::GridMap* grid_map = musica::CreateGridMap(&error);
          // CreateGridMap returns the object even when it sets an error.
          if (!musica::IsSuccess(error))
          {
            musica::Error delete_error;
            musica::DeleteGridMap(grid_map, &delete_error);
            musica::DeleteError(&delete_error);
          }
          check_error(error, "Error creating grid map");
          if (!grid_map)
            throw std::runtime_error("Grid map creation returned null pointer");
          return grid_map;
        });

    mod.method(
        "cpp_delete_grid_map",
        [](musica::GridMap* grid_map)
        {
          if (!grid_map)
            return;
          musica::Error error;
          musica::DeleteGridMap(grid_map, &error);
          report_error(error, "Error deleting GridMap");
        });

    // ── GridMap accessors ────────────────────────────────────────────────
    mod.method(
        "cpp_grid_map_add_grid!",
        [](musica::GridMap* grid_map, musica::Grid* grid)
        {
          check_not_null(grid_map, "GridMap");
          check_not_null(grid, "Grid");
          musica::Error error;
          grid_map->AddGrid(grid, &error);
          check_error(error, "Error adding grid to grid map");
        });

    // Both getters return a Grid that the caller owns. The Julia wrapper takes
    // ownership and deletes it in a finalizer.
    mod.method(
        "cpp_grid_map_get_grid",
        [](musica::GridMap* grid_map, const std::string& name, const std::string& units)
        {
          check_not_null(grid_map, "GridMap");
          musica::Error error;
          musica::Grid* grid = grid_map->GetGrid(name.c_str(), units.c_str(), &error);
          check_error(error, "Error getting grid");
          if (!grid)
            throw std::runtime_error("Grid '" + name + "' [" + units + "] not found in the grid map");
          return grid;
        });

    mod.method(
        "cpp_grid_map_get_grid_by_index",
        [](musica::GridMap* grid_map, int64_t index)
        {
          check_not_null(grid_map, "GridMap");
          if (index < 0)
            throw std::out_of_range("Grid index " + std::to_string(index) + " is negative");
          musica::Error error;
          musica::Grid* grid = grid_map->GetGridByIndex(static_cast<std::size_t>(index), &error);
          check_error(error, "Error getting grid by index");
          if (!grid)
            throw std::out_of_range("Grid index " + std::to_string(index) + " is out of range");
          return grid;
        });

    mod.method(
        "cpp_grid_map_remove_grid!",
        [](musica::GridMap* grid_map, const std::string& name, const std::string& units)
        {
          check_not_null(grid_map, "GridMap");
          musica::Error error;
          grid_map->RemoveGrid(name.c_str(), units.c_str(), &error);
          check_error(error, "Error removing grid");
        });

    mod.method(
        "cpp_grid_map_remove_grid_by_index!",
        [](musica::GridMap* grid_map, int64_t index)
        {
          check_not_null(grid_map, "GridMap");
          if (index < 0)
            throw std::out_of_range("Grid index " + std::to_string(index) + " is negative");
          musica::Error error;
          grid_map->RemoveGridByIndex(static_cast<std::size_t>(index), &error);
          check_error(error, "Error removing grid by index");
        });

    mod.method(
        "cpp_grid_map_number_of_grids",
        [](musica::GridMap* grid_map)
        {
          check_not_null(grid_map, "GridMap");
          musica::Error error;
          std::size_t num_grids = grid_map->GetNumberOfGrids(&error);
          check_error(error, "Error getting number of grids");
          return static_cast<int64_t>(num_grids);
        });

    // ── Profile creation / deletion ──────────────────────────────────────
    mod.method(
        "cpp_create_profile",
        [](const std::string& name, const std::string& units, musica::Grid* grid)
        {
          check_not_null(grid, "Grid");
          musica::Error error;
          musica::Profile* profile = musica::CreateProfile(name.c_str(), units.c_str(), grid, &error);
          // CreateProfile returns the object even when it sets an error, so delete
          // the partly built profile before the exception leaves this function.
          if (!musica::IsSuccess(error))
          {
            musica::Error delete_error;
            musica::DeleteProfile(profile, &delete_error);
            musica::DeleteError(&delete_error);
          }
          check_error(error, "Error creating profile");
          if (!profile)
            throw std::runtime_error("Profile creation returned null pointer");
          return profile;
        });

    mod.method(
        "cpp_delete_profile",
        [](musica::Profile* profile)
        {
          if (!profile)
            return;
          musica::Error error;
          musica::DeleteProfile(profile, &error);
          report_error(error, "Error deleting Profile");
        });

    // ── Profile accessors ────────────────────────────────────────────────
    mod.method(
        "cpp_profile_name",
        [](musica::Profile* profile)
        {
          check_not_null(profile, "Profile");
          musica::Error error;
          std::string name = profile->GetName(&error);
          check_error(error, "Error getting profile name");
          return name;
        });

    mod.method(
        "cpp_profile_units",
        [](musica::Profile* profile)
        {
          check_not_null(profile, "Profile");
          musica::Error error;
          std::string units = profile->GetUnits(&error);
          check_error(error, "Error getting profile units");
          return units;
        });

    mod.method(
        "cpp_profile_num_sections",
        [](musica::Profile* profile)
        {
          check_not_null(profile, "Profile");
          musica::Error error;
          std::size_t num_sections = profile->GetNumberOfSections(&error);
          check_error(error, "Error getting number of profile sections");
          return static_cast<int64_t>(num_sections);
        });

    // The three pointer functions return the address of the TUV-x array as an
    // integer, the same zero-copy approach used for the Grid edges/midpoints.
    // These are for READING only; see the note above the Grid pointer
    // methods on why writes must go through cpp_profile_set_*! instead.
    mod.method(
        "cpp_profile_edge_values_pointer",
        [](musica::Profile* profile)
        {
          check_not_null(profile, "Profile");
          musica::Error error;
          double* values = profile->GetEdgeValuesPointer(&error);
          check_error(error, "Error getting profile edge values pointer");
          if (!values)
            throw std::runtime_error("Profile edge values pointer is null");
          return static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(values));
        });

    mod.method(
        "cpp_profile_midpoint_values_pointer",
        [](musica::Profile* profile)
        {
          check_not_null(profile, "Profile");
          musica::Error error;
          double* values = profile->GetMidpointValuesPointer(&error);
          check_error(error, "Error getting profile midpoint values pointer");
          if (!values)
            throw std::runtime_error("Profile midpoint values pointer is null");
          return static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(values));
        });

    mod.method(
        "cpp_profile_layer_densities_pointer",
        [](musica::Profile* profile)
        {
          check_not_null(profile, "Profile");
          musica::Error error;
          double* values = profile->GetLayerDensitiesPointer(&error);
          check_error(error, "Error getting profile layer densities pointer");
          if (!values)
            throw std::runtime_error("Profile layer densities pointer is null");
          return static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(values));
        });

    mod.method(
        "cpp_profile_set_edge_values!",
        [](musica::Profile* profile, jlcxx::ArrayRef<double> values)
        {
          check_not_null(profile, "Profile");
          musica::Error error;
          profile->SetEdgeValues(values.data(), values.size(), &error);
          check_error(error, "Error setting profile edge values");
        });

    mod.method(
        "cpp_profile_set_midpoint_values!",
        [](musica::Profile* profile, jlcxx::ArrayRef<double> values)
        {
          check_not_null(profile, "Profile");
          musica::Error error;
          profile->SetMidpointValues(values.data(), values.size(), &error);
          check_error(error, "Error setting profile midpoint values");
        });

    mod.method(
        "cpp_profile_set_layer_densities!",
        [](musica::Profile* profile, jlcxx::ArrayRef<double> values)
        {
          check_not_null(profile, "Profile");
          musica::Error error;
          profile->SetLayerDensities(values.data(), values.size(), &error);
          check_error(error, "Error setting profile layer densities");
        });

    mod.method(
        "cpp_profile_exo_layer_density",
        [](musica::Profile* profile)
        {
          check_not_null(profile, "Profile");
          musica::Error error;
          double density = profile->GetExoLayerDensity(&error);
          check_error(error, "Error getting profile exo layer density");
          return density;
        });

    mod.method(
        "cpp_profile_set_exo_layer_density!",
        [](musica::Profile* profile, double exo_layer_density)
        {
          check_not_null(profile, "Profile");
          musica::Error error;
          profile->SetExoLayerDensity(exo_layer_density, &error);
          check_error(error, "Error setting profile exo layer density");
        });

    mod.method(
        "cpp_profile_calculate_exo_layer_density!",
        [](musica::Profile* profile, double scale_height)
        {
          check_not_null(profile, "Profile");
          musica::Error error;
          profile->CalculateExoLayerDensity(scale_height, &error);
          check_error(error, "Error calculating profile exo layer density");
        });

    // ── ProfileMap creation / deletion ───────────────────────────────────
    mod.method(
        "cpp_create_profile_map",
        []()
        {
          musica::Error error;
          musica::ProfileMap* profile_map = musica::CreateProfileMap(&error);
          // CreateProfileMap returns the object even when it sets an error.
          if (!musica::IsSuccess(error))
          {
            musica::Error delete_error;
            musica::DeleteProfileMap(profile_map, &delete_error);
            musica::DeleteError(&delete_error);
          }
          check_error(error, "Error creating profile map");
          if (!profile_map)
            throw std::runtime_error("Profile map creation returned null pointer");
          return profile_map;
        });

    mod.method(
        "cpp_delete_profile_map",
        [](musica::ProfileMap* profile_map)
        {
          if (!profile_map)
            return;
          musica::Error error;
          musica::DeleteProfileMap(profile_map, &error);
          report_error(error, "Error deleting ProfileMap");
        });

    // ── ProfileMap accessors ─────────────────────────────────────────────
    mod.method(
        "cpp_profile_map_add_profile!",
        [](musica::ProfileMap* profile_map, musica::Profile* profile)
        {
          check_not_null(profile_map, "ProfileMap");
          check_not_null(profile, "Profile");
          musica::Error error;
          profile_map->AddProfile(profile, &error);
          check_error(error, "Error adding profile to profile map");
        });

    // Both getters return a Profile that the caller owns. The Julia wrapper
    // takes ownership and deletes it in a finalizer.
    mod.method(
        "cpp_profile_map_get_profile",
        [](musica::ProfileMap* profile_map, const std::string& name, const std::string& units)
        {
          check_not_null(profile_map, "ProfileMap");
          musica::Error error;
          musica::Profile* profile = profile_map->GetProfile(name.c_str(), units.c_str(), &error);
          check_error(error, "Error getting profile");
          if (!profile)
            throw std::runtime_error("Profile '" + name + "' [" + units + "] not found in the profile map");
          return profile;
        });

    mod.method(
        "cpp_profile_map_get_profile_by_index",
        [](musica::ProfileMap* profile_map, int64_t index)
        {
          check_not_null(profile_map, "ProfileMap");
          if (index < 0)
            throw std::out_of_range("Profile index " + std::to_string(index) + " is negative");
          musica::Error error;
          musica::Profile* profile = profile_map->GetProfileByIndex(static_cast<std::size_t>(index), &error);
          check_error(error, "Error getting profile by index");
          if (!profile)
            throw std::out_of_range("Profile index " + std::to_string(index) + " is out of range");
          return profile;
        });

    mod.method(
        "cpp_profile_map_remove_profile!",
        [](musica::ProfileMap* profile_map, const std::string& name, const std::string& units)
        {
          check_not_null(profile_map, "ProfileMap");
          musica::Error error;
          profile_map->RemoveProfile(name.c_str(), units.c_str(), &error);
          check_error(error, "Error removing profile");
        });

    mod.method(
        "cpp_profile_map_remove_profile_by_index!",
        [](musica::ProfileMap* profile_map, int64_t index)
        {
          check_not_null(profile_map, "ProfileMap");
          if (index < 0)
            throw std::out_of_range("Profile index " + std::to_string(index) + " is negative");
          musica::Error error;
          profile_map->RemoveProfileByIndex(static_cast<std::size_t>(index), &error);
          check_error(error, "Error removing profile by index");
        });

    mod.method(
        "cpp_profile_map_number_of_profiles",
        [](musica::ProfileMap* profile_map)
        {
          check_not_null(profile_map, "ProfileMap");
          musica::Error error;
          std::size_t num_profiles = profile_map->GetNumberOfProfiles(&error);
          check_error(error, "Error getting number of profiles");
          return static_cast<int64_t>(num_profiles);
        });

    // ── Radiator creation / deletion ─────────────────────────────────────
    mod.method(
        "cpp_create_radiator",
        [](const std::string& name, musica::Grid* height_grid, musica::Grid* wavelength_grid)
        {
          check_not_null(height_grid, "Grid");
          check_not_null(wavelength_grid, "Grid");
          musica::Error error;
          musica::Radiator* radiator = musica::CreateRadiator(name.c_str(), height_grid, wavelength_grid, &error);
          // CreateRadiator returns the object even when it sets an error, so
          // delete the partly built radiator before the exception leaves this
          // function.
          if (!musica::IsSuccess(error))
          {
            musica::Error delete_error;
            musica::DeleteRadiator(radiator, &delete_error);
            musica::DeleteError(&delete_error);
          }
          check_error(error, "Error creating radiator");
          if (!radiator)
            throw std::runtime_error("Radiator creation returned null pointer");
          return radiator;
        });

    mod.method(
        "cpp_delete_radiator",
        [](musica::Radiator* radiator)
        {
          if (!radiator)
            return;
          musica::Error error;
          musica::DeleteRadiator(radiator, &error);
          report_error(error, "Error deleting Radiator");
        });

    // ── Radiator accessors ───────────────────────────────────────────────
    mod.method(
        "cpp_radiator_name",
        [](musica::Radiator* radiator)
        {
          check_not_null(radiator, "Radiator");
          musica::Error error;
          std::string name = radiator->GetName(&error);
          check_error(error, "Error getting radiator name");
          return name;
        });

    mod.method(
        "cpp_radiator_num_height_sections",
        [](musica::Radiator* radiator)
        {
          check_not_null(radiator, "Radiator");
          musica::Error error;
          std::size_t num_sections = radiator->GetNumberOfHeightSections(&error);
          check_error(error, "Error getting number of radiator height sections");
          return static_cast<int64_t>(num_sections);
        });

    mod.method(
        "cpp_radiator_num_wavelength_sections",
        [](musica::Radiator* radiator)
        {
          check_not_null(radiator, "Radiator");
          musica::Error error;
          std::size_t num_sections = radiator->GetNumberOfWavelengthSections(&error);
          check_error(error, "Error getting number of radiator wavelength sections");
          return static_cast<int64_t>(num_sections);
        });

    // The three pointer functions return the address of the TUV-x array as an
    // integer, the same zero-copy approach used for the Grid and Profile
    // arrays. Each array is height-fastest in memory (row-major with height
    // as the trailing index), which is exactly Julia's column-major layout
    // for a (num_height_sections, num_wavelength_sections) matrix, so no
    // transpose is needed on the Julia side. The number of streams is
    // currently fixed at 1 in TUV-x, so asymmetry factors are exposed as a
    // 2D array. These are for READING only; see the note above the Grid
    // pointer methods on why writes must go through cpp_radiator_set_*!
    // instead.
    mod.method(
        "cpp_radiator_optical_depths_pointer",
        [](musica::Radiator* radiator)
        {
          check_not_null(radiator, "Radiator");
          musica::Error error;
          double* values = radiator->GetOpticalDepthsPointer(&error);
          check_error(error, "Error getting radiator optical depths pointer");
          if (!values)
            throw std::runtime_error("Radiator optical depths pointer is null");
          return static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(values));
        });

    mod.method(
        "cpp_radiator_single_scattering_albedos_pointer",
        [](musica::Radiator* radiator)
        {
          check_not_null(radiator, "Radiator");
          musica::Error error;
          double* values = radiator->GetSingleScatteringAlbedosPointer(&error);
          check_error(error, "Error getting radiator single scattering albedos pointer");
          if (!values)
            throw std::runtime_error("Radiator single scattering albedos pointer is null");
          return static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(values));
        });

    mod.method(
        "cpp_radiator_asymmetry_factors_pointer",
        [](musica::Radiator* radiator)
        {
          check_not_null(radiator, "Radiator");
          musica::Error error;
          double* values = radiator->GetAsymmetryFactorsPointer(&error);
          check_error(error, "Error getting radiator asymmetry factors pointer");
          if (!values)
            throw std::runtime_error("Radiator asymmetry factors pointer is null");
          return static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(values));
        });

    mod.method(
        "cpp_radiator_set_optical_depths!",
        [](musica::Radiator* radiator, jlcxx::ArrayRef<double> values)
        {
          check_not_null(radiator, "Radiator");
          musica::Error error;
          std::size_t num_h = radiator->GetNumberOfHeightSections(&error);
          check_error(error, "Error getting number of radiator height sections");
          std::size_t num_w = radiator->GetNumberOfWavelengthSections(&error);
          check_error(error, "Error getting number of radiator wavelength sections");
          radiator->SetOpticalDepths(values.data(), num_h, num_w, &error);
          check_error(error, "Error setting radiator optical depths");
        });

    mod.method(
        "cpp_radiator_set_single_scattering_albedos!",
        [](musica::Radiator* radiator, jlcxx::ArrayRef<double> values)
        {
          check_not_null(radiator, "Radiator");
          musica::Error error;
          std::size_t num_h = radiator->GetNumberOfHeightSections(&error);
          check_error(error, "Error getting number of radiator height sections");
          std::size_t num_w = radiator->GetNumberOfWavelengthSections(&error);
          check_error(error, "Error getting number of radiator wavelength sections");
          radiator->SetSingleScatteringAlbedos(values.data(), num_h, num_w, &error);
          check_error(error, "Error setting radiator single scattering albedos");
        });

    mod.method(
        "cpp_radiator_set_asymmetry_factors!",
        [](musica::Radiator* radiator, jlcxx::ArrayRef<double> values)
        {
          check_not_null(radiator, "Radiator");
          musica::Error error;
          std::size_t num_h = radiator->GetNumberOfHeightSections(&error);
          check_error(error, "Error getting number of radiator height sections");
          std::size_t num_w = radiator->GetNumberOfWavelengthSections(&error);
          check_error(error, "Error getting number of radiator wavelength sections");
          constexpr std::size_t num_streams = 1;
          radiator->SetAsymmetryFactors(values.data(), num_h, num_w, num_streams, &error);
          check_error(error, "Error setting radiator asymmetry factors");
        });

    // ── RadiatorMap creation / deletion ──────────────────────────────────
    mod.method(
        "cpp_create_radiator_map",
        []()
        {
          musica::Error error;
          musica::RadiatorMap* radiator_map = musica::CreateRadiatorMap(&error);
          // CreateRadiatorMap returns the object even when it sets an error.
          if (!musica::IsSuccess(error))
          {
            musica::Error delete_error;
            musica::DeleteRadiatorMap(radiator_map, &delete_error);
            musica::DeleteError(&delete_error);
          }
          check_error(error, "Error creating radiator map");
          if (!radiator_map)
            throw std::runtime_error("Radiator map creation returned null pointer");
          return radiator_map;
        });

    mod.method(
        "cpp_delete_radiator_map",
        [](musica::RadiatorMap* radiator_map)
        {
          if (!radiator_map)
            return;
          musica::Error error;
          musica::DeleteRadiatorMap(radiator_map, &error);
          report_error(error, "Error deleting RadiatorMap");
        });

    // ── RadiatorMap accessors ────────────────────────────────────────────
    mod.method(
        "cpp_radiator_map_add_radiator!",
        [](musica::RadiatorMap* radiator_map, musica::Radiator* radiator)
        {
          check_not_null(radiator_map, "RadiatorMap");
          check_not_null(radiator, "Radiator");
          musica::Error error;
          radiator_map->AddRadiator(radiator, &error);
          check_error(error, "Error adding radiator to radiator map");
        });

    // Both getters return a Radiator that the caller owns. The Julia wrapper
    // takes ownership and deletes it in a finalizer.
    mod.method(
        "cpp_radiator_map_get_radiator",
        [](musica::RadiatorMap* radiator_map, const std::string& name)
        {
          check_not_null(radiator_map, "RadiatorMap");
          musica::Error error;
          musica::Radiator* radiator = radiator_map->GetRadiator(name.c_str(), &error);
          check_error(error, "Error getting radiator");
          if (!radiator)
            throw std::runtime_error("Radiator '" + name + "' not found in the radiator map");
          return radiator;
        });

    mod.method(
        "cpp_radiator_map_get_radiator_by_index",
        [](musica::RadiatorMap* radiator_map, int64_t index)
        {
          check_not_null(radiator_map, "RadiatorMap");
          if (index < 0)
            throw std::out_of_range("Radiator index " + std::to_string(index) + " is negative");
          musica::Error error;
          musica::Radiator* radiator = radiator_map->GetRadiatorByIndex(static_cast<std::size_t>(index), &error);
          check_error(error, "Error getting radiator by index");
          if (!radiator)
            throw std::out_of_range("Radiator index " + std::to_string(index) + " is out of range");
          return radiator;
        });

    mod.method(
        "cpp_radiator_map_remove_radiator!",
        [](musica::RadiatorMap* radiator_map, const std::string& name)
        {
          check_not_null(radiator_map, "RadiatorMap");
          musica::Error error;
          radiator_map->RemoveRadiator(name.c_str(), &error);
          check_error(error, "Error removing radiator");
        });

    mod.method(
        "cpp_radiator_map_remove_radiator_by_index!",
        [](musica::RadiatorMap* radiator_map, int64_t index)
        {
          check_not_null(radiator_map, "RadiatorMap");
          if (index < 0)
            throw std::out_of_range("Radiator index " + std::to_string(index) + " is negative");
          musica::Error error;
          radiator_map->RemoveRadiatorByIndex(static_cast<std::size_t>(index), &error);
          check_error(error, "Error removing radiator by index");
        });

    mod.method(
        "cpp_radiator_map_number_of_radiators",
        [](musica::RadiatorMap* radiator_map)
        {
          check_not_null(radiator_map, "RadiatorMap");
          musica::Error error;
          std::size_t num_radiators = radiator_map->GetNumberOfRadiators(&error);
          check_error(error, "Error getting number of radiators");
          return static_cast<int64_t>(num_radiators);
        });

    // ── TUVX creation / deletion ──────────────────────────────────────────
    mod.method("cpp_create_tuvx", []() { return new musica::TUVX(); });

    mod.method(
        "cpp_delete_tuvx",
        [](musica::TUVX* tuvx)
        {
          if (!tuvx)
            return;
          delete tuvx;
        });

    mod.method(
        "cpp_tuvx_create_from_file!",
        [](musica::TUVX* tuvx,
           const std::string& config_path,
           musica::GridMap* grids,
           musica::ProfileMap* profiles,
           musica::RadiatorMap* radiators)
        {
          check_not_null(tuvx, "TUVX");
          check_not_null(grids, "GridMap");
          check_not_null(profiles, "ProfileMap");
          check_not_null(radiators, "RadiatorMap");
          musica::Error error;
          tuvx->Create(config_path.c_str(), grids, profiles, radiators, &error);
          check_error(error, "Error creating TUV-x instance from config file");
        });

    mod.method(
        "cpp_tuvx_create_from_string!",
        [](musica::TUVX* tuvx,
           const std::string& config_string,
           musica::GridMap* grids,
           musica::ProfileMap* profiles,
           musica::RadiatorMap* radiators)
        {
          check_not_null(tuvx, "TUVX");
          check_not_null(grids, "GridMap");
          check_not_null(profiles, "ProfileMap");
          check_not_null(radiators, "RadiatorMap");
          musica::Error error;
          tuvx->CreateFromConfigString(config_string.c_str(), grids, profiles, radiators, &error);
          check_error(error, "Error creating TUV-x instance from config string");
        });

    // ── TUVX map accessors ────────────────────────────────────────────────
    // Each getter returns a map that the caller owns. The underlying map
    // classes track whether they own the wrapped TUV-x data (see GridMap,
    // ProfileMap, RadiatorMap), so deleting this wrapper never frees the live
    // core data — it's always safe for the Julia wrapper to delete it in a
    // finalizer, the same as any other map.
    mod.method(
        "cpp_tuvx_get_grid_map",
        [](musica::TUVX* tuvx)
        {
          check_not_null(tuvx, "TUVX");
          musica::Error error;
          musica::GridMap* grid_map = tuvx->GetGridMap(&error);
          check_error(error, "Error getting grid map from TUV-x instance");
          return grid_map;
        });

    mod.method(
        "cpp_tuvx_get_profile_map",
        [](musica::TUVX* tuvx)
        {
          check_not_null(tuvx, "TUVX");
          musica::Error error;
          musica::ProfileMap* profile_map = tuvx->GetProfileMap(&error);
          check_error(error, "Error getting profile map from TUV-x instance");
          return profile_map;
        });

    mod.method(
        "cpp_tuvx_get_radiator_map",
        [](musica::TUVX* tuvx)
        {
          check_not_null(tuvx, "TUVX");
          musica::Error error;
          musica::RadiatorMap* radiator_map = tuvx->GetRadiatorMap(&error);
          check_error(error, "Error getting radiator map from TUV-x instance");
          return radiator_map;
        });

    // ── TUVX count getters ────────────────────────────────────────────────
    // These throw std::runtime_error directly rather than taking an Error*;
    // jlcxx converts the exception to a Julia ErrorException the same way.
    mod.method(
        "cpp_tuvx_photolysis_rate_constant_count",
        [](musica::TUVX* tuvx)
        {
          check_not_null(tuvx, "TUVX");
          return static_cast<int64_t>(tuvx->GetPhotolysisRateConstantCount());
        });

    mod.method(
        "cpp_tuvx_heating_rate_count",
        [](musica::TUVX* tuvx)
        {
          check_not_null(tuvx, "TUVX");
          return static_cast<int64_t>(tuvx->GetHeatingRateCount());
        });

    mod.method(
        "cpp_tuvx_dose_rate_count",
        [](musica::TUVX* tuvx)
        {
          check_not_null(tuvx, "TUVX");
          return static_cast<int64_t>(tuvx->GetDoseRateCount());
        });

    mod.method(
        "cpp_tuvx_num_height_midpoints",
        [](musica::TUVX* tuvx)
        {
          check_not_null(tuvx, "TUVX");
          return static_cast<int64_t>(tuvx->GetNumberOfHeightMidpoints());
        });

    mod.method(
        "cpp_tuvx_num_wavelength_midpoints",
        [](musica::TUVX* tuvx)
        {
          check_not_null(tuvx, "TUVX");
          return static_cast<int64_t>(tuvx->GetNumberOfWavelengthMidpoints());
        });

    // ── TUVX rate orderings ───────────────────────────────────────────────
    // Each ordering is returned as parallel name/index arrays, the same
    // convention used for the MICM species and rate parameter orderings.
    mod.method(
        "cpp_tuvx_photolysis_rate_names",
        [](musica::TUVX* tuvx)
        {
          check_not_null(tuvx, "TUVX");
          musica::Error error;
          musica::Mappings mappings{};
          tuvx->GetPhotolysisRateConstantsOrdering(&mappings, &error);
          auto names = mapping_names(mappings);
          musica::DeleteMappings(&mappings);
          check_error(error, "Error getting photolysis rate constants ordering");
          return names;
        });

    mod.method(
        "cpp_tuvx_photolysis_rate_indices",
        [](musica::TUVX* tuvx)
        {
          check_not_null(tuvx, "TUVX");
          musica::Error error;
          musica::Mappings mappings{};
          tuvx->GetPhotolysisRateConstantsOrdering(&mappings, &error);
          auto indices = mapping_indices(mappings);
          musica::DeleteMappings(&mappings);
          check_error(error, "Error getting photolysis rate constants ordering");
          return indices;
        });

    mod.method(
        "cpp_tuvx_heating_rate_names",
        [](musica::TUVX* tuvx)
        {
          check_not_null(tuvx, "TUVX");
          musica::Error error;
          musica::Mappings mappings{};
          tuvx->GetHeatingRatesOrdering(&mappings, &error);
          auto names = mapping_names(mappings);
          musica::DeleteMappings(&mappings);
          check_error(error, "Error getting heating rates ordering");
          return names;
        });

    mod.method(
        "cpp_tuvx_heating_rate_indices",
        [](musica::TUVX* tuvx)
        {
          check_not_null(tuvx, "TUVX");
          musica::Error error;
          musica::Mappings mappings{};
          tuvx->GetHeatingRatesOrdering(&mappings, &error);
          auto indices = mapping_indices(mappings);
          musica::DeleteMappings(&mappings);
          check_error(error, "Error getting heating rates ordering");
          return indices;
        });

    mod.method(
        "cpp_tuvx_dose_rate_names",
        [](musica::TUVX* tuvx)
        {
          check_not_null(tuvx, "TUVX");
          musica::Error error;
          musica::Mappings mappings{};
          tuvx->GetDoseRatesOrdering(&mappings, &error);
          auto names = mapping_names(mappings);
          musica::DeleteMappings(&mappings);
          check_error(error, "Error getting dose rates ordering");
          return names;
        });

    mod.method(
        "cpp_tuvx_dose_rate_indices",
        [](musica::TUVX* tuvx)
        {
          check_not_null(tuvx, "TUVX");
          musica::Error error;
          musica::Mappings mappings{};
          tuvx->GetDoseRatesOrdering(&mappings, &error);
          auto indices = mapping_indices(mappings);
          musica::DeleteMappings(&mappings);
          check_error(error, "Error getting dose rates ordering");
          return indices;
        });

    // ── TUVX run ──────────────────────────────────────────────────────────
    // The caller (Julia) pre-allocates each output buffer using the count
    // getters above and passes it in to be filled in place, the same
    // zero-extra-copy approach used for the Grid, Profile, and Radiator
    // arrays. Sizes are checked here because an under-sized buffer would
    // make TUV-x write out of bounds.
    mod.method(
        "cpp_tuvx_run!",
        [](musica::TUVX* tuvx,
           double solar_zenith_angle,
           double earth_sun_distance,
           jlcxx::ArrayRef<double> photolysis_rate_constants,
           jlcxx::ArrayRef<double> heating_rates,
           jlcxx::ArrayRef<double> dose_rates,
           jlcxx::ArrayRef<double> actinic_flux,
           jlcxx::ArrayRef<double> spectral_irradiance)
        {
          check_not_null(tuvx, "TUVX");
          std::size_t n_edges = static_cast<std::size_t>(tuvx->GetNumberOfHeightMidpoints()) + 1;
          std::size_t n_wavelengths = static_cast<std::size_t>(tuvx->GetNumberOfWavelengthMidpoints());
          std::size_t n_photolysis = static_cast<std::size_t>(tuvx->GetPhotolysisRateConstantCount());
          std::size_t n_heating = static_cast<std::size_t>(tuvx->GetHeatingRateCount());
          std::size_t n_dose = static_cast<std::size_t>(tuvx->GetDoseRateCount());
          if (photolysis_rate_constants.size() != n_photolysis * n_edges)
            throw std::runtime_error("photolysis_rate_constants buffer has the wrong size");
          if (heating_rates.size() != n_heating * n_edges)
            throw std::runtime_error("heating_rates buffer has the wrong size");
          if (dose_rates.size() != n_dose * n_edges)
            throw std::runtime_error("dose_rates buffer has the wrong size");
          if (actinic_flux.size() != n_wavelengths * n_edges * 3)
            throw std::runtime_error("actinic_flux buffer has the wrong size");
          if (spectral_irradiance.size() != n_wavelengths * n_edges * 3)
            throw std::runtime_error("spectral_irradiance buffer has the wrong size");

          musica::Error error;
          tuvx->Run(
              solar_zenith_angle,
              earth_sun_distance,
              photolysis_rate_constants.data(),
              heating_rates.data(),
              dose_rates.data(),
              actinic_flux.data(),
              spectral_irradiance.data(),
              &error);
          check_error(error, "Error running TUV-x");
        });
  }

  [[maybe_unused]] const musica_julia::Registrar registrar(register_tuvx);
}  // namespace
