#include "padlock/gui/PadlockWindow.hpp"

// #include "padlock/Exception.hpp"
#include <iostream>

[[maybe_unused]] inline std::string&
replaceAllInplace(std::string& str, const std::string& from, const std::string& to) {
    std::size_t pos = 0;
    while (!from.empty() && (pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.size(), to);
        pos += to.size();
    }
    return str;
}

static void set_override_redirect(GdkSurface* surface, bool enable) {
    Display* display = GDK_SURFACE_XDISPLAY(surface);
    Window x11_window = GDK_SURFACE_XID(surface);
    XSetWindowAttributes attrs;
    attrs.override_redirect = enable ? True : False;
    XChangeWindowAttributes(display, x11_window, CWOverrideRedirect, &attrs);
    XFlush(display);
}

static bool grab_pointer(GdkSurface* surface) {
    Display* display = GDK_SURFACE_XDISPLAY(surface);
    Window x11_window = GDK_SURFACE_XID(surface);

    const int result = XGrabPointer(display,
                                    x11_window,
                                    True,
                                    ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                                    GrabModeAsync,
                                    GrabModeAsync,
                                    None,
                                    XCreateFontCursor(display, XC_arrow),
                                    CurrentTime);

    if (result != GrabSuccess) {
        std::cout << "Failed to grab pointer: " << result << std::endl;
    }

    return result == GrabSuccess;
}

static bool grab_keyboard(GdkSurface* surface) {
    Display* display = GDK_SURFACE_XDISPLAY(surface);
    Window x11_window = GDK_SURFACE_XID(surface);
    const int result = XGrabKeyboard(display, x11_window, True, GrabModeAsync, GrabModeAsync, CurrentTime);
    if (result != GrabSuccess) {
        std::cout << "Failed to grab keyboard: " << result << std::endl;
    }
    return result == GrabSuccess;
}

static void ungrab_all(Display* display) {
    XUngrabPointer(display, CurrentTime);
    XUngrabKeyboard(display, CurrentTime);
    XFlush(display);
}

static void forward_key_event(Display* display, XKeyEvent& xkey, bool is_pressed) {
    // Modify the event to re-inject it
    xkey.type = is_pressed ? KeyPress : KeyRelease;

    // Re-inject the event into the X11 event queue
    XSendEvent(display, xkey.window, True, KeyPressMask | KeyReleaseMask, reinterpret_cast<XEvent*>(&xkey));
    XFlush(display);
}

static void process_x11_events(Display* display) {
    XEvent event;

    while (true) {
        XNextEvent(display, &event);

        switch (event.type) {
        case KeyPress:
        case KeyRelease:
            forward_key_event(display, event.xkey, event.type == KeyPress);
            break;

        default:
            break;
        }
    }
}

padlock::PadlockGui::PadlockGui(const std::string& imgPath)
    : mBoxLayout(Gtk::Orientation::VERTICAL) {
    setWindowProperties();

    configurePasswordEntry();
    configureSleepButton();

    signal_realize().connect([this, imgPath]() {
        auto display = get_display();
        auto surface = get_surface();
        auto primary_monitor = display->get_monitor_at_surface(surface);
        Gdk::Rectangle geometry;
        primary_monitor->get_geometry(geometry);
        set_default_size(geometry.get_width(), geometry.get_height());

        // XSizeHints hints;
        // hints.flags = PAllHints;
        // hints.x = 0;
        // hints.y = 0;
        // hints.width = geometry.get_width();
        // hints.height = geometry.get_height();
        // XSetNormalHints(gdk_x11_display_get_xdisplay(get_native()->get_surface()->get_display()->gobj()),
        //                 GDK_SURFACE_XID(get_native()->get_surface()->gobj()),
        //                 &hints);

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
        mOverlay.add_css_class("body");
        auto provider = Gtk::CssProvider::create();
        provider->load_from_string(style);
        Gtk::StyleProvider::add_provider_for_display(display, provider, GTK_STYLE_PROVIDER_PRIORITY_USER);

        mBoxLayout.set_halign(Gtk::Align::CENTER);
        mBoxLayout.set_valign(Gtk::Align::CENTER);
        mBoxLayout.set_spacing(10);
        mBoxLayout.append(mPasswordInput);

        mOverlay.add_overlay(mBoxLayout);
        mOverlay.add_overlay(mButtonSleep);

        set_child(mOverlay);
        set_visible();
        overrideWM();

        // https://discourse.gnome.org/t/gtk-4-grab-keyboard/6251
        // auto toplevels = get_toplevels();
        // for (guint i = 0; i < toplevels->get_n_items(); ++i) {
        //     if (auto toplevel = dynamic_cast<Gtk::Window*>(toplevels->get_object(i).get())) {
        //         if (auto gdktoplevel = dynamic_cast<Gdk::Toplevel*>(toplevel->get_surface().get())) {
        //             gdktoplevel->inhibit_system_shortcuts(nullptr);
        //             std::cout << "Found valid Gdk::Toplevel" << std::endl;
        //         }
        //     }
        // }

        // process_x11_events(GDK_SURFACE_XDISPLAY(get_native()->get_surface()->gobj()));
    });

    // https://gitlab.gnome.org/GNOME/gtk/-/blob/e024a542b0e669cc9088edd88a098abc7e1fe5aa/gdk/wayland/gdkdevice-wayland.c#L4882
    signal_map().connect([this]() {
        // gdk_seat_grab(default_seat, window, GDK_SEAT_CAPABILITY_KEYBOARD);
        auto seat = get_display()->get_default_seat();
        seat->get_type();

        GdkSurface* nativeSurface = get_native()->get_surface()->gobj();
        if (grab_pointer(nativeSurface)) {
            std::cout << "Grabbed pointer" << std::endl;
        } else {
            std::cout << "Failed to grab pointer" << std::endl;
        }
        if (grab_keyboard(nativeSurface)) {
            std::cout << "Grabbed keyboard" << std::endl;
        } else {
            std::cout << "Failed to grab keyboard" << std::endl;
        }
    });

    signal_unmap().connect([this]() {
        GdkSurface* surface = get_native()->get_surface()->gobj();
        ungrab_all(GDK_SURFACE_XDISPLAY(surface));
        std::cout << "Ungrabbed all" << std::endl;
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

    GdkSurface* surface = get_native()->get_surface()->gobj();
    set_override_redirect(surface, true);

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
    close();
    // if (mCallbacks.onSleepButton) {
    //     mCallbacks.onSleepButton();
    // }
}
