#ifndef PADLOCK_AUTHUSER_H_
#define PADLOCK_AUTHUSER_H_

#include "padlock/Exception.h"

#include <security/pam_appl.h>
#include <string.h>
#include <string>

namespace padlock {

class AuthUser final {
public:
    AuthUser(const std::string& username)
        : mHandle(nullptr)
        , mResponse(nullptr)
        , mLastStatus(0) {
        mConversation = { functionConversation, (void*)this };
        if (pam_start("su", username.c_str(), &mConversation, &mHandle) != PAM_SUCCESS)
            throw Exception("pam_start error");
    }

    int tryAuth(const std::string& password) {
        mResponse = new pam_response;
        mResponse->resp = strdup(password.c_str()); // set password to authenticate
        mResponse->resp_retcode = 0;

        const int ret = pam_authenticate(mHandle, 0);
        mLastStatus = ret;
        return ret;
    }

    bool authenticated() const { return mLastStatus == PAM_SUCCESS; }

    ~AuthUser() {
        pam_end(mHandle, mLastStatus);
        if (mResponse) {
            delete mResponse;
            mResponse = nullptr;
        }
    }

private:
    pam_handle_t* mHandle;
    pam_response* mResponse;
    pam_conv mConversation;
    bool mLastStatus;

    static int functionConversation(int numMsg,
                                    const struct pam_message** msg,
                                    struct pam_response** resp,
                                    void* appdataPtr) {
        (void)numMsg;
        (void)msg;
        AuthUser* ptr = static_cast<AuthUser*>(appdataPtr);
        *resp = ptr->mResponse;
        return PAM_SUCCESS;
    }
};

} // namespace padlock

#endif // PADLOCK_AUTHUSER_AUTHUSER_H_
