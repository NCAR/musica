#pragma once

#include <napi.h>
#include <memory>

#include <musica/micm/parse.hpp>

#include <mechanism_configuration/constants.hpp>
#include <mechanism_configuration/v1/parser.hpp>
#include <mechanism_configuration/v1/reaction_types.hpp>
#include <mechanism_configuration/v1/types.hpp>
#include <mechanism_configuration/v1/validation.hpp>

Napi::Value getUnknown_Properties_Map(Napi::Env env, const std::unordered_map<std::string, std::string>& unknown_properties) {
    Napi::Object obj = Napi::Object::New(env);
    for (const auto& [key, value] : unknown_properties) {
        obj.Set(key, Napi::String::New(env, value));
    }
    return obj;
}

void setUnknown_Properties_Map(Napi::Env env, const Napi::Value& value, std::unordered_map<std::string, std::string>& unknown_properties) {
    if (!value.IsObject()) {
        Napi::TypeError::New(env, "Object expected").ThrowAsJavaScriptException();
        return;
    }
    Napi::Object obj = value.As<Napi::Object>();
    Napi::Array keys = obj.GetPropertyNames();
    uint32_t length = keys.Length();
    for (uint32_t i = 0; i < length; i++) {
        Napi::Value key = keys.Get(i);
        Napi::Value val = obj.Get(key);
        if (!key.IsString() || !val.IsString()) {
            Napi::TypeError::New(env, "String keys and values expected").ThrowAsJavaScriptException();
            return;
        }
        unknown_properties[key.As<Napi::String>()] = val.As<Napi::String>();
    }
}