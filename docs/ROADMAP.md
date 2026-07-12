# qypr Development Roadmap

qypr is a native C++ Wayland lockscreen with an integrated, **general-purpose
status bar**. The bar targets Hyprland *and every other Wayland compositor*:
it uses only standard Wayland protocols and freedesktop/systemd interfaces —
never compositor-specific APIs (no `hyprctl`, no Hyprland IPC) and never
daemon-specific interfaces. Widget specs live in [STATUS_BAR.md](STATUS_BAR.md);
this document sequences the work.

## Non-negotiable gates (every phase)

1. **Strict decoupling** — StatusBar ⟂ LockScreen; `Shell` is the sole
   composition point; bar code depends only on shared foundations
   (`EventLoop`, `Painter`, `Theme`, `Widget`, `Invalidator`).
2. **Minimal footprint** — no spawned processes, no polling (push via fds in
   the epoll loop), one shared bus connection per bus, no threads.
3. **Native only** — standard protocols/daemons (UPower, logind,
   NetworkManager, BlueZ, PipeWire, udev); works on any Wayland WM.
4. **Live verification** — every backend is proven against the real daemon
   (harness + `--preview` frames) before its phase is called done. Unit
   tests alone do not close a phase.

## Status

| Phase | Scope | State |
|-------|-------|-------|
| 1 | Core framework: StatusBar, registry, QS panel, popovers | **Done** |
| 2 | Clock + Battery (UPower push over shared `SystemBus`) | **Done** — verified live |
| 2.5 | Lockscreen integration: chromeless, frame-aligned, reveal-dimmed | **Done** |
| 3a | **Brightness** — sysfs read, udev-push change events, logind write; scroll-to-adjust; QS slider | **In progress** |
| 3b | **WiFi** — NetworkManager D-Bus on `SystemBus`; signal icon; QS toggle | Planned |
| 3c | **Bluetooth** — BlueZ D-Bus on `SystemBus`; device count; QS toggle | Planned |
| 3d | **DND** — qypr-local state, Shell-mediated suppression; QS toggle | Planned |
| 4 | **Volume** — libpulse against pipewire-pulse; requires a `pa_mainloop_api` adapter over `EventLoop` (io/time/defer events); QS slider + scroll + mute | Planned |
| 5 | **SNI tray host** — `StatusNotifierWatcher`/Host on the session bus; third-party icons | Planned |
| 6 | **Polish** — full keyboard nav (Home/End, slider arrows), battery charging pulse, icon crossfades, per-indicator tooltips | Planned |
| 7 | **Standalone `qypr-bar`** — separate binary hosting the same StatusBar code on `wlr-layer-shell` for daily (unlocked) use on any Wayland WM | Planned |
| 8 | **WM widgets** (bar host) — workspaces via `ext-workspace-v1`, window title via `ext-foreign-toplevel-list` — compositor-agnostic protocols only | Planned |

## Sequencing rationale

- **D-Bus widgets first (3a–3c):** they reuse the proven `SystemBus` +
  push pattern from Battery — no new architecture per widget, one new
  backend each. Brightness leads because it also plumbs the pointer-scroll
  event path (Seat → InputSink → Shell → bar) that Volume reuses later.
- **Volume as its own phase (4):** libpulse is the only backend needing new
  event-loop machinery (a `pa_mainloop_api` adapter with io/time/defer
  support). Isolating it keeps that risk out of the D-Bus widgets.
- **Standalone bar late (7):** the `Invalidator`-only dependency and the
  decoupling gate mean StatusBar is already hostable; the new work is a
  layer-shell platform host, not bar changes. Landing it after the widget
  set makes the first `qypr-bar` release useful on day one.
- **WM widgets last (8):** workspaces/window-title only make sense in the
  unlocked bar host, and `ext-workspace-v1` keeps them portable across
  compositors (Hyprland, Sway, river, …) without any per-WM code.

## Per-phase verification

| Phase | Proof required |
|-------|---------------|
| 3a | Real backlight % shown; logind write path exercised; udev event observed on external change |
| 3b | Real SSID/strength; toggle round-trip via NetworkManager |
| 3c | Real adapter/device state; toggle round-trip via BlueZ |
| 3d | Cards suppressed while DND on; reappear intact when off |
| 4 | Live sink volume/mute mirror; slider/scroll writes audible in `wpctl status` |
| 5 | A real SNI app (e.g. nm-applet) shows an icon; activate works |
| 7 | Bar renders on Hyprland *and* one other compositor (e.g. Sway) |
