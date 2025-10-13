#pragma once

#include <string>
#include <memory>

namespace musica_addon {

/// @brief C++ wrapper for exposing MUSICA functionality to Node.js
class MusicaWrapper {
public:
    MusicaWrapper();
    ~MusicaWrapper();

    // Version and build information
    std::string GetVersion() const;

    private:
    // Internal implementation details
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};
}
