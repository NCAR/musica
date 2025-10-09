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
    MICMWrapper(const std::string& config_path, int solver_type);
    ~MICMWrapper();

    musica::State* CreateState(size_t number_of_grid_cells);
    void Solve(musica::State* state, double time_step);
    int GetSolverType() const;
    musica::MICM* GetMICM() const { return micm_; }

private:
    musica::MICM* micm_;
    int solver_type_;
};

}