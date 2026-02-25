// Copyright (C) 2023-2026 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// Runtime registry for JavaScript lambda-rate-constant callbacks.
//
// In a native (non-WASM) build the dispatcher is never set and
// InvokeLambdaCallback always returns 0.0.  In the WASM build the JS bindings
// call SetLambdaCallbackDispatcher() once at module initialisation and
// thereafter every registered JS function can be invoked from C++.
#pragma once

#include <micm/system/conditions.hpp>

#include <functional>
#include <map>
#include <string>

namespace musica
{
  /// @brief Return a mutable reference to the global label→callback-id map.
  ///
  /// The map is keyed by the reaction label (e.g. "Lambda.mine") and maps to
  /// the integer ID returned by registerReactionRateCallback on the JS side.
  std::map<std::string, int>& GetLambdaCallbackIds();

  /// @brief Set the platform-specific dispatcher used to call registered
  ///        callbacks.
  ///
  /// The dispatcher is called with (callback_id, T, P, air_density) and must
  /// return the computed rate-constant value.  This is set once from the WASM
  /// bindings after the JS callback vector has been populated.
  void SetLambdaCallbackDispatcher(std::function<double(int, double, double, double)> dispatcher);

  /// @brief Invoke the JS callback registered for @p label.
  ///
  /// If no callback has been registered (either because the label is unknown or
  /// because the dispatcher has not been set) this returns 0.0.
  double InvokeLambdaCallback(const std::string& label, const micm::Conditions& conditions);

}  // namespace musica
