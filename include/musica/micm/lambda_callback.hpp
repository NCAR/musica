// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Runtime registry for JavaScript lambda-rate-constant callbacks.
//
// In a native (non-WASM) build the registry is never populated and
// InvokeLambdaCallback always returns 0.0.  In the WASM build the JS bindings
// call SetLambdaCallback() for each reaction before the solver runs.
#pragma once

#include <micm/system/conditions.hpp>

#include <functional>
#include <string>

namespace musica
{
  /// @brief Register a callable for @p label.
  ///
  /// The function is called with the atmospheric conditions at solve time and
  /// must return the rate-constant value.  Passing a new function for the same
  /// label replaces the previous one.
  void SetLambdaCallback(const std::string& label, std::function<double(const micm::Conditions&)> fn);

  /// @brief Invoke the callable registered for @p label.
  ///
  /// Returns 0.0 if no callable has been registered for @p label.
  double InvokeLambdaCallback(const std::string& label, const micm::Conditions& conditions);

}  // namespace musica
