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
    if (micm == nullptr)
    {
      *error = NoError();
      return;
    }
    try
    {
      delete micm;
      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
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
    micm->Solve(micm, state, time_step, solver_state, solver_stats, error);
  };

  String MicmVersion()
  {
    return CreateString(micm::GetMicmVersion());
  }

  Mappings GetSpeciesOrdering(MICM *micm, musica::State *state, Error *error)
  {
    DeleteError(error);

    std::map<std::string, std::size_t> map;
    bool success = false;  // Track if a valid state was found
    std::visit(
        [&map, &success](auto &state)
        {
          map = state.variable_map_;
          success = true;
        },
        state->state_variant_);

    if (!success)
    {
      std::string msg = "State type not recognized or not supported";
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND, msg.c_str());
      return Mappings();
    }

    Mappings species_ordering;
    species_ordering.mappings_ = new Mapping[map.size()];
    species_ordering.size_ = map.size();

    // Copy data from the map to the array of structs
    std::size_t i = 0;
    for (const auto &entry : map)
    {
      species_ordering.mappings_[i] = ToMapping(entry.first.c_str(), entry.second);
      ++i;
    }
    return species_ordering;
  }

  Mappings GetUserDefinedReactionRatesOrdering(MICM *micm, musica::State *state, Error *error)
  {
    DeleteError(error);

    std::map<std::string, std::size_t> map;
    bool success = false;  // Track if a valid state was found
    std::visit(
        [&map, &success](auto &state)
        {
          map = state.custom_rate_parameter_map_;
          success = true;
        },
        state->state_variant_);

    if (!success)
    {
      std::string msg = "State type not recognized or not supported";
      *error = ToError(MUSICA_ERROR_CATEGORY, MUSICA_ERROR_CODE_SOLVER_TYPE_NOT_FOUND, msg.c_str());
      return Mappings();
    }

    Mappings reaction_rates;
    reaction_rates.mappings_ = new Mapping[map.size()];
    reaction_rates.size_ = map.size();

    // Copy data from the map to the array of structs
    std::size_t i = 0;
    for (const auto &entry : map)
    {
      reaction_rates.mappings_[i] = ToMapping(entry.first.c_str(), entry.second);
      ++i;
    }
    return reaction_rates;
  }

  String GetSpeciesPropertyString(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    DeleteError(error);
    std::string species_name_str(species_name);
    std::string property_name_str(property_name);
    const std::string value_str = micm->GetSpeciesProperty<std::string>(species_name_str, property_name_str, error);
    if (!IsSuccess(*error))
    {
      return String();
    }
    return CreateString(value_str.c_str());
  }

  double GetSpeciesPropertyDouble(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    DeleteError(error);
    std::string species_name_str(species_name);
    std::string property_name_str(property_name);
    return micm->GetSpeciesProperty<double>(species_name_str, property_name_str, error);
  }

  int GetSpeciesPropertyInt(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    DeleteError(error);
    std::string species_name_str(species_name);
    std::string property_name_str(property_name);
    return micm->GetSpeciesProperty<int>(species_name_str, property_name_str, error);
  }

  bool GetSpeciesPropertyBool(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    DeleteError(error);
    std::string species_name_str(species_name);
    std::string property_name_str(property_name);
    return micm->GetSpeciesProperty<bool>(species_name_str, property_name_str, error);
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

    std::size_t num_conditions = micm->NumberOfGridCells();

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