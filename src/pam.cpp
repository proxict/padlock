#include "padlock/pam.hpp"

#include <utility>

using namespace padlock;

pam::Context::Context(const std::string& service_name,
                      const std::string& user,
                      ConvFunc conversation,
                      const std::string& confdir)
    : mHandle(nullptr)
    , mConvFunc(std::move(conversation))
    , mLastStatus(PAM_SUCCESS) {
    pam_conv conv;
    conv.conv = [](int num_msg, const pam_message** msg, pam_response** response, void* userdata) -> int {
        auto self = reinterpret_cast<Context*>(userdata);
        if (self->mConvFunc) {
            return self->mConvFunc(num_msg, msg, response);
        }
        return PAM_ABORT;
    };
    conv.appdata_ptr = this;

    mLastStatus = ::pam_start_confdir(
        service_name.c_str(), user.c_str(), &conv, confdir.empty() ? nullptr : confdir.c_str(), &mHandle);
    switch (mLastStatus) {
    case PAM_SUCCESS:
        break;
    case PAM_BUF_ERR:
        throw pam::Exception("pam_start error: memory buffer error");
    case PAM_SYSTEM_ERR:
        throw pam::Exception("pam_start error: system error");
    case PAM_ABORT:
    default:
        throw pam::Exception("pam_start error");
    }
}

pam::Context::~Context() noexcept {
    if (mHandle) {
        pam_end(mHandle, mLastStatus);
        mHandle = nullptr;
    }
}

[[nodiscard]] pam::AuthStatus pam::Context::authenticate(int flags) {
    mLastStatus = pam_authenticate(mHandle, flags);
    switch (mLastStatus) {
    case PAM_SUCCESS:
        return AuthStatus::SUCCESS;
    case PAM_AUTH_ERR:
        return AuthStatus::AUTH_ERR;
    case PAM_CRED_INSUFFICIENT:
        return AuthStatus::CRED_INSUFFICIENT;
    case PAM_AUTHINFO_UNAVAIL:
        return AuthStatus::AUTHINFO_UNAVAIL;
    case PAM_USER_UNKNOWN:
        return AuthStatus::USER_UNKNOWN;
    case PAM_MAXTRIES:
        return AuthStatus::MAXTRIES;

    case PAM_ABORT:
    default:
        return AuthStatus::ABORT;
    }
}

[[nodiscard]] pam::ValidationStatus pam::Context::validate(int flags) {
    mLastStatus = pam_acct_mgmt(mHandle, flags);
    switch (mLastStatus) {
    case PAM_SUCCESS:
        return ValidationStatus::SUCCESS;
    case PAM_ACCT_EXPIRED:
        return ValidationStatus::ACCT_EXPIRED;
    case PAM_AUTH_ERR:
        return ValidationStatus::AUTH_ERR;
    case PAM_NEW_AUTHTOK_REQD:
        return ValidationStatus::NEW_AUTHTOK_REQD;
    case PAM_PERM_DENIED:
        return ValidationStatus::PERM_DENIED;
    case PAM_USER_UNKNOWN:
        return ValidationStatus::USER_UNKNOWN;
    default:
        return ValidationStatus::ABORT;
    }
}

pam::Context::Context(pam::Context&& other)
    : mHandle(std::exchange(other.mHandle, nullptr))
    , mConvFunc(std::move(other.mConvFunc))
    , mLastStatus(other.mLastStatus) {}

pam::Context& pam::Context::operator=(pam::Context other) {
    using std::swap;
    swap(mHandle, other.mHandle);
    swap(mConvFunc, other.mConvFunc);
    swap(mLastStatus, other.mLastStatus);
    return *this;
}
