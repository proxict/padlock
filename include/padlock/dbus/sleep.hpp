#ifndef PADLOCK_DBUS_SLEEP_HPP
#define PADLOCK_DBUS_SLEEP_HPP

#include <stdio.h>
#include <stdlib.h>
#include <systemd/sd-bus.h>

// Create group "padlock", add ${USER} to this group, create an allow policy in
// /etc/dbus-1/system/padlock.conf
namespace padlock::dbus {

inline int sleep() {
    sd_bus* bus = NULL;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    int ret;

    // Connect to the system bus
    ret = sd_bus_open_system(&bus);
    if (ret < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-ret));
        return 1;
    }

    // Call the "Suspend" method on logind's D-Bus interface
    ret = sd_bus_call_method(bus,
                             "org.freedesktop.login1",         // Service to contact
                             "/org/freedesktop/login1",        // Object path
                             "org.freedesktop.login1.Manager", // Interface
                             "Suspend",                        // Method
                             &error,                           // Error object
                             NULL,                             // No reply expected
                             "b",                              // Input parameter type (boolean)
                             1);                               // Argument (true to suspend)

    if (ret < 0) {
        fprintf(stderr, "Failed to suspend system: %s\n", error.message);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return 1;
    }

    // Clean up
    sd_bus_error_free(&error);
    sd_bus_unref(bus);

    printf("System is going to suspend...\n");
    return 0;
}

} // namespace padlock::dbus

#endif // PADLOCK_DBUS_SLEEP_HPP
