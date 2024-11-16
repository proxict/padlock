#ifndef PADLOCK_GUI_PADLOCKWINDOW_HPP
#define PADLOCK_GUI_PADLOCKWINDOW_HPP

#define GDKMM_DISABLE_DEPRECATED 1
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include <gdkmm.h>
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <gdkmm/screen.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/fixed.h>
#include <gtkmm/image.h>
#include <gtkmm/overlay.h>
#include <gtkmm/stylecontext.h>
#include <gtkmm/window.h>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

namespace padlock {

class PadlockGui final : public Gtk::Window {
public:
    PadlockGui(const std::string& imgPath);

    virtual ~PadlockGui() = default;

    void clearPasswordEntry() { mPasswordInput.set_text(""); }

    void callOnPasswordEntry(std::function<void(const std::string& password)> callback) {
        mCallbacks.onPasswordEnter = std::move(callback);
    }

    void callOnSleepButtonClick(std::function<void()> callback) {
        mCallbacks.onSleepButton = std::move(callback);
    }

private:
    Gtk::Overlay mOverlay;
    Gtk::Image mBackground;
    Gtk::Box mBoxLayout;

    Gtk::Entry mPasswordInput;
    Gtk::Button mButtonSleep;

    struct {
        std::function<void(const std::string&)> onPasswordEnter;
        std::function<void()> onSleepButton;
    } mCallbacks;

    void stretchWindowFullscreen();

    void setWindowProperties();

    void overrideWM();

    void configurePasswordEntry();

    void configureSleepButton();

    static void onTogglePasswordVisibility([[maybe_unused]] Gtk::EntryIconPosition icon_pos,
                                           [[maybe_unused]] const GdkEventButton* event,
                                           Gtk::Entry* entry);

    void onPasswordEnter();

    void onSleepButton();

    PadlockGui& operator=(const PadlockGui&) = delete;
    PadlockGui(const PadlockGui&) = delete;
};

} // namespace padlock

#endif // PADLOCK_GUI_PADLOCKWINDOW_HPP
