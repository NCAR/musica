// Copyright (C) 2025 University Corporation for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0

#ifndef MUSICA_SPECIES_H
#define MUSICA_SPECIES_H

#include <napi.h>
#include <mechanism_configuration/v1/types.hpp>

namespace musica
{

  /// @brief Wrapper class for mechanism_configuration::v1::types::Species
  class Species : public Napi::ObjectWrap<Species>
  {
   public:
    /// @brief Constructor
    Species(const Napi::CallbackInfo& info);

    /// @brief Getter for name property
    Napi::Value GetName(const Napi::CallbackInfo& info);
    /// @brief Setter for name property
    void SetName(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for molecular_weight_kg_mol property
    Napi::Value GetMolecularWeight(const Napi::CallbackInfo& info);
    /// @brief Setter for molecular_weight_kg_mol property
    void SetMolecularWeight(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for constant_concentration_mol_m3 property
    Napi::Value GetConstantConcentration(const Napi::CallbackInfo& info);
    /// @brief Setter for constant_concentration_mol_m3 property
    void SetConstantConcentration(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for constant_mixing_ratio_mol_mol property
    Napi::Value GetConstantMixingRatio(const Napi::CallbackInfo& info);
    /// @brief Setter for constant_mixing_ratio_mol_mol property
    void SetConstantMixingRatio(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for is_third_body property
    Napi::Value GetIsThirdBody(const Napi::CallbackInfo& info);
    /// @brief Setter for is_third_body property
    void SetIsThirdBody(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Getter for other_properties
    Napi::Value GetOtherProperties(const Napi::CallbackInfo& info);
    /// @brief Setter for other_properties
    void SetOtherProperties(const Napi::CallbackInfo& info, const Napi::Value& value);

    /// @brief Serialize species to object
    Napi::Value Serialize(const Napi::CallbackInfo& info);

    /// @brief Get the internal C++ species object
    const mechanism_configuration::v1::types::Species& GetInternalSpecies() const;

    /// @brief Initialize the class and export to JavaScript
    static Napi::Function GetClass(Napi::Env env);

   private:
    mechanism_configuration::v1::types::Species species_;
  };

}  // namespace musica

#endif  // MUSICA_SPECIES_H
