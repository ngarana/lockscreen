# qypr

A lean, memory-efficient, GPU-free **C++ Wayland desktop suite** for Hyprland
(and any compositor implementing `ext-session-lock-v1`). Ships three binaries:

| Binary | Purpose |
|--------|---------|
| `qypr-lock` | Session lock screen |
| `qypr-bar` | Desktop status bar (waybar replacement) |
| `qypr-record` | Notification mirror / backlog daemon |

Software-rendered with **cairo + wl_shm** (no EGL/GL pipeline needed),
authenticated with **PAM**, with **MPRIS** audio control, an **SNI tray**,
and an optional **video wallpaper** via **libmpv**.

## Features

### Lock screen (`qypr-lock`)

- Session lock via `ext-session-lock-v1` (covers every output).
- Clock + date, glassmorphic password field (PAM `login`), status feedback.
- Reveal-on-interaction: idle shows clock + a standby power button; any key or
  mouse movement reveals the password field, power menu, and audio panel.
- Power menu: suspend / hibernate / reboot / shutdown (via `systemctl`).
- MPRIS audio panel: metadata, progress/LIVE, transport, volume slider.
- Lock-screen notifications: glass cards (app tile, title, body) bottom-left
  that fade in and dismiss on click, fed by the real
  `org.freedesktop.Notifications` traffic of whatever daemon is running (SwayNC,
  dunst, mako, ...) via a spec-correct D-Bus monitor connection.
- Video wallpaper: a shuffled, time-of-day playlist decoded by libmpv (falls
  back to the gradient if unavailable).
- Idle dim: after a configurable idle period the video **pauses** (drops the
  decode cost to zero) and the screen **fades to black**; any input reverses it.
  Real backlight/DPMS power-off is left to the idle daemon (hypridle).
- Compositor-throttled repaint (frame callbacks) -- idle CPU = one repaint/sec
  (without video).

### Status bar (`qypr-bar`)

A waybar replacement that shares the lock screen's StatusBar and indicators,
hosted on wlr-layer-shell for the unlocked desktop.

- **Modular indicators**: battery, bluetooth, brightness, clock, DND, media,
  night-light, notifications, SNI tray, volume, wifi, workspaces, active window.
- **Quick Settings panel** (gear icon): brightness slider, volume slider, DND
  toggle, power menu.
- Configurable via `$XDG_CONFIG_HOME/qypr/bar.conf` (position, height, margin,
  modules, clock format, style).
- Session-sensitive widgets: workspaces + active window appear only on the bar
  (hidden on the lock screen).

### Notification mirror (`qypr-record`)

Lightweight daemon that tracks notification traffic for the pre-lock backlog.
Runs without heavy graphical dependencies (no Wayland, Cairo, PAM, mpv).

- Mirrors `org.freedesktop.Notifications` traffic on the session bus.
- Honours `replaces_id`, the `urgency`/`transient` hints, and
  `NotificationClosed`.
- Hands the queue to the lock screen at startup over D-Bus
  (`org.qypr.Notifications`).

### Video wallpaper cost

The video background is decoded by libmpv into a CPU buffer (mpv's software
render API, `vo=libmpv`) and composited with cairo. It is a **heavy** feature:
software-decoding 1080p at ~30 fps costs on the order of **200 MB+ RSS and most
of a CPU core** while locked -- far above the ~34 MB, near-idle cost of the lock
screen itself. Playlists live in `playlists/` (`day.m3u` / `night.m3u`); entries
are resolved relative to the playlist file. Tuning knobs live in
`src/video/VideoPlayer.cpp` (frame interval, `hwdec`, `sw-fast`). Verify the
pipeline without locking via `qypr-lock --video-test [seconds]`.

## Build

Requires: a C++20 compiler, CMake >= 3.20, Ninja, `wayland-scanner`, and dev
headers for `wayland-client`, `wayland-cursor`, `xkbcommon`, `cairo`,
`pangocairo`, `sdbus-c++`, `libsystemd` (sd-bus), `libudev`, `libpulse`,
`librsvg-2.0`, `libpam`, and `mpv` (libmpv).

```sh
./scripts/build.sh            # release build into ./build
```

The build produces three binaries in `build/`:

```
build/qypr-lock    # lock screen
build/qypr-bar     # status bar
build/qypr-record  # notification mirror
```

### Versioning

Semantic versioning (MAJOR.MINOR.PATCH) is set in `CMakeLists.txt`:

