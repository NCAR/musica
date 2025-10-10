#include <napi.h>
#include "musica_wrapper.h"
#include "state_wrapper.h"
#include <memory>

#include <musica/micm/micm.hpp>
#include <musica/util.hpp>

using namespace musica_addon;

// Global constructor references
static Napi::FunctionReference* g_StateConstructor = nullptr;
static Napi::FunctionReference* g_MICMConstructor = nullptr;

// State Class - N-API Wrapper
class StateClass : public Napi::ObjectWrap<StateClass> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "State", {
            InstanceMethod("setConcentrations", &StateClass::SetConcentrations),
            InstanceMethod("getConcentrations", &StateClass::GetConcentrations),
            InstanceMethod("setUserDefinedRateParameters", &StateClass::SetUserDefinedRateParameters),
            InstanceMethod("getUserDefinedRateParameters", &StateClass::GetUserDefinedRateParameters),
            InstanceMethod("setConditions", &StateClass::SetConditions),
            InstanceMethod("getConditions", &StateClass::GetConditions),
            InstanceMethod("getSpeciesOrdering", &StateClass::GetSpeciesOrdering),
            InstanceMethod("getUserDefinedRateParametersOrdering", &StateClass::GetUserDefinedRateParametersOrdering),
            InstanceMethod("getNumberOfGridCells", &StateClass::GetNumberOfGridCells),
            InstanceMethod("concentrationStrides", &StateClass::ConcentrationStrides),
            InstanceMethod("userDefinedRateParameterStrides", &StateClass::UserDefinedRateParameterStrides),
        });

        g_StateConstructor = new Napi::FunctionReference();
        *g_StateConstructor = Napi::Persistent(func);

        exports.Set("State", func);
        return exports;
    }

    StateClass(const Napi::CallbackInfo& info) : Napi::ObjectWrap<StateClass>(info) {
        Napi::Env env = info.Env();

        if (info.Length() < 1 || !info[0].IsExternal()) {
            Napi::TypeError::New(env, "Internal construction only").ThrowAsJavaScriptException();
            return;
        }

        state_ = info[0].As<Napi::External<StateWrapper>>().Data();
    }

    StateWrapper* GetStateWrapper() const { return state_; }

