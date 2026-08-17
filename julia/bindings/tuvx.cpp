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
#include "registration.hpp"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

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

  void check_grid_not_null(musica::Grid* grid)
  {
    if (!grid)
      throw std::runtime_error("Grid pointer is null");
  }

  void check_grid_map_not_null(musica::GridMap* grid_map)
  {
    if (!grid_map)
      throw std::runtime_error("GridMap pointer is null");
  }

  void register_tuvx(jlcxx::Module& mod)
  {
    // ── Version ──────────────────────────────────────────────────────────
    // Calls into the TUV-x Fortran library.
    mod.method("get_tuvx_version", []() { return musica::TUVX::GetVersion(); });

    // ── Opaque types ─────────────────────────────────────────────────────
    mod.add_type<musica::Grid>("CppGrid");
    mod.add_type<musica::GridMap>("CppGridMap");

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
          check_grid_not_null(grid);
          musica::Error error;
          std::string name = grid->GetName(&error);
          check_error(error, "Error getting grid name");
          return name;
        });

    mod.method(
        "cpp_grid_units",
        [](musica::Grid* grid)
        {
          check_grid_not_null(grid);
          musica::Error error;
          std::string units = grid->GetUnits(&error);
          check_error(error, "Error getting grid units");
          return units;
        });

    mod.method(
        "cpp_grid_num_sections",
        [](musica::Grid* grid)
        {
          check_grid_not_null(grid);
          musica::Error error;
          std::size_t num_sections = grid->GetNumberOfSections(&error);
          check_error(error, "Error getting number of grid sections");
          return static_cast<int64_t>(num_sections);
        });

    // The two pointer functions return the address of the TUV-x array as an
    // integer. Julia wraps that address with unsafe_wrap to get a zero-copy
    // view, the same way the Python bindings hand out a numpy view. An integer
    // avoids any dependence on how jlcxx maps a raw double* into Julia.
    mod.method(
        "cpp_grid_edges_pointer",
        [](musica::Grid* grid)
        {
          check_grid_not_null(grid);
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
          check_grid_not_null(grid);
          musica::Error error;
          double* midpoints = grid->GetMidpointsPointer(&error);
          check_error(error, "Error getting grid midpoints pointer");
          if (!midpoints)
            throw std::runtime_error("Grid midpoints pointer is null");
          return static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(midpoints));
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
          check_grid_map_not_null(grid_map);
          check_grid_not_null(grid);
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
          check_grid_map_not_null(grid_map);
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
          check_grid_map_not_null(grid_map);
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
          check_grid_map_not_null(grid_map);
          musica::Error error;
          grid_map->RemoveGrid(name.c_str(), units.c_str(), &error);
          check_error(error, "Error removing grid");
        });

    mod.method(
        "cpp_grid_map_remove_grid_by_index!",
        [](musica::GridMap* grid_map, int64_t index)
        {
          check_grid_map_not_null(grid_map);
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
          check_grid_map_not_null(grid_map);
          musica::Error error;
          std::size_t num_grids = grid_map->GetNumberOfGrids(&error);
          check_error(error, "Error getting number of grids");
          return static_cast<int64_t>(num_grids);
        });
  }

  [[maybe_unused]] const musica_julia::Registrar registrar(register_tuvx);
}  // namespace
