#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>
#include "state_wrapper.h"
#include <musica/micm/micm.hpp>

// Forward declarations of MUSICA types
namespace musica {
    class State;
}

namespace musica_addon {

/// @brief C++ wrapper for MICM solver
class MICMWrapper {
public:
    // Constructor from config file path
    MICMWrapper(const std::string& config_path, int solver_type);

    // Constructor from JSON/YAML config string
    static MICMWrapper* CreateFromConfigString(const std::string& config_string, int solver_type);

    ~MICMWrapper();

    musica::State* CreateState(size_t number_of_grid_cells);
    void Solve(musica::State* state, double time_step);
    int GetSolverType() const;

private:
    musica::MICM* micm_;
    int solver_type_;

    // Private constructor for string-based initialization
    MICMWrapper(musica::MICM* micm, int solver_type);
};

}