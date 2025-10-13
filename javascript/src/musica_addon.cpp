#include <napi.h>
#include "musica_wrapper.h"
#include <memory>
#include <musica/version.hpp>


using namespace musica_addon;

// Global wrapper instance for static functions
static std::unique_ptr<MusicaWrapper> g_wrapper;

// Simple functions that match the stub interface
Napi::String GetVersion(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    try {
        if (!g_wrapper) {
            g_wrapper = std::make_unique<MusicaWrapper>();
        }
        return Napi::String::New(env, g_wrapper->GetVersion());
    } catch (const std::exception& e) {
        return Napi::String::New(env, "Error: " + std::string(e.what()));
    }
}

Napi::Object GetSystemInfo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Object obj = Napi::Object::New(env);

    obj.Set("platform", Napi::String::New(env, "darwin"));
    obj.Set("arch", Napi::String::New(env, "arm64"));
    obj.Set("nodeVersion", Napi::String::New(env, "20.17.0"));

    try {
        if (!g_wrapper) {
            g_wrapper = std::make_unique<MusicaWrapper>();
        }
        obj.Set("musicaVersion", Napi::String::New(env, g_wrapper->GetVersion()));
    } catch (const std::exception& e) {
        obj.Set("musicaVersion", Napi::String::New(env, std::string("Error: ") + e.what()));
    }

    return obj;
}

// Initialize the addon
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    // Basic functions
    exports.Set(Napi::String::New(env, "getVersion"),
                Napi::Function::New(env, GetVersion));

    exports.Set(Napi::String::New(env, "getSystemInfo"),
                Napi::Function::New(env, GetSystemInfo));

    return exports;
}

NODE_API_MODULE(musica_addon, Init)