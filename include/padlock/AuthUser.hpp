#ifndef PADLOCK_AUTHUSER_HPP
#define PADLOCK_AUTHUSER_HPP

#include "padlock/pam.hpp"

#include <optional>
#include <string>

namespace padlock {

class AuthUser final {
public:
    AuthUser(const std::string& username);

    bool authenticate(const std::string& password);

    ~AuthUser() noexcept;

    AuthUser(const AuthUser&) = delete;
    AuthUser& operator=(const AuthUser&) = delete;

    AuthUser(AuthUser&&) = delete;
    AuthUser& operator=(AuthUser&&) = delete;

private:
    void clearResponse();

    pam::Context mPamCtx;
    std::optional<pam_response> mResponse;
};

} // namespace padlock

#endif // PADLOCK_AUTHUSER_HPP
