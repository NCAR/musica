#include "micm_wrapper.h"
#include "state_wrapper.h"
#include <sstream>
#include <memory>
#include <cstring>
#include <stdexcept>

// Include MUSICA headers for real functionality
#include <musica/micm/micm_c_interface.hpp>
#include <musica/micm/state_c_interface.hpp>
#include <musica/micm/micm.hpp>
#include <musica/util.hpp>
#include <musica/version.hpp>
#include <musica/component_versions.hpp>
#include <micm/system/conditions.hpp>

namespace musica_addon {

// ============================================================================
// MICMWrapper Implementation
// ============================================================================

MICMWrapper::MICMWrapper(const std::string& config_path, int solver_type)
    : micm_(nullptr), solver_type_(solver_type) {
    musica::Error error;
    micm_ = musica::CreateMicm(
        config_path.c_str(),
        static_cast<musica::MICMSolver>(solver_type),
        &error);

    if (!musica::IsSuccess(error)) {
        std::string error_msg = "Failed to create MICM solver: ";
        if (error.message_.value_ != nullptr) {
            error_msg += error.message_.value_;
            musica::DeleteString(&error.message_);
        }
        musica::DeleteError(&error);
        throw std::runtime_error(error_msg);
    }
    musica::DeleteError(&error);
}

MICMWrapper::~MICMWrapper() {
    if (micm_ != nullptr) {
        musica::Error error;
        musica::DeleteMicm(micm_, &error);
        musica::DeleteError(&error);
    }
}

musica::State* MICMWrapper::CreateState(size_t number_of_grid_cells) {
    musica::Error error;
    musica::State* state = musica::CreateMicmState(micm_, number_of_grid_cells, &error);

    if (!musica::IsSuccess(error)) {
        std::string error_msg = "Failed to create state: ";
        if (error.message_.value_ != nullptr) {
            error_msg += error.message_.value_;
            musica::DeleteString(&error.message_);
        }
        musica::DeleteError(&error);
        throw std::runtime_error(error_msg);
    }
    musica::DeleteError(&error);
    return state;
}

void MICMWrapper::Solve(musica::State* state, double time_step) {
    musica::Error error;
    musica::String solver_state;
    musica::SolverResultStats stats;

    musica::MicmSolve(micm_, state, time_step, &solver_state, &stats, &error);

    if (!musica::IsSuccess(error)) {
        std::string error_msg = "Failed to solve: ";
        if (error.message_.value_ != nullptr) {
            error_msg += error.message_.value_;
            musica::DeleteString(&error.message_);
        }
        musica::DeleteString(&solver_state);
        musica::DeleteError(&error);
        throw std::runtime_error(error_msg);
    }

    musica::DeleteString(&solver_state);
    musica::DeleteError(&error);
}

int MICMWrapper::GetSolverType() const {
    return solver_type_;
}

}
