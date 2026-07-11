# Notifications on the lock screen

Windows 11-style notification cards (app tile + title + body), bottom-left of the
lock screen, fed by the user's **real** notification traffic — daemon-agnostic
(SwayNC, dunst, mako, …) and implemented strictly on the two relevant standards:

- **[Desktop Notifications Specification 1.2](https://specifications.freedesktop.org/notification-spec/)**
  (`org.freedesktop.Notifications`): message shapes, `replaces_id`, the `urgency`
  and `transient` hints, `NotificationClosed` reasons.
- **[D-Bus Specification — Monitoring interface](https://dbus.freedesktop.org/doc/dbus-specification.html#bus-messages-become-monitor)**
  (`org.freedesktop.DBus.Monitoring.BecomeMonitor`): the sanctioned way to observe
  bus traffic. The match rules are passed **as the argument to `BecomeMonitor`**;
  a monitor connection is receive-only afterwards, so installing matches later
  (classic `AddMatch`, `eavesdrop=true`) can never work — that was the fatal flaw
  of the previous implementation.

## Architecture

```
        app ──Notify──▶ session bus ──▶ daemon (SwayNC/dunst/mako)
                            │
              BecomeMonitor │ (rules passed in the call; connection is
                            │  receive-only afterwards)
              ┌─────────────┴───────────────┐
              ▼                             ▼
    ┌────────────────────────┐   ┌────────────────────────────┐
    │ qypr-record            │   │ qypr-lock (locked session)  │
    │ NotificationMonitor     │   │ NotificationMonitor         │
    │  + NotificationLog      │◀──│  seeds via List() before    │
    │ serves the mirror as    │   │  its own BecomeMonitor,     │
    │ org.qypr.Notifications  │   │  then captures live         │
    └────────────────────────┘   └──────────────┬─────────────┘
     session-long systemd                       │ onChange push
     --user service                             ▼
                                 ┌────────────────────────────┐
                                 │ LockScreen → NotificationView│
                                 │ (src/ui/Notification.*)      │
                                 └────────────────────────────┘
```

One dedicated `sd-bus` connection (libsystemd — already a transitive dependency of
sdbus-c++) becomes a monitor for exactly three rules:

1. `type='method_call',interface='org.freedesktop.Notifications',member='Notify'`
2. `type='method_return',sender='org.freedesktop.Notifications'`
3. `type='signal',interface='org.freedesktop.Notifications',member='NotificationClosed'`

A message filter (`sd_bus_add_filter`) classifies each monitored message; the
connection's fd is driven by the app's `epoll` loop (`sd_bus_process` on
readable). Everything runs on the UI thread: **no background thread, no lock,
no 1-second polling** — the monitor pushes into the view only when the set
actually changes.

## Pre-lock backlog: the `qypr-record` service

A monitor only sees traffic **after** it starts, and SwayNC exposes no content
read-back API (its `org.erikreider.swaync.cc` interface carries counts and DND
state only). So the `qypr-record` service runs the *same* `NotificationMonitor`
mirroring the daemon's queue in memory:

- captures every non-transient `Notify` (with `replaces_id` folding),
- **drops entries the user dismisses** (`NotificationClosed` reasons 2/3 — a
  SwayNC "clear" removes them from the mirror too),
- serves the mirror as the D-Bus service `org.qypr.Notifications`
  (`List() → a(xsssuy)`; see `NotificationLog.*`).

At lock time the lock screen's monitor calls `List()` on its own connection —
in the window *before* its `BecomeMonitor` makes it receive-only — seeds the
view with the undismissed backlog (daemon ids intact, so `NotificationClosed`
still matches seeded cards), then captures live on top. No persistence file:
the mirror lives and dies with the session, exactly like the daemon's queue.

Install:

```sh
install -Dm644 systemd/qypr-notification-log.service \
    ~/.config/systemd/user/qypr-notification-log.service
# edit ExecStart= to point at your built binary
systemctl --user daemon-reload
systemctl --user enable --now qypr-notification-log.service
```

Without the service the lock screen still works — it just shows only what
arrives while locked (a one-line log notes the missing backlog).

## Spec behaviours honoured

| Spec feature | Behaviour |
|---|---|
| `replaces_id` | Updates the existing card in place (same view key → no re-animation). |
| Daemon-assigned id | The daemon's `Notify` **reply** is correlated back to the call (`sender` + message cookie → `reply_cookie` + `destination`), so every card learns its real notification id. |
| `NotificationClosed(id, reason)` | Reason 2 (user dismissed) and 3 (`CloseNotification`) remove the card. Reason 1 (popup expired) is kept — while locked, this stack *is* the user's queue. |
| `urgency` hint | Parsed (tolerantly: any integral variant type); critical cards get the red accent. |
| `transient` hint | Skipped entirely (volume OSDs and the like are never queued). |

Verified live against SwayNC 0.12.6: capture, id correlation, critical urgency,
transient suppression, in-place replacement, and dismissal via
`swaync-client --close-latest` all behave as above.

## What this replaces (and why)

The previous implementation was removed wholesale:

- **sdbus-c++ `addMatch` with `eavesdrop='true'`** — spec-invalid after
  `BecomeMonitor` (the connection can no longer send the `AddMatch` call); it
  never captured anything.
- **`policy/qypr-lock.conf`** (system D-Bus policy edit) — unnecessary:
  `BecomeMonitor` is already granted to the session-bus owner.
- **The TSV `NotificationStore` + file-seeded recorder** — a hand-rolled
  persistence format written to disk to patch over the broken capture, with no
  dismissal tracking (cleared notifications would reappear on the lock screen).
  Replaced by the in-memory `qypr-record` mirror above, handed over via D-Bus.
- **Demo-card fallback on the real lock screen** — fake data in production is
  wrong; sample cards now appear only in `--preview`.

## Limitations

- The pre-lock backlog needs the `qypr-record` service; without it only
  notifications arriving while locked are shown.
- Actions (`actions` array) are not rendered; clicking a card dismisses it
  locally only (a monitor connection cannot call `CloseNotification`).
- Do-not-disturb is the daemon's concern; qypr-lock mirrors the raw `Notify`
  stream regardless of DND popup suppression.
