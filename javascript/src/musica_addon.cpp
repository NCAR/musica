#include <napi.h>
#include <memory>

#include "micm/micm.h"
#include "micm/state.h"

#include "mechanism_configuration/types.h"

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

  // MICM Solver classes
  StateClass::Init(env, exports);
  MICMClass::Init(env, exports);

  // Mechanism Configuration classes
  SpeciesClass::Init(env, exports);
  PhaseSpeciesClass::Init(env, exports);
  PhaseClass::Init(env, exports);
  ReactionComponentClass::Init(env, exports);

  return exports;
}

NODE_API_MODULE(musica_addon, Init)