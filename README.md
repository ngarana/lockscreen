# qypr-lock

A lean memory efficient, GPU-free **C++ Wayland lock screen** for Hyprland (and any compositor
implementing `ext-session-lock-v1`). 

Software-rendered with **cairo + wl_shm** (no EGL/GL pipeline needed for this
UI), authenticated with **PAM**, with an optional **MPRIS** now-playing panel
over **sdbus-c++** and an optional **video wallpaper** via **libmpv**.

## Features

- Session lock via `ext-session-lock-v1` (covers every output).
- Clock + date, glassmorphic password field (PAM `login`), status feedback.
- Reveal-on-interaction: idle shows clock + a standby power button; any key or
  mouse movement reveals the password field, power menu, and audio panel.
- Power menu: suspend / hibernate / reboot / shutdown (via `systemctl`).
- MPRIS audio panel: metadata, progress/LIVE, transport, volume slider.
- Lock-screen notifications: a stack of glass cards (app tile,
  title, body) bottom-left that fade in and dismiss on click, fed by the real
  `org.freedesktop.Notifications` traffic of whatever daemon is running (SwayNC,
  dunst, mako, …) via a spec-correct D-Bus monitor connection. Honours
  `replaces_id`, the `urgency`/`transient` hints, and `NotificationClosed`.
- Video wallpaper: a shuffled, time-of-day playlist decoded by libmpv (falls
  back to the gradient if unavailable).
- Idle dim: after a configurable idle period the video **pauses** (drops the
  decode cost to zero) and the screen **fades to black**; any input reverses it.
  Real backlight/DPMS power-off is left to the idle daemon (hypridle).
- Compositor-throttled repaint (frame callbacks) — idle CPU ≈ one repaint/sec
  (without video).

### Video wallpaper cost

The video background is decoded by libmpv into a CPU buffer (mpv's software
render API, `vo=libmpv`) and composited with cairo. It is a **heavy** feature:
software-decoding 1080p at ~30 fps costs on the order of **200 MB+ RSS and most
of a CPU core** while locked — far above the ~34 MB, near-idle cost of the lock
screen itself. Playlists live in `playlists/` (`day.m3u` / `night.m3u`); entries
are resolved relative to the playlist file. Tuning knobs live in
`src/video/VideoPlayer.cpp` (frame interval, `hwdec`, `sw-fast`). Verify the
pipeline without locking via `qypr-lock --video-test [seconds]`.

## Build

Requires: a C++20 compiler, CMake, Ninja, `wayland-scanner`, and dev headers
for `wayland-client`, `wayland-cursor`, `xkbcommon`, `cairo`, `pangocairo`,
`sdbus-c++`, `libsystemd` (sd-bus, for the notification monitor), `libpam`,
and `mpv` (libmpv, for the video wallpaper).

```sh
./scripts/build.sh            # → ./build/qypr-lock
```

## Notifications

Real notifications are shown by observing `org.freedesktop.Notifications`
traffic on the session bus the same way `busctl monitor` does: a dedicated
sd-bus connection calls `org.freedesktop.DBus.Monitoring.BecomeMonitor` with
the match rules passed in the call (the D-Bus spec's sanctioned mechanism),
so it works with any daemon and needs no policy changes. Sample cards appear
only in `--preview`.

To also carry the **pre-lock backlog** (undismissed notifications from before
the screen locked), enable the session-long mirror service:

```sh
install -Dm644 systemd/qypr-notification-log.service \
    ~/.config/systemd/user/qypr-notification-log.service
systemctl --user daemon-reload
systemctl --user enable --now qypr-notification-log.service
```

It runs `qypr-record`, tracks dismissals (a SwayNC "clear" removes
entries from the mirror), and hands the queue to the lock screen at startup
over D-Bus (`org.qypr.Notifications`).

## Use

```sh
./lock.sh                     # lock the session (builds first if needed)
./run.sh [out.png]            # dev preview to PNG — does NOT lock the session
qypr-lock --preview out.png   # same preview, directly
qypr-lock --idle-timeout 30   # pause video + dim to black after 30s idle (default 60)
qypr-lock --video-test 6      # exercise the video pipeline offscreen, no lock
qypr-record                  # notification mirror service (see Notifications)
```

### hypridle integration

qypr-lock handles video-pause and the fade-to-black dim; the idle daemon owns
real backlight/DPMS. Point hypridle's `lock_cmd` at the binary and place its
`dpms off` timeout *after* qypr-lock's dim, e.g.:

```
general { lock_cmd = pidof qypr-lock || /path/to/qypr-lock --idle-timeout 30 }
listener { timeout = 300  on-timeout = loginctl lock-session }      # lock  @5min
listener { timeout = 345  on-timeout = hyprctl dispatch dpms off }  # panel off, after the dim
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
              StatusMessage, ActionButton, AudioController, Notification
  auth/       PamAuthenticator (PAM on a worker thread)
  power/      PowerManager (systemctl)
  mpris/      MprisController (sdbus-c++)
  notifications/ NotificationMonitor (sd-bus monitor), NotificationLog
              (qypr-record backlog service)
  video/      VideoPlayer (libmpv software render → cairo)
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
