#include "types.h"

namespace constants = mechanism_configuration::constants;
namespace validation = mechanism_configuration::v1::validation;
using namespace mechanism_configuration::v1::types;

// ===== SpeciesClass registration =====
Napi::Object SpeciesClass::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "Species", {
        InstanceAccessor("name", &SpeciesClass::getName, &SpeciesClass::setName),
        InstanceAccessor("molecular_weight", &SpeciesClass::getMolecular_Weight, &SpeciesClass::setMolecular_Weight),
        InstanceAccessor("constant_concentration", &SpeciesClass::getConstant_Concentration, &SpeciesClass::setConstant_Concentration),
        InstanceAccessor("constant_mixing_ratio", &SpeciesClass::getConstant_Mixing_Ratio, &SpeciesClass::setConstant_Mixing_Ratio),
        InstanceAccessor("is_third_party", &SpeciesClass::getIs_Third_Party, &SpeciesClass::setIs_Third_Party),
        InstanceAccessor("other_properties", &SpeciesClass::getUnknown_Properties, &SpeciesClass::setUnknown_Properties)
    });

    exports.Set("Species", func);
    return exports;
}

// ===== SpeciesClass Constructor/Destructor =====
SpeciesClass::SpeciesClass(const Napi::CallbackInfo& info) : ObjectWrap(info) {
    spec = std::make_unique<Species>();
}

SpeciesClass::~SpeciesClass() = default;

// ===== Accessors =====
// name
Napi::Value SpeciesClass::getName(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, spec->name);
}

void SpeciesClass::setName(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (!value.IsString()) {
        Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
        return;
    }
    spec->name = value.As<Napi::String>();
}

// molecular_weight
Napi::Value SpeciesClass::getMolecular_Weight(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (spec->molecular_weight.has_value()) {
        return Napi::Number::New(env, spec->molecular_weight.value());
    }
    return env.Undefined();
}

void SpeciesClass::setMolecular_Weight(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (value.IsNull() || value.IsUndefined()) {
        spec->molecular_weight.reset();
        return;
    }
    if (!value.IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return;
    }
    spec->molecular_weight = value.As<Napi::Number>().DoubleValue();
}

// constant_concentration
Napi::Value SpeciesClass::getConstant_Concentration(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (spec->constant_concentration.has_value()) {
        return Napi::Number::New(env, spec->constant_concentration.value());
    }
    return env.Undefined();
}

void SpeciesClass::setConstant_Concentration(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (value.IsNull() || value.IsUndefined()) {
        spec->constant_concentration.reset();
        return;
    }
    if (!value.IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return;
    }
    spec->constant_concentration = value.As<Napi::Number>().DoubleValue();
}

// constant_mixing_ratio
Napi::Value SpeciesClass::getConstant_Mixing_Ratio(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (spec->constant_mixing_ratio.has_value()) {
        return Napi::Number::New(env, spec->constant_mixing_ratio.value());
    }
    return env.Undefined();
}

void SpeciesClass::setConstant_Mixing_Ratio(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (value.IsNull() || value.IsUndefined()) {
        spec->constant_mixing_ratio.reset();
        return;
    }
    if (!value.IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return;
    }
    spec->constant_mixing_ratio = value.As<Napi::Number>().DoubleValue();
}

// is_third_party
Napi::Value SpeciesClass::getIs_Third_Party(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (spec->is_third_body.has_value()) {
        return Napi::Boolean::New(env, spec->is_third_body.value());
    }
    return env.Undefined();
}

void SpeciesClass::setIs_Third_Party(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (value.IsNull() || value.IsUndefined()) {
        spec->is_third_body.reset();
        return;
    }
    if (!value.IsBoolean()) {
        Napi::TypeError::New(env, "Boolean expected").ThrowAsJavaScriptException();
        return;
    }
    spec->is_third_body = value.As<Napi::Boolean>().Value();
}

// unknown_properties
Napi::Value SpeciesClass::getUnknown_Properties(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto map = spec->unknown_properties;
    return getUnknown_Properties_Map(env, map);
}

void SpeciesClass::setUnknown_Properties(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    auto map = spec->unknown_properties;
    setUnknown_Properties_Map(env, value, map);
}

// ===== PhaseSpeciesClass Registration =====
Napi::Object PhaseSpeciesClass::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "PhaseSpecies", {
        InstanceAccessor("name", &PhaseSpeciesClass::getName, &PhaseSpeciesClass::setName),
        InstanceAccessor("diffusion_coefficient", &PhaseSpeciesClass::getDiffusion_Coefficient, &PhaseSpeciesClass::setDiffusion_Coefficient),
        InstanceAccessor("other_properties", &PhaseSpeciesClass::getUnknown_Properties, &PhaseSpeciesClass::setUnknown_Properties)
    });
    PhaseSpeciesConstructor = Napi::Persistent(func);

    exports.Set("PhaseSpecies", func);
    return exports;
}

// ===== PhaseSpeciesClass Constructor/Destructor =====
PhaseSpeciesClass::PhaseSpeciesClass(const Napi::CallbackInfo& info) : ObjectWrap(info) {
    phase_spec = std::make_unique<PhaseSpecies>();
}
PhaseSpeciesClass::~PhaseSpeciesClass() = default;

// ===== Accessors =====
// name
Napi::Value PhaseSpeciesClass::getName(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, phase_spec->name);
}

void PhaseSpeciesClass::setName(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (!value.IsString()) {
        Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
        return;
    }
    phase_spec->name = value.As<Napi::String>();
}

