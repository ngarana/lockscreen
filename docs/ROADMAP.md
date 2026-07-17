# qypr Development Roadmap

qypr is a native C++ Wayland lockscreen with an integrated, **general-purpose
status bar** — and, as of Phase 7, that same bar also ships as a standalone
desktop panel (`qypr-bar`, a waybar replacement) via wlr-layer-shell. The bar
targets Hyprland *and every other Wayland compositor*: it uses only standard
Wayland protocols and freedesktop/systemd interfaces — never compositor-specific
APIs (no `hyprctl`, no Hyprland IPC) and never daemon-specific interfaces. Widget
specs live in [STATUS_BAR.md](STATUS_BAR.md); this document sequences the work.

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
| 3a | **Brightness** — sysfs read, udev-push change events, logind write; scroll-to-adjust; QS slider | **Done** — verified live |
| 3b | **WiFi** — NetworkManager D-Bus on `SystemBus`; signal icon; QS toggle | **Done** — verified live |
| 3c | **Bluetooth** — BlueZ D-Bus on `SystemBus`; device count; QS toggle | **Done** — verified live |
| 3d | **DND** — qypr-local state, Shell-mediated suppression; QS toggle | **Done** — suppression verified in preview |
| 4 | **Volume** — libpulse against pipewire-pulse via the `PulseLoop` `pa_mainloop_api` adapter over `EventLoop`; QS slider + scroll | **Done** — verified live (mute toggle UI deferred to Phase 6 polish) |
| 5 | **SNI tray host** — host mode against the running `StatusNotifierWatcher` on the session bus; themed icons via inheritance-aware `IconResolver`; left-click `Activate` | **Done** — verified live (nm-applet + blueman icons render; blueman `Activate(ii)` targeted) |
| 6 | **Polish** — tray context menus (`com.canonical.dbusmenu`, needed by menu-only items like nm-applet) + async item fetch; full keyboard nav (Home/End, slider arrows); battery charging pulse; icon crossfades; per-indicator tooltips; volume mute-toggle UI | Planned |
| 7 | **Standalone `qypr-bar`** — separate binary hosting the same StatusBar on `wlr-layer-shell` for daily (unlocked) use on any Wayland WM. Reserves an exclusive zone (a real panel), enables session-sensitive widgets (`setSessionContentVisible(true)`), starts the WM backends, and draws a subtle backdrop (`setBackdrop`) so the chromeless glyphs stay legible over any wallpaper | **Done** — verified live on Hyprland (reserves 66px zone, stacks with waybar; QS opens with real WiFi/BT/battery data; surface grows 66→full for overlays and shrinks back) |
| 8 | **WM widgets** — workspaces (`ext-workspace-v1`) + active window (`wlr-foreign-toplevel-management`) | **Done** — verified live; **gated hidden while locked** (session-sensitive); now surface in the unlocked qypr-bar (Phase 7) |

## Sequencing rationale

- **D-Bus widgets first (3a–3c):** they reuse the proven `SystemBus` +
  push pattern from Battery — no new architecture per widget, one new
  backend each. Brightness leads because it also plumbs the pointer-scroll
  event path (Seat → InputSink → Shell → bar) that Volume reuses later.
- **Volume as its own phase (4):** libpulse is the only backend needing new
  event-loop machinery (a `pa_mainloop_api` adapter with io/time/defer
  support). Isolating it keeps that risk out of the D-Bus widgets.
- **Standalone bar (7):** the `Invalidator`-only dependency and the decoupling
  gate meant StatusBar was already hostable, so the new work was a layer-shell
  platform host (`BarDisplay` + `BarWindow`, parallel to `WaylandDisplay` +
  `Output`; the lock path is untouched) plus a thin `BarApp` that owns the same
  backends and routes input — the StatusBar itself did not change. `Seat` was
  decoupled from the concrete `Output` (a surface→size resolver) so it serves
  both hosts. The bar surface anchors top with an exclusive zone, and grows to
  the full output height only while an overlay is open (then shrinks back so the
  desktop keeps its clicks) — `StatusBar::hasOpenOverlay()` drives that. Landing
  it after the widget set made the first `qypr-bar` release useful on day one.
- **WM widgets (8) built ahead of the bar host, but gated:** the backends
  (`WorkspaceBackend`, `ToplevelBackend`) and indicators
  (`WorkspacesIndicator`, `ActiveWindowIndicator`) are done and verified live,
  but they are **session-sensitive** — they reveal the current workspace and
  focused window — so `StatusBar` hides them unless the host calls
  `setSessionContentVisible(true)`. The lock screen never does, so nothing
  leaks while locked; only the unlocked qypr-bar (Phase 7) turns them on and
  starts the backends. `ext-workspace-v1` is a standard protocol; the active
  window uses `wlr-foreign-toplevel-management` because it is the only widely
  supported protocol carrying per-window *focus* state (the standard
  `ext-foreign-toplevel-list-v1` is list-only). Still no per-WM IPC — portable
  across Hyprland, Sway, river, ….

## Per-phase verification

| Phase | Proof required |
|-------|---------------|
| 3a | Real backlight % shown; logind write path exercised; udev event observed on external change |
| 3b | Real SSID/strength; toggle round-trip via NetworkManager |
| 3c | Real adapter/device state; toggle round-trip via BlueZ |
| 3d | Cards suppressed while DND on; reappear intact when off |
| 4 | Live sink volume/mute mirror; slider/scroll writes audible in `wpctl status` |
| 5 | A real SNI app shows a themed icon (nm-applet `nm-signal-75`, blueman); left-click targets the item's real `Activate(ii)` (verified present on blueman) |
| 7 | Bar renders on Hyprland (verified: reserves a 66px exclusive zone, full width, stacks below waybar; workspaces + active window + system indicators all draw; gear-click opens Quick Settings with real WiFi/BT/battery/brightness/volume, growing the surface 66→1200 then shrinking back); session-sensitive widgets appear only here, never on the lock screen. Cross-WM (Sway/river) still to spot-check |
| 8 | Live workspace list with active highlight + switch-on-click; active-window title tracks focus — both **absent from the locked bar** (verified: preview shows neither), present only under `setSessionContentVisible(true)` |
