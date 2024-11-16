#ifndef PADLOCK_AUTHUSER_HPP
#define PADLOCK_AUTHUSER_HPP

#include "padlock/pam.hpp"

#include <string>

namespace padlock {

class AuthUser final {
public:
    AuthUser(const std::string& username);

    bool authenticate(std::string password);

    ~AuthUser() = default;

    AuthUser(const AuthUser&) = delete;
    AuthUser& operator=(const AuthUser&) = delete;

    AuthUser(AuthUser&&) = delete;
    AuthUser& operator=(AuthUser&&) = delete;

private:
    pam::Context mPamCtx;
    std::string mPassword;
};

} // namespace padlock

#endif // PADLOCK_AUTHUSER_HPP