// diffusion_coefficient
Napi::Value PhaseSpeciesClass::getDiffusion_Coefficient(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (phase_spec->diffusion_coefficient.has_value()) {
        return Napi::Number::New(env, phase_spec->diffusion_coefficient.value());
    }
    return env.Undefined();
}

void PhaseSpeciesClass::setDiffusion_Coefficient(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (value.IsNull() || value.IsUndefined()) {
        phase_spec->diffusion_coefficient.reset();
        return;
    }
    if (!value.IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return;
    }
    phase_spec->diffusion_coefficient = value.As<Napi::Number>().DoubleValue();
}

// unknown_properties
Napi::Value PhaseSpeciesClass::getUnknown_Properties(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto map = phase_spec->unknown_properties;
    return getUnknown_Properties_Map(env, map);
}

void PhaseSpeciesClass::setUnknown_Properties(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    auto map = phase_spec->unknown_properties;
    setUnknown_Properties_Map(env, value, map);
}

// ===== PhaseClass Registration =====
Napi::Object PhaseClass::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "Phase", {
        InstanceAccessor("name", &PhaseClass::getName, &PhaseClass::setName),
        InstanceAccessor("species", &PhaseClass::getSpecies, &PhaseClass::setSpecies),
        InstanceAccessor("other_properties", &PhaseClass::getUnknown_Properties, &PhaseClass::setUnknown_Properties)
    });

    exports.Set("Phase", func);
    return exports;
}

// ===== PhaseClass Constructor/Destructor =====
PhaseClass::PhaseClass(const Napi::CallbackInfo& info) : ObjectWrap(info) {
    phase = std::make_unique<Phase>();
}
PhaseClass::~PhaseClass() = default;

// ===== Accessors =====
// name
Napi::Value PhaseClass::getName(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, phase->name);
}

void PhaseClass::setName(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (!value.IsString()) {
        Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
        return;
    }
    phase->name = value.As<Napi::String>();
}

// species
Napi::Value PhaseClass::getSpecies(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Array array = Napi::Array::New(env, phase->species.size());

    for (size_t i = 0; i < phase->species.size(); ++i) {
        auto instance = PhaseSpeciesClass::PhaseSpeciesConstructor.New({});
        auto phaseSpeciesObj = PhaseSpeciesClass::Unwrap(instance);
        *(phaseSpeciesObj->phase_spec) = phase->species[i];
        array.Set(i, instance);
    }
    return array;
}

void PhaseClass::setSpecies(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (!value.IsArray()) {
        Napi::TypeError::New(env, "Array expected").ThrowAsJavaScriptException();
        return;
    }

    Napi::Array array = value.As<Napi::Array>();
    phase->species.clear();

    for (uint32_t i = 0; i < array.Length(); ++i) {
        Napi::Value element = array[i];
        if (!element.IsObject()) {
            Napi::TypeError::New(env, "Array must contain PhaseSpecies objects").ThrowAsJavaScriptException();
            return;
        }

        PhaseSpeciesClass* phaseSpeciesObj = PhaseSpeciesClass::Unwrap(element.As<Napi::Object>());
        if (!phaseSpeciesObj) {
            Napi::TypeError::New(env, "Invalid PhaseSpecies object").ThrowAsJavaScriptException();
            return;
        }
        phase->species.push_back(*(phaseSpeciesObj->phase_spec));
    }
}

// unknown_properties
Napi::Value PhaseClass::getUnknown_Properties(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto map = phase->unknown_properties;
    return getUnknown_Properties_Map(env, map);
}  

void PhaseClass::setUnknown_Properties(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    auto map = phase->unknown_properties;
    setUnknown_Properties_Map(env, value, map);
}

// ===== ReactionComponentClass Registration =====
Napi::Object ReactionComponentClass::Init(Napi::Env env, Napi::Object exports) {
    return DefineClass(env, "ReactionComponent", {
        InstanceAccessor("species_name", &ReactionComponentClass::getSpecies_Name, &ReactionComponentClass::setSpecies_Name),
        InstanceAccessor("coefficient", &ReactionComponentClass::getCoefficient, &ReactionComponentClass::setCoefficient),
        InstanceAccessor("other_properties", &ReactionComponentClass::getUnknown_Properties, &ReactionComponentClass::setUnknown_Properties)
    });
}

// ===== ReactionComponentClass Constructor/Destructor =====
ReactionComponentClass::ReactionComponentClass(const Napi::CallbackInfo& info) : ObjectWrap(info) {
    reaction_comp = std::make_unique<ReactionComponent>();
}
ReactionComponentClass::~ReactionComponentClass() = default;

// ===== Accessors =====
// species_name
Napi::Value ReactionComponentClass::getSpecies_Name(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, reaction_comp->species_name);
}

void ReactionComponentClass::setSpecies_Name(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (!value.IsString()) {
        Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
        return;
    }
    reaction_comp->species_name = value.As<Napi::String>();
}

// coefficient
Napi::Value ReactionComponentClass::getCoefficient(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Number::New(env, reaction_comp->coefficient);
}

void ReactionComponentClass::setCoefficient(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (!value.IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return;
    }
    reaction_comp->coefficient = value.As<Napi::Number>().DoubleValue();
}

// unknown_properties
Napi::Value ReactionComponentClass::getUnknown_Properties(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto map = reaction_comp->unknown_properties;
    return getUnknown_Properties_Map(env, map);
}

void ReactionComponentClass::setUnknown_Properties(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    auto map = reaction_comp->unknown_properties;
    setUnknown_Properties_Map(env, value, map);
}
