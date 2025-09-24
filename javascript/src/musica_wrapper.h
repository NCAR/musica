#pragma once

#include <string>
#include <memory>

namespace musica_addon {

/**
 * @brief C++ wrapper for MUSICA functionality
 */
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

#endif // MUSICA_WRAPPER_H