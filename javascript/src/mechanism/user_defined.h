// Copyright (C) 2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#ifndef MUSICA_USER_DEFINED_H
#define MUSICA_USER_DEFINED_H

#include <napi.h>
#include <mechanism_configuration/v1/reaction_types.hpp>

namespace musica
{

  /// @brief Wrapper class for mechanism_configuration::v1::types::UserDefined
  ///
  /// Represents a user-defined reaction rate constant with customizable parameters.
  class UserDefined : public Napi::ObjectWrap<UserDefined>
  {
   public:
    /// @brief Constructor
    UserDefined(const Napi::CallbackInfo& info);

    /// @brief Getter for name property
    Napi::Value GetName(const Napi::CallbackInfo& info);
    void SetName(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for scaling_factor
    Napi::Value GetScalingFactor(const Napi::CallbackInfo& info);
    void SetScalingFactor(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for reactants array
    Napi::Value GetReactants(const Napi::CallbackInfo& info);
    void SetReactants(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for products array
    Napi::Value GetProducts(const Napi::CallbackInfo& info);
    void SetProducts(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for gas_phase
    Napi::Value GetGasPhase(const Napi::CallbackInfo& info);
    void SetGasPhase(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Serialize reaction to object
    Napi::Value Serialize(const Napi::CallbackInfo& info);

    /// @brief Get the internal C++ user_defined object
    const mechanism_configuration::v1::types::UserDefined& GetInternalUserDefined() const;

    /// @brief Initialize the class and export to JavaScript
    static Napi::Function GetClass(Napi::Env env);

   private:
    mechanism_configuration::v1::types::UserDefined user_defined_;
  };

}  // namespace musica

#endif  // MUSICA_USER_DEFINED_H
