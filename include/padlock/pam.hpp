#ifndef PADLOCK_PAM_HPP
#define PADLOCK_PAM_HPP

#include "padlock/Exception.hpp"

#include <security/pam_appl.h>

#include <functional>
#include <string>

namespace padlock {

namespace pam {
    class Exception : public padlock::Exception {
    public:
        using padlock::Exception::Exception;
    };

    enum class AuthStatus {
        SUCCESS,
        USER_UNKNOWN,
        AUTH_ERR,
        CRED_INSUFFICIENT,
        AUTHINFO_UNAVAIL,
        MAXTRIES,
        ABORT,
    };

    enum class ValidationStatus {
        SUCCESS,
        ACCT_EXPIRED,
        AUTH_ERR,
        NEW_AUTHTOK_REQD,
        PERM_DENIED,
        USER_UNKNOWN,
        ABORT,
    };

    class Context final {
    public:
        using ConvFunc = std::function<int(int num_msg, const pam_message** msg, pam_response** response)>;

        Context(const std::string& service_name,
                const std::string& user,
                ConvFunc conversation,
                const std::string& confdir = "");

        ~Context() noexcept;

        [[nodiscard]] AuthStatus authenticate(int flags = 0);

        [[nodiscard]] ValidationStatus validate(int flags = 0);

        operator bool() const { return mHandle != nullptr; }

        pam_handle_t* handle() { return mHandle; }

        operator pam_handle_t*() { return mHandle; }

        int getLastStatus() const { return mLastStatus; }

        int setLastStatus(int status) {
            mLastStatus = status;
            return mLastStatus;
        }

        Context(const Context&) = delete;
        Context(Context&& other);
        Context& operator=(Context other);

    private:
        pam_handle_t* mHandle;
        ConvFunc mConvFunc;
        int mLastStatus;
    };
} // namespace pam

} // namespace padlock

#endif // PADLOCK_PAM_HPP
