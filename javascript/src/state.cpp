#include <napi.h>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <musica/micm/micm.hpp>
#include <musica/micm/state_c_interface.hpp>
#include <musica/util.hpp>
#include <micm/system/conditions.hpp>

namespace musica_addon {

/**
 * State class for MICM solver - N-API wrapper
 */
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
            InstanceMethod("getInternalStates", &StateClass::GetInternalStates),
        });

        auto constructor = new Napi::FunctionReference();
        *constructor = Napi::Persistent(func);
        env.SetInstanceData(constructor);

        exports.Set("State", func);
        return exports;
    }

    /**
     * Constructor - creates a single internal state with all grid cells
     * @param info[0] - musica::MICM* solver
     * @param info[1] - number_of_grid_cells
     * @param info[2] - vector_size (optional, defaults to 1 for standard-order)
     */
    StateClass(const Napi::CallbackInfo& info) : Napi::ObjectWrap<StateClass>(info) {
        Napi::Env env = info.Env();

        if (info.Length() < 2) {
            Napi::TypeError::New(env, "Expected at least 2 arguments: solver and number_of_grid_cells")
                .ThrowAsJavaScriptException();
            return;
        }

        if (!info[0].IsExternal() || !info[1].IsNumber()) {
            Napi::TypeError::New(env, "Invalid arguments").ThrowAsJavaScriptException();
            return;
        }

        auto* solver = info[0].As<Napi::External<musica::MICM>>().Data();
        size_t number_of_grid_cells = info[1].As<Napi::Number>().Int64Value();
        size_t vector_size = (info.Length() > 2 && info[2].IsNumber())
            ? info[2].As<Napi::Number>().Int64Value()
            : 1;

        number_of_grid_cells_ = number_of_grid_cells;
        vector_size_ = vector_size;

        // Create single internal state with all grid cells
        state_ = solver->CreateState(number_of_grid_cells);

        // Cache species and parameter orderings
        if (state_) {
            CacheOrderings();
        }
    }

