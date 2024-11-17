cmake_minimum_required(VERSION 3.10)
project(Padlock VERSION 1.0.0 LANGUAGES CXX)

include(cmake/asan.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(padlock
    src/main.cpp
    src/pam.cpp
    src/Padlock.cpp
    src/AuthUser.cpp
    src/gui/PadlockWindow.cpp
)

target_include_directories(padlock
    PRIVATE include
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM4 REQUIRED gtkmm-4.0)

include_directories(${GTKMM4_INCLUDE_DIRS})
link_directories(${GTKMM4_LIBRARY_DIRS})
add_definitions(${GTKMM4_CFLAGS_OTHER})

target_link_libraries(padlock
    PRIVATE pam
    PRIVATE ${GTKMM4_LIBRARIES}
    PRIVATE X11
)

target_compile_options(padlock PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Woverloaded-virtual
    -Wnull-dereference
    -Wformat=2
    -Wsign-conversion
)

install(TARGETS padlock RUNTIME DESTINATION bin)
