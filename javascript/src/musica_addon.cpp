#include <napi.h>
#include <memory>
#include "musica_wrapper.h"

// Include mechanism configuration classes first (needed by micm.cpp)
#include "mechanism/species.cpp"
#include "mechanism/reaction_component.cpp"
#include "mechanism/phase.cpp"
#include "mechanism/arrhenius.cpp"
#include "mechanism/photolysis.cpp"
#include "mechanism/emission.cpp"
#include "mechanism/user_defined.cpp"
#include "mechanism/mechanism.cpp"

// Include the State and MICM class definitions
#include "state.cpp"
#include "micm.cpp"

using namespace musica_addon;
using namespace musica;

// Global wrapper instance for version functions
static std::unique_ptr<MusicaWrapper> g_wrapper;

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

// Module Initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    // Legacy version functions
    exports.Set("getVersion", Napi::Function::New(env, GetVersion));
    exports.Set("getSystemInfo", Napi::Function::New(env, GetSystemInfo));

    // Initialize State class
    StateClass::Init(env, exports);

    // Initialize MICM class
    MICMClass::Init(env, exports);

    // Get State constructor and store it for MICM to use
    Napi::Value state_value = exports.Get("State");
    if (state_value.IsFunction()) {
        Napi::Function state_func = state_value.As<Napi::Function>();
        MICMClass::state_constructor = new Napi::FunctionReference();
        *MICMClass::state_constructor = Napi::Persistent(state_func);
    }

    // Export mechanism configuration classes
    exports.Set("Species", musica::Species::GetClass(env));
    exports.Set("ReactionComponent", musica::ReactionComponent::GetClass(env));
    exports.Set("Phase", musica::Phase::GetClass(env));
    exports.Set("Arrhenius", musica::Arrhenius::GetClass(env));
    exports.Set("Photolysis", musica::Photolysis::GetClass(env));
    exports.Set("Emission", musica::Emission::GetClass(env));
    exports.Set("UserDefined", musica::UserDefined::GetClass(env));
    exports.Set("Mechanism", musica::Mechanism::GetClass(env));

    return exports;
}

NODE_API_MODULE(musica_addon, Init)