private:
    std::unique_ptr<musica::State> state_;
    size_t number_of_grid_cells_;
    size_t vector_size_;
    std::map<std::string, size_t> species_ordering_;
    std::map<std::string, size_t> user_defined_rate_parameters_ordering_;

    void CacheOrderings() {
        musica::Error error;

        // Get species ordering
        musica::Mappings species_mappings;
        musica::GetSpeciesOrdering(state_.get(), &species_mappings, &error);
        if (musica::IsSuccess(error)) {
            for (size_t i = 0; i < species_mappings.size_; ++i) {
                species_ordering_[species_mappings.mappings_[i].name_.value_] = species_mappings.mappings_[i].index_;
            }
        }
        musica::DeleteMappings(&species_mappings);
        musica::DeleteError(&error);

        // Get user-defined rate parameters ordering
        musica::Mappings params_mappings;
        musica::GetUserDefinedRateParametersOrdering(state_.get(), &params_mappings, &error);
        if (musica::IsSuccess(error)) {
            for (size_t i = 0; i < params_mappings.size_; ++i) {
                user_defined_rate_parameters_ordering_[params_mappings.mappings_[i].name_.value_] =
                    params_mappings.mappings_[i].index_;
            }
        }
        musica::DeleteMappings(&params_mappings);
        musica::DeleteError(&error);
    }

    Napi::Value SetConcentrations(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 1 || !info[0].IsObject()) {
            Napi::TypeError::New(env, "Expected object with concentrations").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        Napi::Object conc_obj = info[0].As<Napi::Object>();
        Napi::Array keys = conc_obj.GetPropertyNames();

        // Process each species
        for (uint32_t i = 0; i < keys.Length(); ++i) {
            std::string species_name = keys.Get(i).As<Napi::String>().Utf8Value();

            if (species_ordering_.find(species_name) == species_ordering_.end()) {
                Napi::TypeError::New(env, ("Species " + species_name + " not found in mechanism").c_str())
                    .ThrowAsJavaScriptException();
                return env.Undefined();
            }

            size_t species_idx = species_ordering_[species_name];
            Napi::Value value = conc_obj.Get(species_name);

            std::vector<double> values;
            if (value.IsNumber()) {
                values.push_back(value.As<Napi::Number>().DoubleValue());
            } else if (value.IsArray()) {
                Napi::Array arr = value.As<Napi::Array>();
                for (uint32_t j = 0; j < arr.Length(); ++j) {
                    values.push_back(arr.Get(j).As<Napi::Number>().DoubleValue());
                }
            }

            if (values.size() != number_of_grid_cells_) {
                Napi::TypeError::New(env, ("Concentration list for " + species_name +
                    " must have length " + std::to_string(number_of_grid_cells_)).c_str())
                    .ThrowAsJavaScriptException();
                return env.Undefined();
            }

            // Set concentrations using vector-ordered indexing when vector_size > 1
            musica::Error error;
            size_t array_size;
            double* conc_ptr = musica::GetOrderedConcentrationsPointer(state_.get(), &array_size, &error);

            if (vector_size_ > 1) {
                // Use special indexing for vector-ordered matrices:
                // idx = (group_index * n_species + i_species) * vector_size + row_in_group
                size_t n_species = species_ordering_.size();
                for (size_t i_cell = 0; i_cell < number_of_grid_cells_ && i_cell < values.size(); ++i_cell) {
                    size_t group_index = i_cell / vector_size_;
                    size_t row_in_group = i_cell % vector_size_;
                    size_t idx = (group_index * n_species + species_idx) * vector_size_ + row_in_group;
                    conc_ptr[idx] = values[i_cell];
                }
            } else {
                // Standard-order: use simple stride indexing
                size_t cell_stride, species_stride;
                musica::GetConcentrationsStrides(state_.get(), &error, &cell_stride, &species_stride);
                size_t num_cells = musica::GetNumberOfGridCells(state_.get(), &error);

                for (size_t i_cell = 0; i_cell < num_cells && i_cell < values.size(); ++i_cell) {
                    conc_ptr[species_idx * species_stride + i_cell * cell_stride] = values[i_cell];
                }
            }

            musica::DeleteError(&error);
        }

        return env.Undefined();
    }

    Napi::Value GetConcentrations(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        Napi::Object result = Napi::Object::New(env);

        musica::Error error;
        size_t array_size;
        double* conc_ptr = musica::GetOrderedConcentrationsPointer(state_.get(), &array_size, &error);

        if (vector_size_ > 1) {
            // Use special indexing for vector-ordered matrices
            size_t n_species = species_ordering_.size();
            for (const auto& [species_name, species_idx] : species_ordering_) {
                Napi::Array species_conc = Napi::Array::New(env);

                for (size_t i_cell = 0; i_cell < number_of_grid_cells_; ++i_cell) {
                    size_t group_index = i_cell / vector_size_;
                    size_t row_in_group = i_cell % vector_size_;
                    size_t idx = (group_index * n_species + species_idx) * vector_size_ + row_in_group;
                    species_conc.Set(i_cell, Napi::Number::New(env, conc_ptr[idx]));
                }

                result.Set(species_name, species_conc);
            }
        } else {
            // Standard-order: use simple stride indexing
            size_t cell_stride, species_stride;
            musica::GetConcentrationsStrides(state_.get(), &error, &cell_stride, &species_stride);
            size_t num_cells = musica::GetNumberOfGridCells(state_.get(), &error);

            for (const auto& [species_name, species_idx] : species_ordering_) {
                Napi::Array species_conc = Napi::Array::New(env);

                for (size_t i_cell = 0; i_cell < num_cells; ++i_cell) {
                    double value = conc_ptr[species_idx * species_stride + i_cell * cell_stride];
                    species_conc.Set(i_cell, Napi::Number::New(env, value));
                }

                result.Set(species_name, species_conc);
            }
        }

        musica::DeleteError(&error);
        return result;
    }

    Napi::Value SetUserDefinedRateParameters(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 1 || !info[0].IsObject()) {
            Napi::TypeError::New(env, "Expected object with rate parameters").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        Napi::Object params_obj = info[0].As<Napi::Object>();
        Napi::Array keys = params_obj.GetPropertyNames();

        for (uint32_t i = 0; i < keys.Length(); ++i) {
            std::string param_name = keys.Get(i).As<Napi::String>().Utf8Value();

            if (user_defined_rate_parameters_ordering_.find(param_name) == user_defined_rate_parameters_ordering_.end()) {
                Napi::TypeError::New(env, ("Parameter " + param_name + " not found in mechanism").c_str())
                    .ThrowAsJavaScriptException();
                return env.Undefined();
            }

            size_t param_idx = user_defined_rate_parameters_ordering_[param_name];
            Napi::Value value = params_obj.Get(param_name);

            std::vector<double> values;
            if (value.IsNumber()) {
                values.push_back(value.As<Napi::Number>().DoubleValue());
            } else if (value.IsArray()) {
                Napi::Array arr = value.As<Napi::Array>();
                for (uint32_t j = 0; j < arr.Length(); ++j) {
                    values.push_back(arr.Get(j).As<Napi::Number>().DoubleValue());
                }
            }

            // Set parameters using vector-ordered indexing when vector_size > 1
            musica::Error error;
            size_t array_size;
            double* params_ptr = musica::GetOrderedRateParametersPointer(state_.get(), &array_size, &error);

            if (vector_size_ > 1) {
                // Use special indexing for vector-ordered matrices
                size_t n_params = user_defined_rate_parameters_ordering_.size();
                for (size_t i_cell = 0; i_cell < number_of_grid_cells_ && i_cell < values.size(); ++i_cell) {
                    size_t group_index = i_cell / vector_size_;
                    size_t row_in_group = i_cell % vector_size_;
                    size_t idx = (group_index * n_params + param_idx) * vector_size_ + row_in_group;
                    params_ptr[idx] = values[i_cell];
                }
            } else {
                // Standard-order: use simple stride indexing
                size_t cell_stride, param_stride;
                musica::GetUserDefinedRateParametersStrides(state_.get(), &error, &cell_stride, &param_stride);
                size_t num_cells = musica::GetNumberOfGridCells(state_.get(), &error);

                for (size_t i_cell = 0; i_cell < num_cells && i_cell < values.size(); ++i_cell) {
                    params_ptr[param_idx * param_stride + i_cell * cell_stride] = values[i_cell];
                }
            }

            musica::DeleteError(&error);
        }

        return env.Undefined();
    }

    Napi::Value GetUserDefinedRateParameters(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        Napi::Object result = Napi::Object::New(env);

        musica::Error error;
        size_t array_size;
        double* params_ptr = musica::GetOrderedRateParametersPointer(state_.get(), &array_size, &error);

        if (vector_size_ > 1) {
            // Use special indexing for vector-ordered matrices
            size_t n_params = user_defined_rate_parameters_ordering_.size();
            for (const auto& [param_name, param_idx] : user_defined_rate_parameters_ordering_) {
                Napi::Array param_values = Napi::Array::New(env);

                for (size_t i_cell = 0; i_cell < number_of_grid_cells_; ++i_cell) {
                    size_t group_index = i_cell / vector_size_;
                    size_t row_in_group = i_cell % vector_size_;
                    size_t idx = (group_index * n_params + param_idx) * vector_size_ + row_in_group;
                    param_values.Set(i_cell, Napi::Number::New(env, params_ptr[idx]));
                }

                result.Set(param_name, param_values);
            }
        } else {
            // Standard-order: use simple stride indexing
            size_t cell_stride, param_stride;
            musica::GetUserDefinedRateParametersStrides(state_.get(), &error, &cell_stride, &param_stride);
            size_t num_cells = musica::GetNumberOfGridCells(state_.get(), &error);

            for (const auto& [param_name, param_idx] : user_defined_rate_parameters_ordering_) {
                Napi::Array param_values = Napi::Array::New(env);

                for (size_t i_cell = 0; i_cell < num_cells; ++i_cell) {
                    double value = params_ptr[param_idx * param_stride + i_cell * cell_stride];
                    param_values.Set(i_cell, Napi::Number::New(env, value));
                }

                result.Set(param_name, param_values);
            }
        }

        musica::DeleteError(&error);
        return result;
    }

    Napi::Value SetConditions(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 1 || !info[0].IsObject()) {
            Napi::TypeError::New(env, "Expected object with conditions").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        Napi::Object cond_obj = info[0].As<Napi::Object>();

        std::vector<double> temperatures, pressures, air_densities;
        bool has_temp = false, has_press = false, has_air = false;

        // Extract temperatures
        if (cond_obj.Has("temperatures")) {
            Napi::Value val = cond_obj.Get("temperatures");
            if (val.IsNumber()) {
                temperatures.push_back(val.As<Napi::Number>().DoubleValue());
            } else if (val.IsArray()) {
                Napi::Array arr = val.As<Napi::Array>();
                for (uint32_t i = 0; i < arr.Length(); ++i) {
                    temperatures.push_back(arr.Get(i).As<Napi::Number>().DoubleValue());
                }
            }
            has_temp = !temperatures.empty();
        }

        // Extract pressures
        if (cond_obj.Has("pressures")) {
            Napi::Value val = cond_obj.Get("pressures");
            if (val.IsNumber()) {
                pressures.push_back(val.As<Napi::Number>().DoubleValue());
            } else if (val.IsArray()) {
                Napi::Array arr = val.As<Napi::Array>();
                for (uint32_t i = 0; i < arr.Length(); ++i) {
                    pressures.push_back(arr.Get(i).As<Napi::Number>().DoubleValue());
                }
            }
            has_press = !pressures.empty();
        }

        // Extract air densities
        if (cond_obj.Has("air_densities")) {
            Napi::Value val = cond_obj.Get("air_densities");
            if (val.IsNumber()) {
                air_densities.push_back(val.As<Napi::Number>().DoubleValue());
            } else if (val.IsArray()) {
                Napi::Array arr = val.As<Napi::Array>();
                for (uint32_t i = 0; i < arr.Length(); ++i) {
                    air_densities.push_back(arr.Get(i).As<Napi::Number>().DoubleValue());
                }
            }
            has_air = !air_densities.empty();
        }

        // Set conditions directly on single state
        musica::Error error;
        size_t array_size;
        micm::Conditions* conditions = musica::GetConditionsPointer(state_.get(), &array_size, &error);
        size_t num_cells = musica::GetNumberOfGridCells(state_.get(), &error);

        for (size_t i_cell = 0; i_cell < num_cells; ++i_cell) {
            if (has_temp && i_cell < temperatures.size()) {
                conditions[i_cell].temperature_ = temperatures[i_cell];
            }
            if (has_press && i_cell < pressures.size()) {
                conditions[i_cell].pressure_ = pressures[i_cell];
            }
            if (has_air && i_cell < air_densities.size()) {
                conditions[i_cell].air_density_ = air_densities[i_cell];
            } else if (has_temp && has_press && i_cell < temperatures.size() && i_cell < pressures.size()) {
                // Calculate air density from ideal gas law
                constexpr double GAS_CONSTANT = 8.31446261815324; // J K^-1 mol^-1
                conditions[i_cell].air_density_ = pressures[i_cell] / (GAS_CONSTANT * temperatures[i_cell]);
            }
        }

        musica::DeleteError(&error);

        return env.Undefined();
    }

    Napi::Value GetConditions(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        Napi::Object result = Napi::Object::New(env);

        Napi::Array temps = Napi::Array::New(env);
        Napi::Array press = Napi::Array::New(env);
        Napi::Array air_dens = Napi::Array::New(env);

        musica::Error error;
        size_t array_size;
        micm::Conditions* conditions = musica::GetConditionsPointer(state_.get(), &array_size, &error);
        size_t num_cells = musica::GetNumberOfGridCells(state_.get(), &error);

        for (size_t i_cell = 0; i_cell < num_cells; ++i_cell) {
            temps.Set(i_cell, Napi::Number::New(env, conditions[i_cell].temperature_));
            press.Set(i_cell, Napi::Number::New(env, conditions[i_cell].pressure_));
            air_dens.Set(i_cell, Napi::Number::New(env, conditions[i_cell].air_density_));
        }

        musica::DeleteError(&error);

        result.Set("temperature", temps);
        result.Set("pressure", press);
        result.Set("air_density", air_dens);

        return result;
    }

    Napi::Value GetSpeciesOrdering(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        Napi::Object result = Napi::Object::New(env);

        for (const auto& [name, idx] : species_ordering_) {
            result.Set(name, Napi::Number::New(env, idx));
        }

        return result;
    }

    Napi::Value GetUserDefinedRateParametersOrdering(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        Napi::Object result = Napi::Object::New(env);

        for (const auto& [name, idx] : user_defined_rate_parameters_ordering_) {
            result.Set(name, Napi::Number::New(env, idx));
        }

        return result;
    }

    Napi::Value GetNumberOfGridCells(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        return Napi::Number::New(env, number_of_grid_cells_);
    }

    Napi::Value ConcentrationStrides(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (!state_) {
            return env.Null();
        }

        musica::Error error;
        size_t cell_stride, species_stride;
        musica::GetConcentrationsStrides(state_.get(), &error, &cell_stride, &species_stride);
        musica::DeleteError(&error);

        Napi::Array result = Napi::Array::New(env, 2);
        result.Set(uint32_t(0), Napi::Number::New(env, cell_stride));
        result.Set(uint32_t(1), Napi::Number::New(env, species_stride));

        return result;
    }

    Napi::Value UserDefinedRateParameterStrides(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (!state_) {
            return env.Null();
        }

        musica::Error error;
        size_t cell_stride, param_stride;
        musica::GetUserDefinedRateParametersStrides(state_.get(), &error, &cell_stride, &param_stride);
        musica::DeleteError(&error);

        Napi::Array result = Napi::Array::New(env, 2);
        result.Set(uint32_t(0), Napi::Number::New(env, cell_stride));
        result.Set(uint32_t(1), Napi::Number::New(env, param_stride));

        return result;
    }

    Napi::Value GetInternalStates(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        Napi::Array result = Napi::Array::New(env, 1);
        result.Set(uint32_t(0), Napi::External<musica::State>::New(env, state_.get()));
        return result;
    }

    // Allow MICM class to access internal state
    musica::State* GetInternalState() const {
        return state_.get();
    }

    friend class MICMClass;
};

}
