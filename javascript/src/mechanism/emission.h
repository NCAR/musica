// Copyright (C) 2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#ifndef MUSICA_EMISSION_H
#define MUSICA_EMISSION_H

#include <napi.h>
#include <mechanism_configuration/v1/reaction_types.hpp>

namespace musica
{

  /// @brief Wrapper class for mechanism_configuration::v1::types::Emission
  ///
  /// Represents an emission reaction type with products and a scaling factor
  ///
  /// Properties:
  ///   scaling_factor = scaling factor to apply to user-provided rate constants
  ///   products = list of products
  ///   name = identifier (optional)
  ///   gas_phase = identifier indicating which gas phase the reaction takes place in
  ///   unknown_properties = unknown properties prefixed with __ (unmapped fields)
  class Emission : public Napi::ObjectWrap<Emission>
  {
   public:
    /// @brief Constructor
    Emission(const Napi::CallbackInfo& info);

    /// @brief Getter for name property
    Napi::Value GetName(const Napi::CallbackInfo& info);
    void SetName(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for scaling_factor
    Napi::Value GetScalingFactor(const Napi::CallbackInfo& info);
    void SetScalingFactor(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for products array
    Napi::Value GetProducts(const Napi::CallbackInfo& info);
    void SetProducts(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for gas_phase
    Napi::Value GetGasPhase(const Napi::CallbackInfo& info);
    void SetGasPhase(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Serialize reaction to object
    Napi::Value Serialize(const Napi::CallbackInfo& info);

    /// @brief Get the internal C++ emission object
    const mechanism_configuration::v1::types::Emission& GetInternalEmission() const;

    /// @brief Initialize the class and export to JavaScript
    static Napi::Function GetClass(Napi::Env env);

   private:
    mechanism_configuration::v1::types::Emission emission_;
  };

}  // namespace musica

#endif  // MUSICA_EMISSION_H
