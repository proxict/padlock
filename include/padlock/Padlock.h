#ifndef PADLOCK_PADLOCK_H_
#define PADLOCK_PADLOCK_H_

#include "padlock/AuthUser.h"
#include "padlock/Exception.h"
#include "padlock/gui/PadlockWindow.h"

#include <pwd.h>
#include <string>
#include <unistd.h>

namespace padlock {

namespace {

    std::string getCurrentUser() {
        passwd* pw = getpwuid(getuid());
        if (!pw || !pw->pw_name) {
            throw Exception("Unknown user");
        }
        return pw->pw_name;
    }
} // namespace

class Padlock final {
public:
    Padlock(const std::string& imagePath)
        : mAuthUser(getCurrentUser()) {
        auto app = Gtk::Application::create();
        PadlockGui padlockGui(imagePath, Gtk::WINDOW_POPUP);

        padlockGui.callOnPasswordEntry([this, &padlockGui](const std::string& password) {
            if (unlock(password)) {
                padlockGui.close();
            } else {
                padlockGui.clearPasswordEntry();
            }
        });

        padlockGui.callOnSleepButtonClick([]() { system("systemctl suspend"); });

        app->run(padlockGui);
    }

    bool unlock(const std::string& password) {
        mAuthUser.tryAuth(password);
        return mAuthUser.authenticated();
    }

private:
    Padlock& operator=(const Padlock&) = delete;
    Padlock(const Padlock&) = delete;

    AuthUser mAuthUser;
};

} // namespace padlock

#endif
