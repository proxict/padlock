#ifndef PADLOCK_PADLOCK_HPP
#define PADLOCK_PADLOCK_HPP

#include "padlock/AuthUser.hpp"

#include <string>

namespace padlock {

class Padlock final {
public:
    Padlock(const std::string& imagePath);

private:
    Padlock& operator=(const Padlock&) = delete;
    Padlock(const Padlock&) = delete;

    AuthUser mAuthUser;
};

} // namespace padlock

#endif // PADLOCK_PADLOCK_HPP
