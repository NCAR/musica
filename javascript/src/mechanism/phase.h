// Copyright (C) 2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#ifndef MUSICA_PHASE_H
#define MUSICA_PHASE_H

#include <napi.h>
#include <mechanism_configuration/v1/types.hpp>

namespace musica
{

  /// @brief Wrapper class for mechanism_configuration::v1::types::Phase
  class Phase : public Napi::ObjectWrap<Phase>
  {
   public:
    /// @brief Constructor
    Phase(const Napi::CallbackInfo& info);

    /// @brief Getter for name property
    Napi::Value GetName(const Napi::CallbackInfo& info);
    /// @brief Setter for name property
    void SetName(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for species array
    Napi::Value GetSpecies(const Napi::CallbackInfo& info);
    /// @brief Setter for species array
    void SetSpecies(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Serialize phase to object
    Napi::Value Serialize(const Napi::CallbackInfo& info);

    /// @brief Get the internal C++ phase object
    const mechanism_configuration::v1::types::Phase& GetInternalPhase() const;

    /// @brief Initialize the class and export to JavaScript
    static Napi::Function GetClass(Napi::Env env);

   private:
    mechanism_configuration::v1::types::Phase phase_;
  };

}  // namespace musica

#endif  // MUSICA_PHASE_H
