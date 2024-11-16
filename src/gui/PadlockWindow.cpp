#include "padlock/gui/PadlockWindow.hpp"

#include "padlock/Exception.hpp"

padlock::PadlockGui::PadlockGui(const std::string& imgPath)
    : Gtk::Window(Gtk::WINDOW_POPUP)
    , mBoxLayout(Gtk::ORIENTATION_VERTICAL) {
    setWindowProperties();
    show();
    overrideWM();

    configurePasswordEntry();
    configureSleepButton();

    const int width = get_allocated_width();
    const int height = get_allocated_height();
    mBackground.set(
        Gdk::Pixbuf::create_from_file(imgPath)->scale_simple(width, height, Gdk::INTERP_BILINEAR));
    mBackground.set_can_default(false);
    mOverlay.add(mBackground);

    mBoxLayout.set_halign(Gtk::ALIGN_CENTER);
    mBoxLayout.set_valign(Gtk::ALIGN_CENTER);
    mBoxLayout.set_spacing(10);
    mBoxLayout.add(mPasswordInput);

    mOverlay.add_overlay(mBoxLayout);
    mOverlay.add_overlay(mButtonSleep);

    add(mOverlay);
    show_all_children();
}

void padlock::PadlockGui::stretchWindowFullscreen() {
    // TODO: display image on all monitors, controls only on primary
    Gdk::Rectangle rect;
    get_display()->get_primary_monitor()->get_workarea(rect);
    set_default_size(rect.get_width(), rect.get_height());
}

void padlock::PadlockGui::setWindowProperties() {
    set_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK | Gdk::TOUCH_MASK | Gdk::BUTTON_PRESS_MASK |
               Gdk::POINTER_MOTION_MASK);
    set_border_width(0);
    set_accept_focus(true);
    set_modal(true);
    set_resizable(false);

    stretchWindowFullscreen();
}

void padlock::PadlockGui::overrideWM() {
    get_window()->set_override_redirect(true);

    const auto grabSuccess =
        get_display()->get_default_seat()->grab(get_window(), Gdk::SEAT_CAPABILITY_ALL, true);
    if (grabSuccess != Gdk::GRAB_SUCCESS) {
        throw padlock::Exception("Device grab failed with an error (" + std::to_string(grabSuccess) + ')');
    }
}

void padlock::PadlockGui::configurePasswordEntry() {
    mPasswordInput.set_visibility(false);
    mPasswordInput.set_icon_from_icon_name("view-reveal-symbolic", Gtk::ENTRY_ICON_SECONDARY);
    mPasswordInput.set_icon_activatable(true, Gtk::ENTRY_ICON_SECONDARY);
    mPasswordInput.signal_icon_press().connect(
        sigc::bind(sigc::ptr_fun(&PadlockGui::onTogglePasswordVisibility), &mPasswordInput));

    mPasswordInput.signal_activate().connect(std::bind(&PadlockGui::onPasswordEnter, this));
    mPasswordInput.set_placeholder_text("Enter password");
    mPasswordInput.set_can_default(true);
    mPasswordInput.grab_focus();
}

void padlock::PadlockGui::configureSleepButton() {
    mButtonSleep.add_label("Sleep");
    mButtonSleep.signal_clicked().connect(std::bind(&PadlockGui::onSleepButton, this));
    mButtonSleep.set_can_default(false);

    mButtonSleep.set_halign(Gtk::ALIGN_END);
    mButtonSleep.set_valign(Gtk::ALIGN_END);
    mButtonSleep.set_margin_end(10);
    mButtonSleep.set_margin_bottom(10);
}

void padlock::PadlockGui::onTogglePasswordVisibility([[maybe_unused]] Gtk::EntryIconPosition icon_pos,
                                                     [[maybe_unused]] const GdkEventButton* event,
                                                     Gtk::Entry* entry) {
    entry->set_visibility(!entry->get_visibility());
}

void padlock::PadlockGui::onPasswordEnter() {
    if (mCallbacks.onPasswordEnter) {
        mCallbacks.onPasswordEnter(mPasswordInput.get_text());
    }
}

void padlock::PadlockGui::onSleepButton() {
    if (mCallbacks.onSleepButton) {
        mCallbacks.onSleepButton();
    }
}
