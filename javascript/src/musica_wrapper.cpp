#include "musica_wrapper.h"
#include <sstream>
#include <memory>
#include <cstring>

// Include MUSICA headers for real functionality
#include <musica/micm/micm_c_interface.hpp>
#include <musica/micm/state_c_interface.hpp>
#include <musica/micm/micm.hpp>
#include <musica/util.hpp>
#include <musica/version.hpp>
#include <musica/component_versions.hpp>

namespace musica_addon {

// Forward declaration of implementation
class MusicaWrapper::Impl {
public:
    musica::MICM* micm_solver_ = nullptr;
    musica::State* micm_state_ = nullptr;
    bool has_solver_ = false;

    Impl() = default;
    ~Impl() {
        musica::Error error;
        if (micm_state_) {
            musica::DeleteState(micm_state_, &error);
        }
        if (micm_solver_) {
            musica::DeleteMicm(micm_solver_, &error);
        }
    }
};

MusicaWrapper::MusicaWrapper() : pImpl_(std::make_unique<Impl>()) {
}

MusicaWrapper::~MusicaWrapper() = default;

std::string MusicaWrapper::GetVersion() const {
    const char* version = musica::GetMusicaVersion();
    if (version != nullptr) {
        return std::string(version);
    }
    return ""; // Return empty if MUSICA function fails
}
}