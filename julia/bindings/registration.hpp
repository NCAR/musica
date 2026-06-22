// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Self-registration registry for the Julia bindings. Each component translation
// unit (micm.cpp, tuvx.cpp, ...) registers its binding function at static-init
// time, and define_julia_module() runs every registered function. This lets a
// component be added or omitted purely by compiling its .cpp (decided in CMake),
// with no preprocessor guards in the module entry point.
#pragma once

#include "jlcxx/jlcxx.hpp"

#include <functional>
#include <utility>
#include <vector>

namespace musica_julia
{
  /// @brief A per-component binding registration function.
  using BindingRegistration = std::function<void(jlcxx::Module&)>;

  /// @brief The registry of binding functions.
  ///
  /// Returned by reference from a function-local static so it is constructed on
  /// first use, avoiding the static-initialization-order fiasco between the
  /// Registrar objects in the component files and this registry.
  std::vector<BindingRegistration>& binding_registry();

  /// @brief Registers a binding function into the registry on construction.
  ///
  /// Declare one at file scope in each component file:
  ///   const musica_julia::Registrar registrar(register_my_component);
  struct Registrar
  {
    explicit Registrar(BindingRegistration fn)
    {
      binding_registry().push_back(std::move(fn));
    }
  };
}  // namespace musica_julia
