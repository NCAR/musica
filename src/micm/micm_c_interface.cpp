#include <musica/micm/micm_c_interface.hpp>

namespace musica
{
  template <typename Func>
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
    return decltype(func())();
  }

  MICM *CreateMicm(const char *config_path, MICMSolver solver_type, int num_grid_cells, Error *error)
  {
    return HandleErrors([&]() {
      Chemistry chemistry = ReadConfiguration(std::string(config_path));
      MICM *micm = new MICM(chemistry, solver_type, num_grid_cells);
      *error = NoError();
      return micm;
    }, error);
  }

  void DeleteMicm(const MICM *micm, Error *error)
  {
    HandleErrors([&]() {
      delete micm;
      *error = NoError();
    }, error);
  }

  void MicmSolve(
      MICM *micm,
      musica::State *state,
      double time_step,
      String *solver_state,
      SolverResultStats *solver_stats,
      Error *error)
  {
    HandleErrors([&]() {
      micm->Solve(micm, state, time_step, solver_state, solver_stats);
      *error = NoError();
    }, error);
  }

  String MicmVersion()
  {
    return CreateString(micm::GetMicmVersion());
  }

  Mappings GetSpeciesOrdering(MICM *micm, musica::State *state, Error *error)
  {
    return HandleErrors([&]() {
      Mappings species_ordering;
      std::map<std::string, std::size_t> map = std::visit([](auto &state) { return state.variable_map_; }, state->state_variant_);

      species_ordering.mappings_ = new Mapping[map.size()];
      species_ordering.size_ = map.size();

      std::size_t i = 0;
      for (const auto &entry : map)
      {
        species_ordering.mappings_[i] = ToMapping(entry.first.c_str(), entry.second);
        ++i;
      }

      *error = NoError();
      return species_ordering;
    }, error);
  }

  Mappings GetUserDefinedReactionRatesOrdering(MICM *micm, musica::State *state, Error *error)
  {
    return HandleErrors([&]() {
      Mappings reaction_rates;
      std::map<std::string, std::size_t> map = std::visit([](auto &state) { return state.custom_rate_parameter_map_; }, state->state_variant_);

      reaction_rates.mappings_ = new Mapping[map.size()];
      reaction_rates.size_ = map.size();

      std::size_t i = 0;
      for (const auto &entry : map)
      {
        reaction_rates.mappings_[i] = ToMapping(entry.first.c_str(), entry.second);
        ++i;
      }

      *error = NoError();
      return reaction_rates;
    }, error);
  }

  template <typename T>
  T GetSpeciesProperty(MICM *micm, const char *species_name, const char *property_name, Error *error)
  {
    return HandleErrors([&]() {
      std::string species_name_str(species_name);
      std::string property_name_str(property_name);
      T val = micm->GetSpeciesProperty<T>(species_name_str, property_name_str);
      *error = NoError();
      return val;
    }, error);
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