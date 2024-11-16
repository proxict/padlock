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
                  if (msgCount == 0) {
                      return 1;
                  }

                  // PAM expects a response for every message
                  if (!(*resp = static_cast<pam_response*>(
                            calloc(static_cast<std::size_t>(msgCount), sizeof(pam_response))))) {
                      perror("calloc");
                      return 1;
                  }

                  for (int i = 0; i < msgCount; ++i) {
                      std::cout << '[' << msgStyleToStr(msg[i]->msg_style) << "]: " << msg[i]->msg << '\n';

                      if (msg[i]->msg_style != PAM_PROMPT_ECHO_OFF &&
                          msg[i]->msg_style != PAM_PROMPT_ECHO_ON) {
                          continue;
                      }

                      if (!(resp[i]->resp = strdup(mPassword.c_str()))) {
                          perror("strdup");
                          return 1;
                      }
                      resp[i]->resp_retcode = 0;
                  }

                  return PAM_SUCCESS;
              }) {}

bool padlock::AuthUser::authenticate(std::string password) {
    mPassword = std::move(password);
    return mPamCtx.authenticate() == pam::AuthStatus::SUCCESS &&
           mPamCtx.validate() == pam::ValidationStatus::SUCCESS;
}
