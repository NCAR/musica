// Copyright (C) 2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#ifndef MUSICA_REACTION_COMPONENT_H
#define MUSICA_REACTION_COMPONENT_H

#include <napi.h>
#include <mechanism_configuration/v1/types.hpp>

namespace musica
{

  /// @brief Wrapper class for mechanism_configuration::v1::types::ReactionComponent
  class ReactionComponent : public Napi::ObjectWrap<ReactionComponent>
  {
   public:
    /// @brief Constructor
    ReactionComponent(const Napi::CallbackInfo& info);

    /// @brief Getter for species_name property
    Napi::Value GetSpeciesName(const Napi::CallbackInfo& info);
    /// @brief Setter for species_name property
    void SetSpeciesName(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for coefficient property
    Napi::Value GetCoefficient(const Napi::CallbackInfo& info);
    /// @brief Setter for coefficient property
    void SetCoefficient(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Serialize reaction component to object
    Napi::Value Serialize(const Napi::CallbackInfo& info);

    /// @brief Get the internal C++ reaction component object
    const mechanism_configuration::v1::types::ReactionComponent& GetInternalComponent() const;

    /// @brief Initialize the class and export to JavaScript
    static Napi::Function GetClass(Napi::Env env);

   private:
    mechanism_configuration::v1::types::ReactionComponent component_;
  };

}  // namespace musica

#endif  // MUSICA_REACTION_COMPONENT_H