```sh
cmake -B build -DCMAKE_PROJECT_VERSION_EXTRA="-rc1" ..   # pre-release tag
./build/qypr-lock --version                               # → qypr 0.1.0-rc1
```

### Test suite

```sh
./scripts/test.sh                 # run unit tests
./scripts/coverage_report.py      # generate coverage report
```

## Use

```sh
./lock.sh                         # lock the session (builds first if needed)
./run.sh [out.png]                # dev preview to PNG -- does NOT lock
qypr-lock --preview out.png       # same preview, directly
qypr-lock --idle-timeout 30       # pause video + dim after 30s idle (default 60)
qypr-lock --video-test 6          # exercise video pipeline offscreen
qypr-lock --version               # print version
qypr-bar                          # start the status bar
qypr-bar --version                # print version
qypr-record                       # notification mirror service (see below)
qypr-record --version             # print version
```

### Notifications

To carry the **pre-lock backlog** (undismissed notifications from before the
screen locked), enable the session-long mirror service:

```sh
install -Dm644 systemd/qypr-notification-log.service \
    ~/.config/systemd/user/qypr-notification-log.service
systemctl --user daemon-reload
systemctl --user enable --now qypr-notification-log.service
```

### Status bar configuration

Copy the example config and customise:

```sh
mkdir -p ~/.config/qypr
cp examples/bar.conf ~/.config/qypr/bar.conf
$EDITOR ~/.config/qypr/bar.conf
```

### hypridle integration

qypr-lock handles video-pause and the fade-to-black dim; the idle daemon owns
real backlight/DPMS. Point hypridle's `lock_cmd` at the binary:

```
general { lock_cmd = pidof qypr-lock || /path/to/qypr-lock --idle-timeout 30 }
listener { timeout = 300  on-timeout = loginctl lock-session }      # lock  @5min
listener { timeout = 345  on-timeout = hyprctl dispatch dpms off }  # panel off
```

To bind it in Hyprland:

```
bind = $mod, L, exec, /home/arch/.config/qypr/build/qypr-lock
bind = $mod, B, exec, /home/arch/.config/qypr/build/qypr-bar
```

## Architecture

One event loop (`epoll`); everything else plugs into it. Layers depend on the
`core/Interfaces.hpp` boundaries, never on each other's internals.

```
src/
  core/       EventLoop, App, BarApp, Config, Types (geometry/colour/easing),
              Interfaces
  wayland/    WaylandDisplay, LockSession, BarDisplay, BarWindow, Output,
              Seat, ShmBuffer, Cursor
  render/     Painter (cairo + pango helpers)
  ui/         Theme, Widget, LockScreen, Clock, PasswordField, StatusMessage,
              ActionButton, PowerDialog, AudioController, Notification,
              Shell, IconResolver
  ui/statusbar/  StatusBar, StatusIndicator, IndicatorRegistry, QSTile,
              QuickSettingsPanel, PopoverManager, DetailedPopover
  ui/indicators/ BatteryIndicator, BluetoothIndicator, BrightnessIndicator,
              ClockIndicator, DNDIndicator, MediaIndicator, NotificationIndicator,
              PowerMenuIndicator, SNITrayHost, VolumeIndicator, WifiIndicator,
              WorkspacesIndicator, ActiveWindowIndicator
  auth/       PamAuthenticator (PAM on a worker thread)
  power/      PowerManager (systemctl)
  mpris/      MprisController (sdbus-c++)
  notifications/ NotificationMonitor (sd-bus monitor), NotificationLog,
              NotificationActions
  system/     SystemBus, BatteryBackend, BluetoothBackend, BrightnessBackend,
              DndState, SNIBackend, VolumeBackend, WifiBackend,
              WorkspaceBackend, ToplevelBackend, PulseLoop
  video/      VideoPlayer (libmpv software render -> cairo)
```

- **KISS** -- software cairo rendering instead of a bespoke GL/shader stack;
  synchronous MPRIS polling instead of signal plumbing.
- **DRY** -- one `Theme`, one `Painter`, one `EventLoop`; widgets share a base;
  the lock screen and status bar share the same StatusBar + indicators.
- **SOLID** -- the Wayland layer talks to the UI only through `InputSink` /
  `RenderHost`; PAM, power, MPRIS, and system backends are self-contained
  services.

## Security notes

- The password is copied straight into the PAM worker and cleared right after;
  the UI only ever holds the in-progress input.
- Authentication runs off the UI thread so a slow PAM stack can't freeze the
  lock surface.
