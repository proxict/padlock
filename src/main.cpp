#include <iostream>
#include <unistd.h> // fork

#include "padlock/Padlock.h"

static int runGuiApp(const std::string& imagePath) {
    pid_t pid = fork();
    if (pid != 0) { // exit in parent process
        return 0;
    }

    try {
        padlock::Padlock padlock(imagePath);
    } catch (const padlock::Exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 2; // This should never be executed
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: padlock <image-path>\n";
        return 1;
    }
    const std::string arg(argv[1]);
    if (arg == "-h" || arg == "--help") {
        std::cerr << "Usage: padlock <image-path>\n";
        return 0;
    }

    return runGuiApp(argv[1]);
}
