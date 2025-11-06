// Copyright (C) 2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#ifndef MUSICA_MECHANISM_H
#define MUSICA_MECHANISM_H

#include <napi.h>
#include <mechanism_configuration/v1/mechanism.hpp>

namespace musica
{

  /// @brief Wrapper class for mechanism_configuration::v1::types::Mechanism
  ///
  /// This class provides a JavaScript interface to the C++ Mechanism type,
  /// allowing users to create, modify, and export mechanism configurations
  /// in JSON or YAML format.
  class Mechanism : public Napi::ObjectWrap<Mechanism>
  {
   public:
    /// @brief Constructor
    Mechanism(const Napi::CallbackInfo& info);

    /// @brief Getter for name property
    Napi::Value GetName(const Napi::CallbackInfo& info);
    /// @brief Setter for name property
    void SetName(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for species array
    Napi::Value GetSpecies(const Napi::CallbackInfo& info);
    /// @brief Setter for species array
    void SetSpecies(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for phases array
    Napi::Value GetPhases(const Napi::CallbackInfo& info);
    /// @brief Setter for phases array
    void SetPhases(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for reactions array
    Napi::Value GetReactions(const Napi::CallbackInfo& info);
    /// @brief Setter for reactions array
    void SetReactions(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Serialize mechanism to JSON object
    /// @return JavaScript object containing the complete mechanism configuration
    Napi::Value Serialize(const Napi::CallbackInfo& info);

    /// @brief Export mechanism to JSON or YAML file
    /// @param filename Path to the output file (.json, .yaml, or .yml extension)
    Napi::Value Export(const Napi::CallbackInfo& info);

    /// @brief Get the internal C++ mechanism object
    const mechanism_configuration::v1::types::Mechanism& GetInternalMechanism() const;

    /// @brief Initialize the class and export to JavaScript
    static Napi::Function GetClass(Napi::Env env);

   private:
    mechanism_configuration::v1::types::Mechanism mechanism_;
  };

}  // namespace musica

#endif  // MUSICA_MECHANISM_H
