#include "padlock/gui/PadlockWindow.hpp"

// #include "padlock/Exception.hpp"

[[maybe_unused]] inline std::string&
replaceAllInplace(std::string& str, const std::string& from, const std::string& to) {
    std::size_t pos = 0;
    while (!from.empty() && (pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.size(), to);
        pos += to.size();
    }
    return str;
}

padlock::PadlockGui::PadlockGui(const std::string& imgPath)
    : mBoxLayout(Gtk::Orientation::VERTICAL) {
    setWindowProperties();
    overrideWM();

    configurePasswordEntry();
    configureSleepButton();

    signal_realize().connect([this, imgPath]() {
        auto display = get_display();
        auto surface = get_surface();
        auto primary_monitor = display->get_monitor_at_surface(surface);
        Gdk::Rectangle geometry;
        primary_monitor->get_geometry(geometry);

        // auto monitors = display->get_monitors();
        // for (guint i = 0; i < monitors->get_n_items(); ++i) {
        //     auto monitorPtr = dynamic_cast<Gdk::Monitor*>(monitors->get_object(i).get());
        //     Glib::RefPtr<Gdk::Monitor> monitor(monitorPtr);
        //     monitor->get_geometry();
        // }

        std::string style = R"css(
            .body {
                background-image: url("file://{IMG_PATH}");
                background-position: center;
                background-size: cover;
            }
        )css";
        replaceAllInplace(style, "{IMG_PATH}", imgPath);
        auto provider = Gtk::CssProvider::create();
        provider->load_from_string(style);
        auto ctx = get_style_context();
        ctx->add_class("body");
        ctx->add_provider(provider, GTK_STYLE_PROVIDER_PRIORITY_USER);

        set_default_size(geometry.get_width(), geometry.get_height());

        mBoxLayout.set_halign(Gtk::Align::CENTER);
        mBoxLayout.set_valign(Gtk::Align::CENTER);
        mBoxLayout.set_spacing(10);
        mBoxLayout.append(mPasswordInput);

        mOverlay.add_overlay(mBoxLayout);
        mOverlay.add_overlay(mButtonSleep);

        set_child(mOverlay);
        set_visible();
    });
}

void padlock::PadlockGui::setWindowProperties() {
    unset_transient_for();
    set_decorated(false);
    set_resizable(false);
    set_modal(true);
    set_focusable(true);
    fullscreen();
}

void padlock::PadlockGui::overrideWM() {
    // TODO: GTK4 no longer handles seat grabs

    // get_window()->set_override_redirect(true);
    // const auto grabSuccess =
    //     get_display()->get_default_seat()->grab(get_window(), Gdk::SEAT_CAPABILITY_ALL, true);
    // if (grabSuccess != Gdk::GRAB_SUCCESS) {
    //     throw padlock::Exception("Device grab failed with an error (" + std::to_string(grabSuccess) + ')');
    // }
}

void padlock::PadlockGui::configurePasswordEntry() {
    mPasswordInput.set_visibility(false);
    mPasswordInput.set_input_purpose(Gtk::InputPurpose::PASSWORD);
    mPasswordInput.set_icon_from_icon_name("view-reveal-symbolic", Gtk::Entry::IconPosition::SECONDARY);
    mPasswordInput.set_icon_activatable(true, Gtk::Entry::IconPosition::SECONDARY);
    mPasswordInput.signal_icon_press().connect(
        std::bind(&PadlockGui::onTogglePasswordVisibility, &mPasswordInput));

    mPasswordInput.signal_activate().connect(std::bind(&PadlockGui::onPasswordEnter, this));
    mPasswordInput.set_placeholder_text("Enter password");
    mPasswordInput.grab_focus();
}

void padlock::PadlockGui::configureSleepButton() {
    mButtonSleep.set_label("Sleep");
    mButtonSleep.signal_clicked().connect(std::bind(&PadlockGui::onSleepButton, this));

    mButtonSleep.set_halign(Gtk::Align::END);
    mButtonSleep.set_valign(Gtk::Align::END);
    mButtonSleep.set_margin_end(10);
    mButtonSleep.set_margin_bottom(10);
}

void padlock::PadlockGui::onTogglePasswordVisibility(Gtk::Entry* entry) {
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
