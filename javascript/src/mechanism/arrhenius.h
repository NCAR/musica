// Copyright (C) 2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#ifndef MUSICA_ARRHENIUS_H
#define MUSICA_ARRHENIUS_H

#include <napi.h>
#include <mechanism_configuration/v1/reaction_types.hpp>

namespace musica
{

  /// @brief Wrapper class for mechanism_configuration::v1::types::Arrhenius
  ///
  /// Represents an Arrhenius rate constant:
  /// k = A * exp( C / T ) * ( T / D )^B * exp( E * P )
  ///
  /// where:
  ///   k = rate constant
  ///   A = pre-exponential factor [(mol m-3)^(n-1)s-1]
  ///   B = temperature exponent [unitless]
  ///   C = exponential term [K-1]
  ///   D = reference temperature [K]
  ///   E = pressure scaling term [Pa-1]
  ///   T = temperature [K]
  ///   P = pressure [Pa]
  ///   n = number of reactants
  class Arrhenius : public Napi::ObjectWrap<Arrhenius>
  {
   public:
    /// @brief Constructor
    Arrhenius(const Napi::CallbackInfo& info);

    /// @brief Getter for name property
    Napi::Value GetName(const Napi::CallbackInfo& info);
    void SetName(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for A (pre-exponential factor)
    Napi::Value GetA(const Napi::CallbackInfo& info);
    void SetA(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for B (temperature exponent)
    Napi::Value GetB(const Napi::CallbackInfo& info);
    void SetB(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for C (exponential term)
    Napi::Value GetC(const Napi::CallbackInfo& info);
    void SetC(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for D (reference temperature)
    Napi::Value GetD(const Napi::CallbackInfo& info);
    void SetD(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for E (pressure scaling)
    Napi::Value GetE(const Napi::CallbackInfo& info);
    void SetE(const Napi::CallbackInfo& info, const Napi::Value& value);

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

    /// @brief Get the internal C++ arrhenius object
    const mechanism_configuration::v1::types::Arrhenius& GetInternalArrhenius() const;

    /// @brief Initialize the class and export to JavaScript
    static Napi::Function GetClass(Napi::Env env);

   private:
    mechanism_configuration::v1::types::Arrhenius arrhenius_;
  };

}  // namespace musica

#endif  // MUSICA_ARRHENIUS_H
