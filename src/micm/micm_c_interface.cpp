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

}  // namespace musica