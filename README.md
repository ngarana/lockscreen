# qypr-lock

A lean, GPU-free **C++ Wayland lock screen** for Hyprland (and any compositor
implementing `ext-session-lock-v1`). Migrated from a Quickshell/QML shell to
native C++ to cut memory use and remove the QML/QtQuick runtime.

Software-rendered with **cairo + wl_shm** (no EGL/GL pipeline needed for this
UI), authenticated with **PAM**, with an optional **MPRIS** now-playing panel
over **sdbus-c++**.

## Features

- Session lock via `ext-session-lock-v1` (covers every output).
- Clock + date, glassmorphic password field (PAM `login`), status feedback.
- Reveal-on-interaction: idle shows clock + a standby power button; any key or
  mouse movement reveals the password field, power menu, and audio panel.
- Power menu: suspend / hibernate / reboot / shutdown (via `systemctl`).
- MPRIS audio panel: metadata, progress/LIVE, transport, volume slider.
- Compositor-throttled repaint (frame callbacks) — idle CPU ≈ one repaint/sec.

## Build

Requires: a C++20 compiler, CMake, Ninja, `wayland-scanner`, and dev headers
for `wayland-client`, `xkbcommon`, `cairo`, `pangocairo`, `sdbus-c++`, `libpam`.

```sh
./scripts/build.sh            # → ./build/qypr-lock
```

## Use

```sh
./lock.sh                     # lock the session (builds first if needed)
./run.sh [out.png]            # dev preview to PNG — does NOT lock the session
qypr-lock --preview out.png   # same preview, directly
```

To bind it in Hyprland, point your idle/lock action at the binary, e.g.:

```
bind = $mod, L, exec, /home/arch/.config/qypr/build/qypr-lock
```

## Architecture

One event loop (`epoll`); everything else plugs into it. Layers depend on the
`core/Interfaces.hpp` boundaries, never on each other's internals.

```
src/
  core/       EventLoop, App, Types (geometry/colour/easing), Interfaces
  wayland/    WaylandDisplay, LockSession, Output, Seat, ShmBuffer
  render/     Painter (cairo + pango helpers)
  ui/         Theme, Widget, LockScreen, Clock, PasswordField,
              StatusMessage, ActionButton, AudioController
  auth/       PamAuthenticator (PAM on a worker thread)
  power/      PowerManager (systemctl)
  mpris/      MprisController (sdbus-c++)
```

- **KISS** — software cairo rendering instead of a bespoke GL/shader stack;
  synchronous MPRIS polling instead of signal plumbing.
- **DRY** — one `Theme`, one `Painter`, one `EventLoop`; widgets share a base.
- **SOLID** — the Wayland layer talks to the UI only through `InputSink` /
  `RenderHost`; PAM, power, and MPRIS are self-contained services.

## Security notes

- The password is copied straight into the PAM worker and cleared right after;
  the UI only ever holds the in-progress input.
- Authentication runs off the UI thread so a slow PAM stack can't freeze the
  lock surface.
