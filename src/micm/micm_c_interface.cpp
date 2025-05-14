#include <musica/micm/cuda_availability.hpp>
#include <musica/micm/micm_c_interface.hpp>

namespace musica
{
  template<typename Func>
  auto HandleErrors(Func func, Error *error) -> decltype(func())
  {
    DeleteError(error);
    try
    {
      return func();
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
    catch (const std::exception &e)
    {
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_UNKNOWN, e.what());
    }
    catch (...)
    {
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_UNKNOWN, "Unknown error");
    }
    return decltype(func())();
  }

  MICM *CreateMicm(const char *config_path, MICMSolver solver_type, Error *error)
  {
    return HandleErrors(
        [&]()
        {
          Chemistry chemistry = ReadConfiguration(std::string(config_path));
          MICM *micm = new MICM(chemistry, solver_type);
          *error = NoError();
          return micm;
        },
        error);
  }

  MICM *CreateMicmFromChemistryMechanism(const Chemistry *chemistry, MICMSolver solver_type, Error *error)
  {
    return HandleErrors(
        [&]()
        {
          MICM *micm = new MICM(*chemistry, solver_type);
          *error = NoError();
          return micm;
        },
        error);
  }

  void DeleteMicm(const MICM *micm, Error *error)
  {
    HandleErrors(
        [&]()
        {
          delete micm;
          *error = NoError();
        },
        error);
  }

  void MicmSolve(
      MICM *micm,
      musica::State *state,
      double time_step,
      String *solver_state,
      SolverResultStats *solver_stats,
      Error *error)
  {
    HandleErrors(
        [&]()
        {
          micm->Solve(state, time_step, solver_state, solver_stats);
          *error = NoError();
        },
        error);
  }

  String MicmVersion()
  {
    return CreateString(micm::GetMicmVersion());
  }

  template<typename T>
  T GetSpeciesProperty(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    return HandleErrors(
        [&]()
        {
          std::string species_name_str(species_name);
          std::string property_name_str(property_name);
          T val = micm->GetSpeciesProperty<T>(species_name_str, property_name_str);
          *error = NoError();
          return val;
        },
        error);
  }

  String GetSpeciesPropertyString(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    std::string val = GetSpeciesProperty<std::string>(micm, species_name, property_name, error);
    if (!IsSuccess(*error))
    {
      return String();
    }
    return CreateString(val.c_str());
  }

  double GetSpeciesPropertyDouble(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    return GetSpeciesProperty<double>(micm, species_name, property_name, error);
  }

  int GetSpeciesPropertyInt(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    return GetSpeciesProperty<int>(micm, species_name, property_name, error);
  }

  bool GetSpeciesPropertyBool(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    return GetSpeciesProperty<bool>(micm, species_name, property_name, error);
  }

  bool _IsCudaAvailable(Error *error)
  {
    return HandleErrors([&]() { return musica::IsCudaAvailable(); }, error);
  }

  size_t GetMaximumNumberOfGridCells(MICM *micm)
  {
    return micm->GetMaximumNumberOfGridCells();
  }

}  // namespace musica