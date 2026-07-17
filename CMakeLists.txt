cmake_minimum_required(VERSION 3.20)
project(qypr LANGUAGES C CXX)

# -----------------------------------------------------------------------------
# Standards & defaults
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Coverage is a test-only opt-in (configure with -DENABLE_COVERAGE=ON). It is
# scoped to the qypr-test target below, so the shipped qypr-lock / qypr-record
# binaries are never instrumented (no -O0, no .gcda writes at runtime).
option(ENABLE_COVERAGE "Instrument the qypr-test target for coverage" OFF)

set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers)

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET wayland-client)
pkg_check_modules(WLCURSOR REQUIRED IMPORTED_TARGET wayland-cursor)
pkg_check_modules(XKB      REQUIRED IMPORTED_TARGET xkbcommon)
pkg_check_modules(CAIRO    REQUIRED IMPORTED_TARGET cairo)
pkg_check_modules(PANGO    REQUIRED IMPORTED_TARGET pangocairo)
pkg_check_modules(SDBUS    REQUIRED IMPORTED_TARGET sdbus-c++)
pkg_check_modules(SYSTEMD  REQUIRED IMPORTED_TARGET libsystemd)  # sd-bus (notification monitor)
pkg_check_modules(UDEV     REQUIRED IMPORTED_TARGET libudev)     # backlight change events
pkg_check_modules(PULSE    REQUIRED IMPORTED_TARGET libpulse)    # volume (pipewire-pulse)
pkg_check_modules(MPV      REQUIRED IMPORTED_TARGET mpv)
pkg_check_modules(RSVG     REQUIRED IMPORTED_TARGET librsvg-2.0)  # SVG icon rasterization

find_library(PAM_LIBRARY pam REQUIRED)
find_path(PAM_INCLUDE_DIR security/pam_appl.h REQUIRED)

# -----------------------------------------------------------------------------
# Wayland protocol code generation (ext-session-lock-v1)
# -----------------------------------------------------------------------------
find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)
pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)

set(GEN_DIR "${CMAKE_BINARY_DIR}/wayland-generated")
file(MAKE_DIRECTORY "${GEN_DIR}")

# Generate client-header + private-code for a protocol XML, appending the two
# outputs to PROTO_SOURCES. <name> is the file stem (without -v1.xml).
set(PROTO_SOURCES "")
function(qypr_gen_protocol name xml)
    set(hdr "${GEN_DIR}/${name}-client-protocol.h")
    set(src "${GEN_DIR}/${name}-protocol.c")
    # wlr-layer-shell's get_layer_surface has an argument literally named
    # `namespace` — a C++ keyword that breaks the generated header when included
    # from C++. Rename the identifier (whole-word; a no-op for other protocols).
    add_custom_command(OUTPUT "${hdr}"
        COMMAND "${WAYLAND_SCANNER}" client-header "${xml}" "${hdr}"
        COMMAND sed -i "s/\\bnamespace\\b/wl_namespace/g" "${hdr}"
        DEPENDS "${xml}" VERBATIM)
    add_custom_command(OUTPUT "${src}"
        COMMAND "${WAYLAND_SCANNER}" private-code "${xml}" "${src}"
        DEPENDS "${xml}" VERBATIM)
    set(PROTO_SOURCES ${PROTO_SOURCES} "${hdr}" "${src}" PARENT_SCOPE)
endfunction()

qypr_gen_protocol(ext-session-lock-v1
    "${WAYLAND_PROTOCOLS_DIR}/staging/ext-session-lock/ext-session-lock-v1.xml")
# General-purpose bar widgets — compositor-agnostic protocols only:
#   workspaces  → ext-workspace-v1 (staging, standard)
#   active window / taskbar → wlr-foreign-toplevel-management (vendored; the only
#     widely-supported protocol carrying per-window *focus* state — ext's
#     foreign-toplevel-list is list-only).
qypr_gen_protocol(ext-workspace-v1
    "${WAYLAND_PROTOCOLS_DIR}/staging/ext-workspace/ext-workspace-v1.xml")
qypr_gen_protocol(wlr-foreign-toplevel-management-unstable-v1
    "${CMAKE_SOURCE_DIR}/protocols/wlr-foreign-toplevel-management-unstable-v1.xml")
# Standalone qypr-bar host surface:
#   wlr-layer-shell (vendored) anchors the bar as a desktop panel with an
#   exclusive zone. xdg-shell is generated only to satisfy the xdg_popup symbol
#   that layer-shell's get_popup request references — qypr-bar never uses popups.
qypr_gen_protocol(xdg-shell
    "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml")