private:
    StateWrapper* state_;

    Napi::Value SetConcentrations(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 1 || !info[0].IsObject()) {
            Napi::TypeError::New(env, "Expected object with concentrations").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        Napi::Object conc_obj = info[0].As<Napi::Object>();
        std::map<std::string, std::vector<double>> concentrations;

        Napi::Array keys = conc_obj.GetPropertyNames();
        for (uint32_t i = 0; i < keys.Length(); ++i) {
            std::string key = keys.Get(i).As<Napi::String>().Utf8Value();
            Napi::Value value = conc_obj.Get(key);

            std::vector<double> values;
            if (value.IsNumber()) {
                values.push_back(value.As<Napi::Number>().DoubleValue());
            } else if (value.IsArray()) {
                Napi::Array arr = value.As<Napi::Array>();
                for (uint32_t j = 0; j < arr.Length(); ++j) {
                    values.push_back(arr.Get(j).As<Napi::Number>().DoubleValue());
                }
            }
            concentrations[key] = values;
        }

        try {
            state_->SetConcentrations(concentrations);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        }

        return env.Undefined();
    }

    Napi::Value GetConcentrations(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        try {
            auto concentrations = state_->GetConcentrations();
            Napi::Object result = Napi::Object::New(env);

            for (const auto& pair : concentrations) {
                Napi::Array arr = Napi::Array::New(env, pair.second.size());
                for (size_t i = 0; i < pair.second.size(); ++i) {
                    arr.Set(i, Napi::Number::New(env, pair.second[i]));
                }
                result.Set(pair.first, arr);
            }

            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value SetUserDefinedRateParameters(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 1 || !info[0].IsObject()) {
            Napi::TypeError::New(env, "Expected object with rate parameters").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        Napi::Object params_obj = info[0].As<Napi::Object>();
        std::map<std::string, std::vector<double>> params;

        Napi::Array keys = params_obj.GetPropertyNames();
        for (uint32_t i = 0; i < keys.Length(); ++i) {
            std::string key = keys.Get(i).As<Napi::String>().Utf8Value();
            Napi::Value value = params_obj.Get(key);

            std::vector<double> values;
            if (value.IsNumber()) {
                values.push_back(value.As<Napi::Number>().DoubleValue());
            } else if (value.IsArray()) {
                Napi::Array arr = value.As<Napi::Array>();
                for (uint32_t j = 0; j < arr.Length(); ++j) {
                    values.push_back(arr.Get(j).As<Napi::Number>().DoubleValue());
                }
            }
            params[key] = values;
        }

        try {
            state_->SetUserDefinedRateParameters(params);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        }

        return env.Undefined();
    }

    Napi::Value GetUserDefinedRateParameters(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        try {
            auto params = state_->GetUserDefinedRateParameters();
            Napi::Object result = Napi::Object::New(env);

            for (const auto& pair : params) {
                Napi::Array arr = Napi::Array::New(env, pair.second.size());
                for (size_t i = 0; i < pair.second.size(); ++i) {
                    arr.Set(i, Napi::Number::New(env, pair.second[i]));
                }
                result.Set(pair.first, arr);
            }

            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value SetConditions(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 1 || !info[0].IsObject()) {
            Napi::TypeError::New(env, "Expected object with conditions").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        Napi::Object cond_obj = info[0].As<Napi::Object>();

        std::vector<double>* temperatures = nullptr;
        std::vector<double>* pressures = nullptr;
        std::vector<double>* air_densities = nullptr;

        std::vector<double> temp_temps, temp_press, temp_air;

        if (cond_obj.Has("temperatures")) {
            Napi::Value val = cond_obj.Get("temperatures");
            if (val.IsNumber()) {
                temp_temps.push_back(val.As<Napi::Number>().DoubleValue());
            } else if (val.IsArray()) {
                Napi::Array arr = val.As<Napi::Array>();
                for (uint32_t i = 0; i < arr.Length(); ++i) {
                    temp_temps.push_back(arr.Get(i).As<Napi::Number>().DoubleValue());
                }
            }
            if (!temp_temps.empty()) temperatures = &temp_temps;
        }

        if (cond_obj.Has("pressures")) {
            Napi::Value val = cond_obj.Get("pressures");
            if (val.IsNumber()) {
                temp_press.push_back(val.As<Napi::Number>().DoubleValue());
            } else if (val.IsArray()) {
                Napi::Array arr = val.As<Napi::Array>();
                for (uint32_t i = 0; i < arr.Length(); ++i) {
                    temp_press.push_back(arr.Get(i).As<Napi::Number>().DoubleValue());
                }
            }
            if (!temp_press.empty()) pressures = &temp_press;
        }

        if (cond_obj.Has("air_densities")) {
            Napi::Value val = cond_obj.Get("air_densities");
            if (val.IsNumber()) {
                temp_air.push_back(val.As<Napi::Number>().DoubleValue());
            } else if (val.IsArray()) {
                Napi::Array arr = val.As<Napi::Array>();
                for (uint32_t i = 0; i < arr.Length(); ++i) {
                    temp_air.push_back(arr.Get(i).As<Napi::Number>().DoubleValue());
                }
            }
            if (!temp_air.empty()) air_densities = &temp_air;
        }

        try {
            state_->SetConditions(temperatures, pressures, air_densities);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        }

        return env.Undefined();
    }

    Napi::Value GetConditions(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        try {
            auto conditions = state_->GetConditions();
            Napi::Object result = Napi::Object::New(env);

            for (const auto& pair : conditions) {
                Napi::Array arr = Napi::Array::New(env, pair.second.size());
                for (size_t i = 0; i < pair.second.size(); ++i) {
                    arr.Set(i, Napi::Number::New(env, pair.second[i]));
                }
                result.Set(pair.first, arr);
            }

            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetSpeciesOrdering(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        try {
            auto ordering = state_->GetSpeciesOrdering();
            Napi::Object result = Napi::Object::New(env);

            for (const auto& pair : ordering) {
                result.Set(pair.first, Napi::Number::New(env, pair.second));
            }

            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetUserDefinedRateParametersOrdering(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        try {
            auto ordering = state_->GetUserDefinedRateParametersOrdering();
            Napi::Object result = Napi::Object::New(env);

            for (const auto& pair : ordering) {
                result.Set(pair.first, Napi::Number::New(env, pair.second));
            }

            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetNumberOfGridCells(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        return Napi::Number::New(env, state_->GetNumberOfGridCells());
    }

    Napi::Value ConcentrationStrides(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        size_t cell_stride, species_stride;
        state_->GetConcentrationStrides(cell_stride, species_stride);

        Napi::Array result = Napi::Array::New(env, 2);
        result.Set(uint32_t(0), Napi::Number::New(env, cell_stride));
        result.Set(uint32_t(1), Napi::Number::New(env, species_stride));

        return result;
    }

    Napi::Value UserDefinedRateParameterStrides(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        size_t cell_stride, param_stride;
        state_->GetUserDefinedRateParameterStrides(cell_stride, param_stride);

        Napi::Array result = Napi::Array::New(env, 2);
        result.Set(uint32_t(0), Napi::Number::New(env, cell_stride));
        result.Set(uint32_t(1), Napi::Number::New(env, param_stride));

        return result;
    }
};

// MICM Class - N-API Wrapper
class MICMClass : public Napi::ObjectWrap<MICMClass> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "MICM", {
            InstanceMethod("createState", &MICMClass::CreateState),
            InstanceMethod("solve", &MICMClass::Solve),
        });

        Napi::FunctionReference* constructor = new Napi::FunctionReference();
        *constructor = Napi::Persistent(func);
        env.SetInstanceData(constructor);

        exports.Set("MICM", func);
        return exports;
    }

    MICMClass(const Napi::CallbackInfo& info) : Napi::ObjectWrap<MICMClass>(info) {
        Napi::Env env = info.Env();

        if (info.Length() < 2) {
            Napi::TypeError::New(env, "Expected 2 arguments: config_path and solver_type")
                .ThrowAsJavaScriptException();
            return;
        }

        if (!info[0].IsString()) {
            Napi::TypeError::New(env, "config_path must be a string").ThrowAsJavaScriptException();
            return;
        }

        if (!info[1].IsNumber()) {
            Napi::TypeError::New(env, "solver_type must be a number").ThrowAsJavaScriptException();
            return;
        }

        std::string config_path = info[0].As<Napi::String>().Utf8Value();
        int solver_type = info[1].As<Napi::Number>().Int32Value();

        try {
            micm_ = std::make_unique<musica::MICM>(config_path.c_str(), static_cast<musica::MICMSolver>(solver_type));
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        }
    }

private:
    std::unique_ptr<musica::MICM> micm_;
    std::vector<std::shared_ptr<StateWrapper>> states_;

    Napi::Value CreateState(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected 1 argument: number_of_grid_cells")
                .ThrowAsJavaScriptException();
            return env.Null();
        }

        if (!info[0].IsNumber()) {
            Napi::TypeError::New(env, "number_of_grid_cells must be a number")
                .ThrowAsJavaScriptException();
            return env.Null();
        }

        size_t num_cells = info[0].As<Napi::Number>().Int64Value();

        try {
            musica::State* raw_state = micm_->CreateState(num_cells);
            auto state_wrapper = std::make_shared<StateWrapper>(raw_state);
            states_.push_back(state_wrapper);

            // Return StateClass instance using global constructor
            if (g_StateConstructor == nullptr) {
                Napi::Error::New(env, "State constructor not initialized").ThrowAsJavaScriptException();
                return env.Null();
            }

            Napi::Object state_obj = g_StateConstructor->New({Napi::External<StateWrapper>::New(env, state_wrapper.get())});
            return state_obj;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value Solve(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 2) {
            Napi::TypeError::New(env, "Expected 2 arguments: state and time_step")
                .ThrowAsJavaScriptException();
            return env.Undefined();
        }

        if (!info[0].IsObject() || !info[1].IsNumber()) {
            Napi::TypeError::New(env, "Invalid arguments").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        // Extract StateWrapper from the object
        Napi::Object state_obj = info[0].As<Napi::Object>();
        StateClass* state_class = StateClass::Unwrap(state_obj);
        double time_step = info[1].As<Napi::Number>().DoubleValue();

        try {
            musica::Error error;
            musica::String solver_state;
            musica::SolverResultStats stats;
            micm_->Solve(state_class->GetStateWrapper()->GetState(), time_step, &solver_state, &stats);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        }

        return env.Undefined();
    }

    friend class StateClass;
};


// Global wrapper instance for static functions
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
    // Legacy functions
    exports.Set("getVersion", Napi::Function::New(env, GetVersion));
    exports.Set("getSystemInfo", Napi::Function::New(env, GetSystemInfo));

    // Register classes
    StateClass::Init(env, exports);
    MICMClass::Init(env, exports);

    return exports;
}

NODE_API_MODULE(musica_addon, Init)