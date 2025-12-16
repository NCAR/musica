#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

// Forward declarations of MUSICA types
namespace musica
{
  class State;
}

namespace musica_addon
{

  // Custom deleter for musica::State
  struct StateDeleter
  {
    void operator()(musica::State* state) const;
  };

  /// @brief C++ wrapper for MICM state
  class StateWrapper
  {
   public:
    StateWrapper(musica::State* state);
    ~StateWrapper() = default;

    void SetConcentrations(const std::map<std::string, std::vector<double>>& concentrations);
    std::map<std::string, std::vector<double>> GetConcentrations();
    void SetUserDefinedRateParameters(const std::map<std::string, std::vector<double>>& params);
    std::map<std::string, std::vector<double>> GetUserDefinedRateParameters();
    void SetConditions(
        const std::vector<double>* temperatures,
        const std::vector<double>* pressures,
        const std::vector<double>* air_densities);
    std::map<std::string, std::vector<double>> GetConditions();

    std::map<std::string, int> GetSpeciesOrdering();
    std::map<std::string, int> GetUserDefinedRateParametersOrdering();

    void GetConcentrationStrides(size_t& cell_stride, size_t& species_stride);
    void GetUserDefinedRateParameterStrides(size_t& cell_stride, size_t& param_stride);

    size_t GetNumberOfGridCells();

    double* GetConcentrationsPointer(size_t& array_size);
    musica::State* GetState() const
    {
      return state_.get();
    }

   private:
    std::unique_ptr<musica::State, StateDeleter> state_;
  };

}  // namespace musica_addon
