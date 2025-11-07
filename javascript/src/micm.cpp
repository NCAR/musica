// #include <napi.h>
// #include <memory>
// #include <musica/micm/micm.hpp>
// #include <musica/micm/parse.hpp>
// #include <musica/util.hpp>
// #include <mechanism_configuration/v1/mechanism.hpp>

// // Forward declare classes
// namespace musica {
//     class Mechanism;
// }

// namespace musica_addon {
//     class StateClass;
// }

// namespace musica_addon {

// /**
//  * MICM class - N-API wrapper
//  *
//  * This class wraps musica::MICM and creates State objects that internally
//  * manage multiple musica::State instances for vector-ordered solvers.
//  */
// class MICMClass : public Napi::ObjectWrap<MICMClass> {
// public:
//     static Napi::FunctionReference* state_constructor;

//     static Napi::Object Init(Napi::Env env, Napi::Object exports) {
//         Napi::Function func = DefineClass(env, "MICM", {
//             InstanceMethod("createState", &MICMClass::CreateState),
//             InstanceMethod("solve", &MICMClass::Solve),
//             InstanceMethod("getSolverType", &MICMClass::GetSolverType),
//             InstanceMethod("getVectorSize", &MICMClass::GetVectorSize),
//         });

//         auto* constructor = new Napi::FunctionReference();
//         *constructor = Napi::Persistent(func);
//         env.SetInstanceData(constructor);

//         exports.Set("MICM", func);
//         return exports;
//     }

//     MICMClass(const Napi::CallbackInfo& info) : Napi::ObjectWrap<MICMClass>(info) {
//         Napi::Env env = info.Env();

//         if (info.Length() < 2) {
//             Napi::TypeError::New(env, "Expected 2 arguments: (config_path or mechanism) and solver_type")
//                 .ThrowAsJavaScriptException();
//             return;
//         }

//         if (!info[1].IsNumber()) {
//             Napi::TypeError::New(env, "solver_type must be a number").ThrowAsJavaScriptException();
//             return;
//         }

//         int solver_type = info[1].As<Napi::Number>().Int32Value();

//         try {
//             // Check if first argument is a string (config path) or object (Mechanism)
//             if (info[0].IsString()) {
//                 // File path constructor
//                 std::string config_path = info[0].As<Napi::String>().Utf8Value();
//                 micm_ = std::make_unique<musica::MICM>(config_path.c_str(), static_cast<musica::MICMSolver>(solver_type));
//             }
//             else if (info[0].IsObject()) {
//                 // Mechanism object constructor
//                 Napi::Object mechanism_obj = info[0].As<Napi::Object>();

//                 // Unwrap the Mechanism object
//                 musica::Mechanism* mechanism = musica::Mechanism::Unwrap(mechanism_obj);
//                 if (!mechanism) {
//                     Napi::TypeError::New(env, "First argument must be a string path or Mechanism object")
//                         .ThrowAsJavaScriptException();
//                     return;
//                 }

//                 // Get the internal v1::types::Mechanism
//                 const mechanism_configuration::v1::types::Mechanism& v1_mechanism = mechanism->GetInternalMechanism();

//                 // Convert v1 mechanism to Chemistry object
//                 musica::Chemistry chemistry = musica::ConvertV1Mechanism(v1_mechanism);

//                 // Create MICM from Chemistry
//                 micm_ = std::make_unique<musica::MICM>(chemistry, static_cast<musica::MICMSolver>(solver_type));
//             }
//             else {
//                 Napi::TypeError::New(env, "First argument must be a string path or Mechanism object")
//                     .ThrowAsJavaScriptException();
//                 return;
//             }

//             solver_type_ = solver_type;

//             // Determine vector size based on solver type
//             switch (static_cast<musica::MICMSolver>(solver_type)) {
//                 case musica::MICMSolver::Rosenbrock:
//                 case musica::MICMSolver::BackwardEuler:
//                 case musica::MICMSolver::CudaRosenbrock:
//                     vector_size_ = musica::MUSICA_VECTOR_SIZE;
//                     break;
//                 case musica::MICMSolver::RosenbrockStandardOrder:
//                 case musica::MICMSolver::BackwardEulerStandardOrder:
//                     vector_size_ = 1;
//                     break;
//                 default:
//                     vector_size_ = 1;
//                     break;
//             }
//         } catch (const std::exception& e) {
//             Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
//         }
//     }

// private:
//     std::unique_ptr<musica::MICM> micm_;
//     int solver_type_;
//     size_t vector_size_;

//     Napi::Value CreateState(const Napi::CallbackInfo& info) {
//         Napi::Env env = info.Env();

//         if (info.Length() < 1) {
//             Napi::TypeError::New(env, "Expected 1 argument: number_of_grid_cells")
//                 .ThrowAsJavaScriptException();
//             return env.Null();
//         }

//         if (!info[0].IsNumber()) {
//             Napi::TypeError::New(env, "number_of_grid_cells must be a number")
//                 .ThrowAsJavaScriptException();
//             return env.Null();
//         }

//         size_t num_cells = info[0].As<Napi::Number>().Int64Value();

//         try {
//             // Create StateClass instance with solver, number_of_grid_cells, and vector_size
//             if (state_constructor == nullptr) {
//                 Napi::Error::New(env, "State constructor not initialized").ThrowAsJavaScriptException();
//                 return env.Null();
//             }

//             Napi::Object state_obj = state_constructor->New({
//                 Napi::External<musica::MICM>::New(env, micm_.get()),
//                 Napi::Number::New(env, num_cells),
//                 Napi::Number::New(env, vector_size_)
//             });

//             return state_obj;
//         } catch (const std::exception& e) {
//             Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
//             return env.Null();
//         }
//     }

//     Napi::Value Solve(const Napi::CallbackInfo& info) {
//         Napi::Env env = info.Env();

//         if (info.Length() < 2) {
//             Napi::TypeError::New(env, "Expected 2 arguments: state and time_step")
//                 .ThrowAsJavaScriptException();
//             return env.Undefined();
//         }

//         if (!info[0].IsObject() || !info[1].IsNumber()) {
//             Napi::TypeError::New(env, "Invalid arguments").ThrowAsJavaScriptException();
//             return env.Undefined();
//         }

//         Napi::Object state_obj = info[0].As<Napi::Object>();
//         StateClass* state_class = StateClass::Unwrap(state_obj);
//         double time_step = info[1].As<Napi::Number>().DoubleValue();

//         try {
//             // Solve single internal state
//             musica::State* state = state_class->GetInternalState();
//             musica::String solver_state;
//             musica::SolverResultStats stats;
//             micm_->Solve(state, time_step, &solver_state, &stats);
//         } catch (const std::exception& e) {
//             Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
//         }

//         return env.Undefined();
//     }

//     Napi::Value GetSolverType(const Napi::CallbackInfo& info) {
//         Napi::Env env = info.Env();
//         return Napi::Number::New(env, solver_type_);
//     }

//     Napi::Value GetVectorSize(const Napi::CallbackInfo& info) {
//         Napi::Env env = info.Env();
//         return Napi::Number::New(env, vector_size_);
//     }

//     friend class StateClass;
// };

// // Initialize static member
// Napi::FunctionReference* MICMClass::state_constructor = nullptr;

// }