qypr_gen_protocol(wlr-layer-shell-unstable-v1
    "${CMAKE_SOURCE_DIR}/protocols/wlr-layer-shell-unstable-v1.xml")

# -----------------------------------------------------------------------------
# Targets
# -----------------------------------------------------------------------------
file(GLOB_RECURSE QYPR_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
# Entry points carry their own main() and are added per-target below, so no
# executable ever links two of them; NotificationLog is recorder-only.
list(REMOVE_ITEM QYPR_SOURCES
    "${CMAKE_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/bar_main.cpp"
    "${CMAKE_SOURCE_DIR}/src/recorder_main.cpp"
    "${CMAKE_SOURCE_DIR}/src/notifications/NotificationLog.cpp"
)

# 1. Main Lock Screen target (links all graphics, mpv, PAM, sdbus-c++, etc.)
add_executable(qypr-lock
    ${QYPR_SOURCES}
    src/main.cpp
    ${PROTO_SOURCES})

target_include_directories(qypr-lock PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${GEN_DIR}"
    "${PAM_INCLUDE_DIR}")

target_link_libraries(qypr-lock PRIVATE
    PkgConfig::WAYLAND
    PkgConfig::WLCURSOR
    PkgConfig::XKB
    PkgConfig::CAIRO
    PkgConfig::PANGO
    PkgConfig::SDBUS
    PkgConfig::SYSTEMD
    PkgConfig::UDEV
    PkgConfig::PULSE
    PkgConfig::MPV
    PkgConfig::RSVG
    "${PAM_LIBRARY}"
    Threads::Threads)

# 1b. Standalone status-bar target (qypr-bar): the unlocked-desktop bar on
# wlr-layer-shell. Shares the whole object set with qypr-lock (only the entry
# point differs), so it links the same libraries even though the bar itself
# never drives video/PAM/lock.
add_executable(qypr-bar
    ${QYPR_SOURCES}
    src/bar_main.cpp
    ${PROTO_SOURCES})

target_include_directories(qypr-bar PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${GEN_DIR}"
    "${PAM_INCLUDE_DIR}")

target_link_libraries(qypr-bar PRIVATE
    PkgConfig::WAYLAND
    PkgConfig::WLCURSOR
    PkgConfig::XKB
    PkgConfig::CAIRO
    PkgConfig::PANGO
    PkgConfig::SDBUS
    PkgConfig::SYSTEMD
    PkgConfig::UDEV
    PkgConfig::PULSE
    PkgConfig::MPV
    PkgConfig::RSVG
    "${PAM_LIBRARY}"
    Threads::Threads)

# 2. Standalone Recorder target (only needs systemd for sd-bus)
add_executable(qypr-record
    src/core/Config.cpp          # NotificationMonitor resolves its XDG config dir
    src/core/EventLoop.cpp
    src/notifications/NotificationMonitor.cpp
    src/notifications/NotificationLog.cpp
    src/recorder_main.cpp
)

target_include_directories(qypr-record PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(qypr-record PRIVATE
    PkgConfig::SYSTEMD
)

# 3. Unit Test suite target (mock/headless environment)
add_executable(qypr-test
    tests/unit_tests.cpp
    tests/mocks.cpp
    ${QYPR_SOURCES}
    src/main.cpp
    src/notifications/NotificationLog.cpp
    src/recorder_main.cpp
    ${PROTO_SOURCES})

target_compile_definitions(qypr-test PRIVATE TESTING)

if(ENABLE_COVERAGE)
    target_compile_options(qypr-test PRIVATE --coverage -O0 -g)
    target_link_options(qypr-test PRIVATE --coverage)
endif()

target_include_directories(qypr-test PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${GEN_DIR}"
    "${PAM_INCLUDE_DIR}")

target_link_libraries(qypr-test PRIVATE
    PkgConfig::WAYLAND
    PkgConfig::WLCURSOR
    PkgConfig::XKB
    PkgConfig::CAIRO
    PkgConfig::PANGO
    PkgConfig::SYSTEMD
    PkgConfig::UDEV
    PkgConfig::PULSE
    PkgConfig::MPV
    PkgConfig::RSVG
    Threads::Threads)

# qypr-test is a headless, mock-linked binary — build it, but never install it.
install(TARGETS qypr-lock qypr-bar qypr-record RUNTIME DESTINATION bin)

# Man pages.
install(FILES man/man1/qypr-lock.1 man/man1/qypr-bar.1 man/man1/qypr-record.1
        DESTINATION share/man/man1)

# Example configuration (copy to $XDG_CONFIG_HOME/qypr/bar.conf to use).
install(FILES examples/bar.conf DESTINATION share/qypr/examples)
