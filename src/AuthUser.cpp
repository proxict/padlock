#include "padlock/AuthUser.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string_view>

static const std::string_view msgStyleToStr(const int style) {
    switch (style) {
    case PAM_PROMPT_ECHO_OFF:
        return "ECHO OFF";
    case PAM_PROMPT_ECHO_ON:
        return "ECHO ON";
    case PAM_ERROR_MSG:
        return "ERROR";
    case PAM_TEXT_INFO:
        return "INFO";
    default:
        return "UNKNOWN";
    }
}

padlock::AuthUser::AuthUser(const std::string& username)
    : mPamCtx("login",
              username,
              [this](int msgCount, const struct pam_message** msg, struct pam_response** resp) -> int {
                  for (int i = 0; i < msgCount; ++i) {
                      std::cout << '[' << msgStyleToStr(msg[i]->msg_style) << "]: " << msg[i]->msg << '\n';
                  }
                  *resp = &mResponse.value();
                  return PAM_SUCCESS;
              }) {}

bool padlock::AuthUser::authenticate(const std::string& password) {
    clearResponse();
    mResponse.emplace();
    mResponse->resp = strdup(password.c_str());
    mResponse->resp_retcode = 0;

    return mPamCtx.authenticate() == pam::AuthStatus::SUCCESS &&
           mPamCtx.validate() == pam::ValidationStatus::SUCCESS;
}

void padlock::AuthUser::clearResponse() {
    if (mResponse) {
        free(mResponse->resp);
        mResponse.reset();
    }
}

padlock::AuthUser::~AuthUser() noexcept {
    clearResponse();
}

