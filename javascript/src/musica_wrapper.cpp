#include "musica_wrapper.h"
#include <musica/version.hpp>

namespace musica_addon {

class MusicaWrapper::Impl {
public:
};

MusicaWrapper::MusicaWrapper() : pImpl_(std::make_unique<Impl>()) {
}

MusicaWrapper::~MusicaWrapper() = default;

std::string MusicaWrapper::GetVersion() const {
    const char* version = musica::GetMusicaVersion();
    if (version != nullptr) {
        return std::string(version);
    }
    return "";
}
}