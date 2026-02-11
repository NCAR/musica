#include <musica/micm/cuda_availability.hpp>
#include <musica/micm/micm_c_interface.hpp>
#include <musica/micm/parse.hpp>

namespace musica
{
  template<typename Func>
  auto HandleErrors(Func func, Error* error) -> decltype(func())
  {
    DeleteError(error);
    try
    {
      return func();
    }
    catch (const std::system_error& e)
    {
      ToError(e, error);
    }
    catch (const std::exception& e)
    {
      ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_UNKNOWN, e.what(), error);
    }
    catch (...)
    {
      ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_UNKNOWN, "Unknown error", error);
    }
    return decltype(func())();
  }

  MICM* CreateMicm(const char* config_path, MICMSolver solver_type, Error* error)
  {
    return HandleErrors(
        [&]()
        {
          Chemistry const chemistry = ReadConfiguration(std::string(config_path));
          MICM* micm = new MICM(chemistry, solver_type);
          NoError(error);
          return micm;
        },
        error);
  }

  MICM* CreateMicmFromChemistryMechanism(const Chemistry* chemistry, MICMSolver solver_type, Error* error)
  {
    return HandleErrors(
        [&]()
        {
          MICM* micm = new MICM(*chemistry, solver_type);
          NoError(error);
          return micm;
        },
        error);
  }

  MICM* CreateMicmFromConfigString(const char* config_string, MICMSolver solver_type, Error* error)
  {
    return HandleErrors(
        [&]()
        {
          // Parse JSON/YAML string to Chemistry object
          Chemistry chemistry = ReadConfigurationFromString(std::string(config_string));
          MICM* micm = new MICM(chemistry, solver_type);
          NoError(error);
          return micm;
        },
        error);
  }

  void DeleteMicm(MICM* micm, Error* error)
  {
    HandleErrors(
        [&]()
        {
          delete micm;
          NoError(error);
        },
        error);
  }

  void MicmSolve(
      MICM* micm,
      musica::State* state,
      double time_step,
      String* solver_state,
      SolverResultStats* solver_stats,
      Error* error)
  {
    HandleErrors(
        [&]()
        {
          micm::SolverResult result = micm->Solve(state, time_step);
          *solver_stats = result.stats_;
          CreateString(micm::SolverStateToString(result.state_).c_str(), solver_state);
          NoError(error);
        },
        error);
  }

  void MicmVersion(String* micm_version)
  {
    CreateString(micm::GetMicmVersion(), micm_version);
  }

  template<typename T>
  T GetSpeciesProperty(MICM* micm, const char* species_name, const char* property_name, Error* error)
  {
    return HandleErrors(
        [&]()
        {
          std::string const species_name_str(species_name);
          std::string const property_name_str(property_name);
          T val = micm->GetSpeciesProperty<T>(species_name_str, property_name_str);
          NoError(error);
          return val;
        },
        error);
  }

  void GetSpeciesPropertyString(
      MICM* micm,
      const char* species_name,
      const char* property_name,
      String* species_property,
      Error* error)
  {
    std::string const val = GetSpeciesProperty<std::string>(micm, species_name, property_name, error);
    if (!IsSuccess(*error))
    {
      *species_property = String();
      return;
    }
    CreateString(val.c_str(), species_property);
  }

  double GetSpeciesPropertyDouble(MICM* micm, const char* species_name, const char* property_name, Error* error)
  {
    return GetSpeciesProperty<double>(micm, species_name, property_name, error);
  }

  int GetSpeciesPropertyInt(MICM* micm, const char* species_name, const char* property_name, Error* error)
  {
    return GetSpeciesProperty<int>(micm, species_name, property_name, error);
  }

  bool GetSpeciesPropertyBool(MICM* micm, const char* species_name, const char* property_name, Error* error)
  {
    return GetSpeciesProperty<bool>(micm, species_name, property_name, error);
  }

  bool _IsCudaAvailable(Error* error)
  {
    return HandleErrors([&]() { return musica::IsCudaAvailable(); }, error);
  }

  size_t GetMaximumNumberOfGridCells(MICM* micm)
  {
    return micm->GetMaximumNumberOfGridCells();
  }

  std::size_t GetVectorSize(musica::MICMSolver solver_type)
  {
    constexpr std::size_t nonvectorized_size = 1;
    switch (solver_type)
    {
      case musica::MICMSolver::Rosenbrock:
      case musica::MICMSolver::BackwardEuler:
      case musica::MICMSolver::CudaRosenbrock: return musica::MUSICA_VECTOR_SIZE;
      case musica::MICMSolver::RosenbrockStandardOrder:
      case musica::MICMSolver::BackwardEulerStandardOrder: return nonvectorized_size;
      default: throw std::runtime_error("Invalid MICM solver type.");
    }
  }

  // Helper: convert C struct to C++ struct
  static musica::RosenbrockSolverParameters ToRosenbrockParams(const RosenbrockSolverParametersC* c_params)
  {
    musica::RosenbrockSolverParameters params;
    params.relative_tolerance = c_params->relative_tolerance;
    params.h_min = c_params->h_min;
    params.h_max = c_params->h_max;
    params.h_start = c_params->h_start;
    params.max_number_of_steps = c_params->max_number_of_steps;
    if (c_params->absolute_tolerances != nullptr && c_params->num_absolute_tolerances > 0)
    {
      params.absolute_tolerances.assign(
          c_params->absolute_tolerances, c_params->absolute_tolerances + c_params->num_absolute_tolerances);
    }
    return params;
  }

  static musica::BackwardEulerSolverParameters ToBackwardEulerParams(const BackwardEulerSolverParametersC* c_params)
  {
    musica::BackwardEulerSolverParameters params;
    params.relative_tolerance = c_params->relative_tolerance;
    params.max_number_of_steps = c_params->max_number_of_steps;
    if (c_params->absolute_tolerances != nullptr && c_params->num_absolute_tolerances > 0)
    {
      params.absolute_tolerances.assign(
          c_params->absolute_tolerances, c_params->absolute_tolerances + c_params->num_absolute_tolerances);
    }
    if (c_params->time_step_reductions != nullptr && c_params->num_time_step_reductions > 0)
    {
      params.time_step_reductions.assign(
          c_params->time_step_reductions, c_params->time_step_reductions + c_params->num_time_step_reductions);
    }
    return params;
  }

  void SetRosenbrockSolverParameters(MICM* micm, const RosenbrockSolverParametersC* params, Error* error)
  {
    HandleErrors(
        [&]()
        {
          micm->SetSolverParameters(ToRosenbrockParams(params));
          NoError(error);
        },
        error);
  }

  void SetBackwardEulerSolverParameters(MICM* micm, const BackwardEulerSolverParametersC* params, Error* error)
  {
    HandleErrors(
        [&]()
        {
          micm->SetSolverParameters(ToBackwardEulerParams(params));
          NoError(error);
        },
        error);
  }

  void GetRosenbrockSolverParameters(MICM* micm, RosenbrockSolverParametersC* params, Error* error)
  {
    HandleErrors(
        [&]()
        {
          auto cpp_params = micm->GetRosenbrockSolverParameters();
          params->relative_tolerance = cpp_params.relative_tolerance;
          params->h_min = cpp_params.h_min;
          params->h_max = cpp_params.h_max;
          params->h_start = cpp_params.h_start;
          params->max_number_of_steps = cpp_params.max_number_of_steps;
          if (!cpp_params.absolute_tolerances.empty())
          {
            params->num_absolute_tolerances = cpp_params.absolute_tolerances.size();
            params->absolute_tolerances = new double[params->num_absolute_tolerances];
            std::copy(
                cpp_params.absolute_tolerances.begin(),
                cpp_params.absolute_tolerances.end(),
                params->absolute_tolerances);
          }
          else
          {
            params->absolute_tolerances = nullptr;
            params->num_absolute_tolerances = 0;
          }
          NoError(error);
        },
        error);
  }

  void GetBackwardEulerSolverParameters(MICM* micm, BackwardEulerSolverParametersC* params, Error* error)
  {
    HandleErrors(
        [&]()
        {
          auto cpp_params = micm->GetBackwardEulerSolverParameters();
          params->relative_tolerance = cpp_params.relative_tolerance;
          params->max_number_of_steps = cpp_params.max_number_of_steps;
          if (!cpp_params.absolute_tolerances.empty())
          {
            params->num_absolute_tolerances = cpp_params.absolute_tolerances.size();
            params->absolute_tolerances = new double[params->num_absolute_tolerances];
            std::copy(
                cpp_params.absolute_tolerances.begin(),
                cpp_params.absolute_tolerances.end(),
                params->absolute_tolerances);
          }
          else
          {
            params->absolute_tolerances = nullptr;
            params->num_absolute_tolerances = 0;
          }
          if (!cpp_params.time_step_reductions.empty())
          {
            params->num_time_step_reductions = cpp_params.time_step_reductions.size();
            params->time_step_reductions = new double[params->num_time_step_reductions];
            std::copy(
                cpp_params.time_step_reductions.begin(),
                cpp_params.time_step_reductions.end(),
                params->time_step_reductions);
          }
          else
          {
            params->time_step_reductions = nullptr;
            params->num_time_step_reductions = 0;
          }
          NoError(error);
        },
        error);
  }

}  // namespace musica