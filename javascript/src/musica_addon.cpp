#include <napi.h>
#include <memory>

#include "micm.h"
#include "micm_wrapper.h"
#include "state.h"
#include "state_wrapper.h"

#include <musica/version.hpp>

using namespace musica_addon;

Napi::String GetVersion(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();
  try
  {
    const char* version = musica::GetMusicaVersion();
    return Napi::String::New(env, version);
  }
  catch (const std::exception& e)
  {
    return Napi::String::New(env, "Error: " + std::string(e.what()));
  }
}

// Module Initialization
Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  // Legacy functions
  exports.Set("getVersion", Napi::Function::New(env, GetVersion));

  // Register classes
  StateClass::Init(env, exports);
  MICMClass::Init(env, exports);

  return exports;
}

NODE_API_MODULE(musica_addon, Init)