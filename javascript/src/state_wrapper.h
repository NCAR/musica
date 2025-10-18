#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>

// Forward declarations of MUSICA types
namespace musica {
    class State;
}

namespace musica_addon {

  /// @brief C++ wrapper for MICM state
class StateWrapper {
public:
    StateWrapper(musica::State* state);
    ~StateWrapper();

    void SetConcentrations(const std::map<std::string, std::vector<double>>& concentrations);
    std::map<std::string, std::vector<double>> GetConcentrations();
    void SetUserDefinedRateParameters(const std::map<std::string, std::vector<double>>& params);
    std::map<std::string, std::vector<double>> GetUserDefinedRateParameters();
    void SetConditions(const std::vector<double>* temperatures,
                      const std::vector<double>* pressures,
                      const std::vector<double>* air_densities);
    std::map<std::string, std::vector<double>> GetConditions();

    std::map<std::string, int> GetSpeciesOrdering();
    std::map<std::string, int> GetUserDefinedRateParametersOrdering();

    void GetConcentrationStrides(size_t& cell_stride, size_t& species_stride);
    void GetUserDefinedRateParameterStrides(size_t& cell_stride, size_t& param_stride);

    size_t GetNumberOfGridCells();

    double* GetConcentrationsPointer(size_t& array_size);
    musica::State* GetState() const { return state_; }

private:
    musica::State* state_;
    bool owns_state_;
};

}
