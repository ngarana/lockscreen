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
pkg_check_modules(MPV      REQUIRED IMPORTED_TARGET mpv)

find_library(PAM_LIBRARY pam REQUIRED)
find_path(PAM_INCLUDE_DIR security/pam_appl.h REQUIRED)

# -----------------------------------------------------------------------------
# Wayland protocol code generation (ext-session-lock-v1)
# -----------------------------------------------------------------------------
find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)
pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)

set(GEN_DIR "${CMAKE_BINARY_DIR}/wayland-generated")
file(MAKE_DIRECTORY "${GEN_DIR}")

set(SESSION_LOCK_XML
    "${WAYLAND_PROTOCOLS_DIR}/staging/ext-session-lock/ext-session-lock-v1.xml")

set(PROTO_HEADER "${GEN_DIR}/ext-session-lock-v1-client-protocol.h")
set(PROTO_CODE   "${GEN_DIR}/ext-session-lock-v1-protocol.c")

add_custom_command(
    OUTPUT "${PROTO_HEADER}"
    COMMAND "${WAYLAND_SCANNER}" client-header "${SESSION_LOCK_XML}" "${PROTO_HEADER}"
    DEPENDS "${SESSION_LOCK_XML}"
    VERBATIM)

add_custom_command(
    OUTPUT "${PROTO_CODE}"
    COMMAND "${WAYLAND_SCANNER}" private-code "${SESSION_LOCK_XML}" "${PROTO_CODE}"
    DEPENDS "${SESSION_LOCK_XML}"
    VERBATIM)

# -----------------------------------------------------------------------------
# Target
# -----------------------------------------------------------------------------
file(GLOB_RECURSE QYPR_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(qypr-lock
    ${QYPR_SOURCES}
    "${PROTO_CODE}"
    "${PROTO_HEADER}")

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
    PkgConfig::MPV
    "${PAM_LIBRARY}"
    Threads::Threads)

install(TARGETS qypr-lock RUNTIME DESTINATION bin)
