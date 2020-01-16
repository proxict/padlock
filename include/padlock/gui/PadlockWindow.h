#ifndef PADLOCK_GUI_PADLOCKWINDOW_H_
#define PADLOCK_GUI_PADLOCKWINDOW_H_

#include "padlock/Exception.h"
#include "padlock/gui/EntryWidget.h"

#include <gdkmm/display.h>
#include <gdkmm/screen.h>
#include <gtkmm/stylecontext.h>
#include <gtkmm/window.h>

namespace padlock {

class PadlockGui final : public Gtk::Window {
public:
    PadlockGui(const std::string& imgPath) {
        mBgImage.set(imgPath);
        init();
    }

    PadlockGui(const std::string& imgPath, const Gtk::WindowType winType)
        : Gtk::Window(winType) {
        mBgImage.set(imgPath);
        init();
    }

    virtual ~PadlockGui() = default;

    void clearPasswordEntry() { mPasswordInput.clear(); }

    void callOnPasswordEntry(std::function<void(const std::string& password)>&& callback) {
        mCallbacks.onPasswordEnter = callback;
    }

    void callOnSleepButtonClick(std::function<void()>&& callback) { mCallbacks.onSleepButton = callback; }

private:
    Gtk::Fixed mFidexLayout;
    Gtk::Button mButtonSleep;
    Gtk::Image mBgImage;
    Entry mPasswordInput;

    struct {
        std::function<void(const std::string&)> onPasswordEnter;
        std::function<void()> onSleepButton;
    } mCallbacks;

    void onSleepButton() { mCallbacks.onSleepButton(); }

    void stretchWindowFullscreen() {
        const auto rec = get_screen()->get_monitor_workarea(get_screen()->get_primary_monitor());
        set_default_size(rec.get_width(), rec.get_height());
    }

    void setWindowProperties() {
        set_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK | Gdk::TOUCH_MASK | Gdk::BUTTON_PRESS_MASK |
                   Gdk::POINTER_MOTION_MASK);
        set_border_width(0);
        set_accept_focus(true);
        set_modal(true);
        set_resizable(false);

        stretchWindowFullscreen();
    }

    void overrideWM() {
        get_window()->set_override_redirect(true);

        const auto grabSuccess =
            get_display()->get_default_seat()->grab(get_window(), Gdk::SEAT_CAPABILITY_ALL, true);
        if (grabSuccess != Gdk::GRAB_SUCCESS) {
            throw Exception("Device grab failed with an error (" + std::to_string(grabSuccess) + ')');
        }
    }

    void setSleepButton() {
        mButtonSleep.add_label("Sleep");
        mButtonSleep.set_size_request(50, 50);
        mButtonSleep.signal_clicked().connect(sigc::mem_fun(*this, &PadlockGui::onSleepButton));
        mButtonSleep.set_can_default(false);
    }

    void init() {
        setWindowProperties();
        show();
        overrideWM();

        setSleepButton();

        mPasswordInput.callOnReturn([this](Entry* entry) {
            if (mCallbacks.onPasswordEnter) {
                mCallbacks.onPasswordEnter(entry->getText());
            }
            return false;
        });

        int sleepButtonSizeX, sleepButtonSizeY;
        mButtonSleep.get_size_request(sleepButtonSizeX, sleepButtonSizeY);

        int entrySizeX, entrySizeY;
        mPasswordInput.get_size_request(entrySizeX, entrySizeY);

        mPasswordInput.setIsPassword(true);
        mPasswordInput.setCueBanner("Enter Password");

        mFidexLayout.put(mBgImage, 0, 0);
        mFidexLayout.put(
            mButtonSleep, get_width() - sleepButtonSizeX - 5, get_height() - sleepButtonSizeY - 5);
        mFidexLayout.put(mPasswordInput, get_width() / 2 - entrySizeX / 2, get_height() / 2 - entrySizeY / 2);

        add(mFidexLayout);
        show_all_children();
    }

    PadlockGui& operator=(const PadlockGui&) = delete;
    PadlockGui(const PadlockGui&) = delete;
};

} // namespace padlock

#endif // PADLOCK_GUI_PADLOCKWINDOW_H_
