#pragma omce

#include "common.h"

#pragma once
#include <napi.h>
#include <memory>


class SpeciesClass : public Napi::ObjectWrap<SpeciesClass> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    SpeciesClass(const Napi::CallbackInfo& info);
    ~SpeciesClass();

private:
    std::unique_ptr<Species> spec;

    // Getters/Setters for InstanceAccessor
    Napi::Value getName(const Napi::CallbackInfo& info);
    void setName(const Napi::CallbackInfo& info, const Napi::Value& value);

    Napi::Value getMolecular_Weight(const Napi::CallbackInfo& info);
    void setMolecular_Weight(const Napi::CallbackInfo& info, const Napi::Value& value);

    Napi::Value getConstant_Concentration(const Napi::CallbackInfo& info);
    void setConstant_Concentration(const Napi::CallbackInfo& info, const Napi::Value& value);

    Napi::Value getConstant_Mixing_Ratio(const Napi::CallbackInfo& info);
    void setConstant_Mixing_Ratio(const Napi::CallbackInfo& info, const Napi::Value& value);

    Napi::Value getIs_Third_Party(const Napi::CallbackInfo& info);
    void setIs_Third_Party(const Napi::CallbackInfo& info, const Napi::Value& value);

    Napi::Value getUnknown_Properties(const Napi::CallbackInfo& info);
    void setUnknown_Properties(const Napi::CallbackInfo& info, const Napi::Value& value);
};

class PhaseSpeciesClass : public Napi::ObjectWrap<PhaseSpeciesClass> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    PhaseSpeciesClass(const Napi::CallbackInfo& info);
    ~PhaseSpeciesClass();

    // Make PhaseClass a friend so it can access private members
    friend class PhaseClass;

private:
    static Napi::FunctionReference PhaseSpeciesConstructor;
    std::unique_ptr<PhaseSpecies> phase_spec;

    // Getters/Setters for InstanceAccessor
    Napi::Value getName(const Napi::CallbackInfo& info);
    void setName(const Napi::CallbackInfo& info, const Napi::Value& value);

    Napi::Value getDiffusion_Coefficient(const Napi::CallbackInfo& info);
    void setDiffusion_Coefficient(const Napi::CallbackInfo& info, const Napi::Value& value);

    Napi::Value getUnknown_Properties(const Napi::CallbackInfo& info);
    void setUnknown_Properties(const Napi::CallbackInfo& info, const Napi::Value& value);
};

class PhaseClass : public Napi::ObjectWrap<PhaseClass> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    PhaseClass(const Napi::CallbackInfo& info);
    ~PhaseClass();
private:
    std::unique_ptr<Phase> phase;

    // Getters/Setters for InstanceAccessor
    Napi::Value getName(const Napi::CallbackInfo& info);
    void setName(const Napi::CallbackInfo& info, const Napi::Value& value);

    Napi::Value getSpecies(const Napi::CallbackInfo& info);
    void setSpecies(const Napi::CallbackInfo& info, const Napi::Value& value);

    Napi::Value getUnknown_Properties(const Napi::CallbackInfo& info);
    void setUnknown_Properties(const Napi::CallbackInfo& info, const Napi::Value& value);
};

class ReactionComponentClass : public Napi::ObjectWrap<ReactionComponentClass> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    ReactionComponentClass(const Napi::CallbackInfo& info);
    ~ReactionComponentClass();
private:
    std::unique_ptr<ReactionComponent> reaction_comp;

    // Getters/Setters for InstanceAccessor
    Napi::Value getSpecies_Name(const Napi::CallbackInfo& info);
    void setSpecies_Name(const Napi::CallbackInfo& info, const Napi::Value& value);

    Napi::Value getCoefficient(const Napi::CallbackInfo& info);
    void setCoefficient(const Napi::CallbackInfo& info, const Napi::Value& value);

    Napi::Value getUnknown_Properties(const Napi::CallbackInfo& info);
    void setUnknown_Properties(const Napi::CallbackInfo& info, const Napi::Value& value);
};
