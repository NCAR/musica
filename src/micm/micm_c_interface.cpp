#include <musica/micm/micm_c_interface.hpp>

namespace musica
{
  MICM *CreateMicm(const char *config_path, MICMSolver solver_type, int num_grid_cells, Error *error)
  {
    MICM *micm = nullptr;
    DeleteError(error);
    try {
      Chemistry chemistry = ReadConfiguration(std::string(config_path));
      micm = new MICM(chemistry, solver_type, num_grid_cells);
      *error = NoError();
    }
    catch (const std::system_error& e)
    {
      *error = ToError(e);
    }
    catch (const std::exception &e)
    {
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_FAILED_TO_CREATE_SOLVER, e.what());
    }

    return micm;
  }

  void DeleteMicm(const MICM *micm, Error *error)
  {
    DeleteError(error);
    try
    {
      delete micm;
      *error = NoError();
    }
    catch (const std::system_error& e)
    {
      *error = ToError(e);
    }
    catch (const std::exception &e)
    {
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_UNKNOWN, e.what());
    }
  }

  void MicmSolve(
      MICM *micm,
      musica::State *state,
      double time_step,
      String *solver_state,
      SolverResultStats *solver_stats,
      Error *error)
  {
    DeleteError(error);
    try
    {
      micm->Solve(micm, state, time_step, solver_state, solver_stats);
      *error = NoError();
    }
    catch (const std::system_error& e)
    {
      *error = ToError(e);
    }
    catch (const std::exception& e)
    {
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_UNKNOWN, e.what());
    }
  };

  String MicmVersion()
  {
    return CreateString(micm::GetMicmVersion());
  }

  Mappings GetSpeciesOrdering(MICM *micm, musica::State *state, Error *error)
  {
    DeleteError(error);
    Mappings species_ordering;

    try {
      std::map<std::string, std::size_t> map = std::visit([](auto &state) { return state.variable_map_; }, state->state_variant_);

      species_ordering.mappings_ = new Mapping[map.size()];
      species_ordering.size_ = map.size();

      // Copy data from the map to the array of structs
      std::size_t i = 0;
      for (const auto &entry : map)
      {
        species_ordering.mappings_[i] = ToMapping(entry.first.c_str(), entry.second);
        ++i;
      }

      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
    catch (const std::exception &e)
    {
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_UNKNOWN, e.what());
    }
    return species_ordering;
  }

  Mappings GetUserDefinedReactionRatesOrdering(MICM *micm, musica::State *state, Error *error)
  {
    DeleteError(error);
    Mappings reaction_rates;

    try
    {
      std::map<std::string, std::size_t> map = std::visit([](auto &state) { return state.custom_rate_parameter_map_; }, state->state_variant_);

      reaction_rates.mappings_ = new Mapping[map.size()];
      reaction_rates.size_ = map.size();

      // Copy data from the map to the array of structs
      std::size_t i = 0;
      for (const auto &entry : map)
      {
        reaction_rates.mappings_[i] = ToMapping(entry.first.c_str(), entry.second);
        ++i;
      }

      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
    catch (const std::exception &e)
    {
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_UNKNOWN, e.what());
    }
    return reaction_rates;
  }

  template<typename T>
  T GetSpeciesProperty(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    DeleteError(error);
    T val = T();
    try {
      std::string species_name_str(species_name);
      std::string property_name_str(property_name);
      val = micm->GetSpeciesProperty<T>(species_name_str, property_name_str);
      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      DeleteError(error);
      *error = ToError(e);
    }
    catch (const std::exception &e)
    {
      DeleteError(error);
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_UNKNOWN, e.what());
    }

    return val;
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

  Mappings GetSpeciesOrderingFortran(MICM *micm, Error *error)
  {
    musica::State *state = CreateMicmState(micm, error);
    auto speciesOrdering = GetSpeciesOrdering(micm, state, error);
    DeleteState(state, error);
    return speciesOrdering;
  }

  Mappings GetUserDefinedReactionRatesOrderingFortran(MICM *micm, Error *error)
  {
    musica::State *state = CreateMicmState(micm, error);
    auto reactionRates = GetUserDefinedReactionRatesOrdering(micm, state, error);
    DeleteState(state, error);
    return reactionRates;
  }

  void MicmSolveFortran(
      MICM *micm,
      double time_step,
      double *temperature,
      double *pressure,
      double *air_density,
      double *concentrations,
      double *custom_rate_parameters,
      String *solver_state,
      SolverResultStats *solver_stats,
      Error *error)
  {
    musica::State *state = CreateMicmState(micm, error);

    std::size_t num_conditions = std::visit([](auto &solver) -> std::size_t { return solver->GetNumberOfGridCells(); }, micm->solver_variant_);

    std::vector<micm::Conditions> conditions_vector(num_conditions);
    for (size_t i = 0; i < num_conditions; i++)
    {
      conditions_vector[i].temperature_ = temperature[i];
      conditions_vector[i].pressure_ = pressure[i];
      conditions_vector[i].air_density_ = air_density[i];
    }
    state->SetOrderedConcentrations(concentrations);
    state->SetOrderedRateConstants(custom_rate_parameters);
    state->SetConditions(conditions_vector);
    MicmSolve(micm, state, time_step, solver_state, solver_stats, error);

    std::vector<double> conc = state->GetOrderedConcentrations();
    for (int i = 0; i < conc.size(); i++)
    {
      concentrations[i] = conc[i];
    }
    DeleteState(state, error);
  }
}