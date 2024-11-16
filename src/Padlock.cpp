#include "padlock/Padlock.hpp"
#include "padlock/Exception.hpp"
#include "padlock/gui/PadlockWindow.hpp"

extern "C" {
#include <pwd.h>
#include <unistd.h>
}

#include <cstring>

static std::string getCurrentUser() {
    passwd* pw = getpwuid(getuid());
    if (!pw || !pw->pw_name) {
        throw padlock::Exception("Failed to get current user: ", std::strerror(errno));
    }
    return pw->pw_name;
}

padlock::Padlock::Padlock(const std::string& imagePath)
    : mAuthUser(getCurrentUser()) {
    auto app = Gtk::Application::create();
    PadlockGui padlockGui(imagePath);

    padlockGui.callOnPasswordEntry([this, &padlockGui](const std::string& password) {
        // TODO: consider running this in another thread and just notify here.
        if (mAuthUser.authenticate(password)) {
            padlockGui.close();
        } else {
            padlockGui.clearPasswordEntry();
        }
    });

    padlockGui.callOnSleepButtonClick([]() { system("systemctl suspend"); });

    app->signal_activate().connect([&]() {
        app->add_window(padlockGui);
        padlockGui.present();
    });

    app->run();
}
