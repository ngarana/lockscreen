# Audio Controller Component Specification

## Overview

Add audio playback controls to the lockscreen that automatically appear when audio is active through **MPV** or **MPD**. The controls should match the existing glassmorphic design, follow the same reveal/hide behavior as the rest of the lockscreen UI, and be implemented in a way that fits **Quickshell 0.2.x** and normal **Qt Quick / QML** component design.

The primary implementation path should use **Quickshell's built-in MPRIS service** instead of hand-rolling D-Bus bindings. If we later want the slider to control the **system output volume** instead of the **active player volume**, that should be a separate optional integration via **Quickshell.Services.Pipewire**.

---

## Architecture

### Supported Sources

1. **MPV** via MPRIS bridge, typically exposed as `org.mpris.MediaPlayer2.mpv`
2. **MPD** via MPRIS bridge, typically exposed as `org.mpris.MediaPlayer2.mpd`

The lockscreen should treat **MPRIS** as the source-of-truth interface for playback state, metadata, transport controls, and player volume. Native MPD TCP support should be considered an **optional fallback phase**, not the default architecture.

### Preferred Quickshell Stack

- `Quickshell.Services.Mpris.Mpris` provides `players`, an `ObjectModel<MprisPlayer>`
- `MprisPlayer` already exposes player state, metadata, control methods, and capability flags
- `Quickshell.Services.Pipewire` is only needed if the product requirement becomes "control speaker/output volume" instead of "control the active player's MPRIS volume"

### Component Hierarchy

```text
src/
├── components/
│   ├── AudioController.qml        # Main audio controller widget
│   ├── AudioPlayerButton.qml      # Individual transport button
│   └── AudioMetadata.qml          # Track information + progress display
├── services/
│   ├── AudioService.qml           # Singleton wrapper around Quickshell MPRIS
│   └── AudioConfig.qml            # Optional future singleton config
└── widgets/
    └── LockScreen.qml             # Modified to include AudioController
```

---

## Component Specifications

### 1. AudioService.qml (Service Layer)

**Purpose:** Provide a single lockscreen-friendly API over Quickshell's MPRIS player model.

#### Root Type and Registration

Use a QML singleton and make `Singleton` the root object:

```qml
pragma Singleton

import Quickshell
import Quickshell.Services.Mpris
import QtQuick

Singleton {
    id: root
}
```

Add it to `src/services/qmldir`:

```text
singleton AudioService 1.0 AudioService.qml
```

#### Responsibilities

- Observe `Mpris.players`
- Select the active player with a stable priority order
- Flatten `MprisPlayer` state into simple UI-facing properties
- Expose capability-gated transport methods
- Normalize source identity (`"mpv"`, `"mpd"`, or `""`)
- Optionally support a future output-volume mode backed by Pipewire

#### Key Properties

Use Quickshell's property model directly where possible. Important detail: **Quickshell MPRIS position and length are in seconds, not milliseconds**.

```qml
readonly property var players: Mpris.players.values
readonly property var activePlayer: null

readonly property bool hasPlayer: activePlayer !== null
readonly property bool isActive: hasPlayer && activePlayer.playbackState !== MprisPlaybackState.Stopped
readonly property bool isPlaying: hasPlayer && activePlayer.isPlaying

readonly property string title: hasPlayer ? activePlayer.trackTitle : ""
readonly property string artist: hasPlayer ? activePlayer.trackArtist : ""
readonly property string album: hasPlayer ? activePlayer.trackAlbum : ""
readonly property string source: hasPlayer ? sourceFromDbusName(activePlayer.dbusName) : ""
readonly property string sourceLabel: hasPlayer ? activePlayer.identity : ""

readonly property real durationSeconds: hasPlayer && activePlayer.lengthSupported ? activePlayer.length : 0
readonly property real positionSeconds: hasPlayer && activePlayer.positionSupported ? activePlayer.position : 0
readonly property real volume: hasPlayer && activePlayer.volumeSupported ? activePlayer.volume : 1.0

readonly property bool hasMetadata: title !== "" || artist !== "" || album !== ""
readonly property bool canControl: hasPlayer && activePlayer.canControl
readonly property bool canTogglePlaying: hasPlayer && activePlayer.canTogglePlaying
readonly property bool canGoNext: hasPlayer && activePlayer.canGoNext
readonly property bool canGoPrevious: hasPlayer && activePlayer.canGoPrevious
readonly property bool canSeek: hasPlayer && activePlayer.canSeek && activePlayer.positionSupported
readonly property bool canSetVolume: hasPlayer && activePlayer.canControl && activePlayer.volumeSupported

property var playerPriority: [
    "org.mpris.MediaPlayer2.mpv",
    "org.mpris.MediaPlayer2.mpd"
]
property string volumeMode: "player" // "player", "output", "none"
property string lastActiveDbusName: ""
```

#### Key Methods

```qml
function play()
function pause()
function togglePlaying()
function next()
function previous()
function stop()
function setVolume(level)   // 0.0 - 1.0
function seekTo(seconds)    // absolute seconds
function selectPreferredPlayer(players)
function sourceFromDbusName(dbusName)
```

#### Signals

```qml
signal activePlayerChanged()
signal playbackStateChanged(bool playing)
signal metadataChanged()
signal volumeChanged(real volume)
signal positionChanged(real positionSeconds)
signal playerChanged(string source)
```

#### Implementation Notes

- Import `Quickshell.Services.Mpris` and use `Mpris.players`; do **not** start with raw D-Bus property monitoring for the main path.
- `Mpris.players` is an `ObjectModel`, so any JS filtering/sorting logic should use `Mpris.players.values` for reactive updates.
- Use the typed `MprisPlayer` properties instead of unpacking raw MPRIS metadata maps:
  - `trackTitle`
  - `trackArtist`
  - `trackAlbum`
  - `identity`
  - `dbusName`
  - `isPlaying`
  - `playbackState`
  - `canTogglePlaying`
  - `canGoNext`
  - `canGoPrevious`
  - `canSeek`
  - `positionSupported`
  - `volumeSupported`
- Always check `canXyz` or `xyzSupported` properties before enabling UI or invoking commands.
- For absolute seek from a slider, set `activePlayer.position = seconds` when `canSeek` and `positionSupported` are true.
- `MprisPlayer.position` does not continuously update reactively unless it is being monitored. Use a `Timer` for 1 Hz updates or `FrameAnimation` only while a smooth slider/progress animation is actually visible.
- If `volumeMode === "output"` becomes a requirement, switch the slider to `Pipewire.defaultAudioSink` instead of overloading MPRIS player volume semantics.
- Native MPD TCP fallback is outside the MVP. If it is ever added, use `Quickshell.Io.Socket` or `Process`; the spec should not assume a `TcpSocket` type.

#### Active Player Selection Logic

Prefer predictable and stable selection over frequent switching:

```text
1. Collect supported players from Mpris.players.values
2. Filter to configured priorities (mpv, mpd by default)
3. Prefer currently playing players in priority order
4. If nothing is playing, keep the last active prioritized player while paused
5. Otherwise fall back to the first controllable prioritized player
6. Clear activePlayer only when no relevant players remain
```

This avoids flicker when a player briefly pauses between tracks.

---

### 2. AudioPlayerButton.qml (Component)

**Purpose:** Glassmorphic transport button for previous / play-pause / next actions.

**Base Pattern:** Reuse the existing `ActionButton.qml` visual style, but add transport-specific state and keyboard/accessibility behavior.

#### Properties

```qml
property string action: "playPause"   // "playPause", "next", "previous", "stop"
property string icon: ""
property string label: ""
property bool enabled: true
property bool hovered
property bool pressed
signal clicked()
```

#### Visual Design

- Match the existing circular glass button styling
- Use `implicitWidth` and `implicitHeight` instead of relying on fixed external sizing
- Default size: `Theme.audio.buttonSize`
- Icon size: `Theme.audio.buttonIconSize`
- Hover scale animation: `1.0 -> 1.1`
- Use non-emoji monochrome transport glyphs to avoid color-emoji fallback issues in `Text`

#### Qt / Accessibility Requirements

- Set `Accessible.role: Accessible.Button`
- Set `Accessible.name` to the button label
- Implement `Accessible.onPressAction` to trigger the same behavior as a click
- Support keyboard activation with `Enter`, `Return`, and `Space`
- Expose `enabled` so unsupported actions can be visibly disabled instead of silently failing

---

### 3. AudioMetadata.qml (Component)

**Purpose:** Display current track information and progress in a layout-friendly container.

#### Layout

Use `ColumnLayout` and `RowLayout`, not bare `Row` or `Column`, so the component behaves cleanly inside responsive containers and keeps pixel alignment.

```text
┌─────────────────────────────────────────┐
│  Now Playing                            │
│  Track Title                            │
│  Artist - Album                         │
│  ──────────────── 2:34 / 4:12 ─────     │
└─────────────────────────────────────────┘
```

#### Properties

```qml
property string title
property string artist
property string album
property string sourceLabel
property real positionSeconds: 0
property real durationSeconds: 0
property bool showProgress: true
property bool showSourceLabel: false
property bool isLive: durationSeconds <= 0
```

#### Visual Design

- Glassmorphic surface matching `Theme.colors.glass`
- Rounded corners: `Theme.radius.large`
- Subtle border: `Theme.colors.glassBorder`
- Text eliding for long strings
- Optional source label (`MPV` / `MPD`) if multiple players become confusing
- Progress bar should use a custom glass track or a fully styled control

#### Qt Layout Requirements

- Define `implicitWidth` and `implicitHeight`
- Set `clip: true` on the visual panel if long text animations are used
- Avoid using `childrenRect` to drive implicit size, because that is easy to turn into a binding loop in container components

#### Behavior

- Fade in/out using `Theme.animation.reveal`
- Show `"Unknown Track"` / `"Unknown Artist"` only in the rendered text layer, not by mutating service state
- Hide the progress bar when `showProgress` is false or `durationSeconds <= 0`
- Treat streams or unknown lengths as live content

---

### 4. AudioController.qml (Component)

**Purpose:** Combine metadata, transport controls, and optional volume into one lockscreen widget.

#### Root Structure

Use an outer `Item` with explicit implicit sizing, then place a glass panel inside with a `ColumnLayout`.

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../services"
import "../components"

Item {
    implicitWidth: panel.implicitWidth
    implicitHeight: panel.implicitHeight

    property bool revealed: false

    visible: revealed && AudioService.isActive
    enabled: visible
    opacity: visible ? 1.0 : 0.0

    Rectangle {
        id: panel

        ColumnLayout {
            AudioMetadata { }

            RowLayout {
                AudioPlayerButton { action: "previous" }
                AudioPlayerButton { action: "playPause" }
                AudioPlayerButton { action: "next" }
            }

            RowLayout {
                Text { text: "Vol" }
                Slider { }
                Text { text: "75%" }
            }
        }
    }
}
```

#### Properties

```qml
property bool revealed: false   // bound to root.uiRevealed
property bool showVolume: true
property bool showProgress: true
```

#### Behavior

- Auto-show only when both `revealed` and `AudioService.isActive` are true
- Set both `visible` and `enabled` when hidden; `opacity: 0` alone must not be treated as "non-interactive"
- Position bottom-center above the power buttons
- Keep the panel width bounded on small screens:
  - `width <= Math.min(parent.width - horizontalMargins, Theme.audio.maxWidth)`

#### Control Binding Rules

- Previous button enabled only when `AudioService.canGoPrevious`
- Play/pause button enabled only when `AudioService.canTogglePlaying`
- Next button enabled only when `AudioService.canGoNext`
- Seek bar enabled only when `AudioService.canSeek`
- Volume slider shown only when:
  - `showVolume`
  - `AudioService.volumeMode !== "none"`
  - and the chosen backend supports writable volume

#### Slider Notes

If `Slider` from `QtQuick.Controls` is used:

- Import `QtQuick.Controls`
- Fully style it so it does not look like an unthemed platform control
- Use `live: true` only if the backend write rate is acceptable
- Debounce writes if the active player is noisy about volume changes

If styling `Slider` cleanly becomes more work than value, replace it with a custom `Rectangle`-based track and handle.

---

## Integration with LockScreen.qml

### Placement

The audio controller should sit above the power button row and below the password/status area.

```text
Clock
PasswordField
StatusMessage
AudioController   <- new
Power Buttons
```

### Required Imports

```qml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../services"
import "../components"
```

### Integration Sketch

```qml
AudioController {
    id: audioController
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottom: powerButtonsRow.top
    anchors.bottomMargin: root.uiRevealed && visible ? Theme.spacing.large : 0

    revealed: root.uiRevealed
    showVolume: true
    showProgress: true
    z: 2
}
```

### Layout Notes

- Do not size the controller from `childrenRect`
- Keep the power button row anchored independently; only its spacing relationship to the controller should change
- Test on narrow displays so the controller does not collide with the password field or run off-screen

---

## Theme Extensions

Add to `Theme.qml`:

```qml
property var audio: QtObject {
    readonly property int buttonSize: 48
    readonly property int buttonIconSize: 20
    readonly property int minWidth: 320
    readonly property int maxWidth: 420
    readonly property int progressHeight: 4
    readonly property int volumeSliderWidth: 150
    readonly property int spacing: 12
    readonly property int panelPadding: 16
}
```

---

## Quickshell / MPRIS Mapping

Use Quickshell's typed MPRIS API directly:

- `Mpris.players.values` -> candidate players list
- `player.dbusName` -> source detection / priority
- `player.identity` -> human-readable label
- `player.trackTitle` -> title
- `player.trackArtist` -> artist
- `player.trackAlbum` -> album
- `player.isPlaying` / `player.playbackState` -> playback state
- `player.position` -> current position in **seconds**
- `player.length` -> duration in **seconds**
- `player.volume` -> player volume only
- `player.canTogglePlaying` / `canGoNext` / `canGoPrevious` / `canSeek` -> UI enablement

Do not build the core UI against raw `Metadata` maps unless a future feature needs fields that `MprisPlayer` does not already expose.

---

## Runtime Requirements

### MPV

MPV must expose an MPRIS bridge. Do not hardcode a single distro-specific script path in implementation logic; treat the exact installation path as environment-specific.

### MPD

MPD must also expose an MPRIS bridge, commonly via `mpd-mpris`.

### Optional Output Volume Mode

If the requirement is "speaker volume" rather than "player volume":

- Add `import Quickshell.Services.Pipewire`
- Bind to `Pipewire.defaultAudioSink`
- Keep that behavior behind `AudioService.volumeMode === "output"`

---

## Behavior Specifications

### Auto-Detection Logic

```text
IF prioritized playing player exists THEN
    activePlayer = that player
ELSE IF last active prioritized player still exists and is paused THEN
    activePlayer = last active player
ELSE IF prioritized controllable player exists THEN
    activePlayer = that player
ELSE
    activePlayer = null
END IF
```

### State Transitions

1. **No player -> player starts**
   - Detect from `Mpris.players.values` and `playbackState`
   - Update `activePlayer`
   - Fade in controller when `root.uiRevealed` is also true

2. **Playing -> paused**
   - Keep controller visible while the player is still considered active
   - Do not immediately clear metadata

3. **Player stops / disappears**
   - Optionally keep the last state for a short debounce window to avoid flicker
   - Then fade out and clear the active player

4. **Player switch (MPV <-> MPD)**
   - Update `activePlayer`
   - Swap bindings without a UI reset animation

### Interaction Behavior

- Play/pause toggles through `AudioService.togglePlaying()`
- Next/previous only enabled when the player advertises support
- Progress bar click/drag seeks only when `AudioService.canSeek`
- Volume writes should be clamped to `0.0 .. 1.0`

---

## Edge Cases

1. **No metadata available**
   - Show fallback display strings in the metadata component
   - Keep controls visible if transport control is still available

2. **Live stream or unknown length**
   - Hide seek bar or render a live-state label
   - Do not show bogus `0:00 / 0:00`

3. **Player has no writable volume**
   - Hide the player-volume slider
   - Or switch to Pipewire output volume mode

4. **Multiple players active**
   - Use priority order plus "sticky" last-active behavior
   - Avoid flapping between players every time metadata changes

5. **MPRIS unavailable**
   - Log a warning
   - Keep the controller hidden
   - Treat native MPD TCP as a later fallback, not MVP behavior

6. **Hidden but transparent widget**
   - Hidden state must disable interactivity; opacity alone is not enough

---

## Accessibility and Input

- Transport buttons should be keyboard-activatable
- Buttons should expose accessible role/name/action
- Focus should remain predictable when the controller reveals or hides
- If the controller is hidden, it should not keep active focus

Future enhancement:

- Media key integration for play/pause/next/previous

---

## Testing Plan

### Manual Testing

Create `tests/manual_audio_test.sh`:

```bash
#!/usr/bin/env bash
set -euo pipefail

echo "=== Audio Controller Manual Tests ==="

echo "Active MPRIS players:"
playerctl -l || true

echo
echo "1. Start MPV playback"
echo "   Check: controller appears after UI reveal"
echo "   Check: title / artist / album populate"
echo "   Check: play-pause button reflects state"

echo
echo "2. Test MPV transport"
echo "   playerctl -p mpv play-pause"
echo "   playerctl -p mpv next"

echo
echo "3. Switch to MPD"
echo "   Stop MPV and start MPD playback"
echo "   Check: controller switches cleanly"

echo
echo "4. Capability checks"
echo "   Verify next/previous/seek/volume only show as enabled when supported"

echo
echo "5. Hide / reveal behavior"
echo "   Reveal UI -> controller visible"
echo "   Auto-hide UI -> controller hidden and non-interactive"
```

### Integration Testing

1. Start lockscreen while MPV is already playing
2. Start lockscreen while MPD is already playing
3. Pause and resume while locked
4. Stop playback and verify controller hides cleanly
5. Verify focus returns to the password field when appropriate
6. Verify narrow-screen layout does not overlap lower controls

---

## Implementation Phases

### Phase 1: Core Service

- [x] Add `AudioService.qml` singleton
- [x] Register it in `src/services/qmldir`
- [x] Select active player from `Mpris.players.values`
- [x] Expose flattened metadata / transport / capability properties
- [x] Add monitored position updates only while needed

### Phase 2: UI Components

- [x] Add `AudioPlayerButton.qml`
- [x] Add `AudioMetadata.qml`
- [x] Add `AudioController.qml`
- [x] Style progress and volume controls to match the theme
- [x] Add accessibility and keyboard activation

### Phase 3: Integration

- [x] Modify `LockScreen.qml`
- [x] Add new component entries to `src/components/qmldir`
- [x] Extend `Theme.qml`
- [x] Verify reveal/hide and focus behavior

### Phase 4: Optional Enhancements

- [x] Add Pipewire-backed output volume mode
- [x] Add sticky-player debounce logic if needed
- [x] Add native MPD fallback only if MPRIS bridge is unavailable in real use

### Phase 5: Documentation

- [x] Update `README.md`
- [x] Add setup notes for MPV / MPD MPRIS bridges
- [x] Add troubleshooting guide

---

## Configuration

### Future Config Surface

```qml
// ~/.config/lockscreen/config.qml
AudioConfig {
    enabled: true
    playerPriority: [
        "org.mpris.MediaPlayer2.mpv",
        "org.mpris.MediaPlayer2.mpd"
    ]
    showVolume: true
    showProgress: true
    volumeMode: "player"   // "player", "output", "none"
    lingerAfterStopMs: 3000
}
```

### Environment Variables

```bash
# Audio controller debug mode
LOCKSCREEN_AUDIO_DEBUG=1

# Only relevant if a future MPD native fallback is implemented
MPD_HOST=localhost
MPD_PORT=6600
```

---

## Dependencies

### Required

- **Quickshell** with `Quickshell.Services.Mpris`
- **MPRIS-compatible player exposure** for MPV and/or MPD

### Optional

- **Quickshell.Services.Pipewire** for output volume mode
- **playerctl** for testing
- **mpc** for MPD testing
- **mpd-mpris** if MPD does not already expose MPRIS

---

## Performance Considerations

1. Use Quickshell's reactive MPRIS model instead of polling raw D-Bus properties
2. Only force `positionChanged()` while playback is visible and position is actually needed
3. Prefer `Timer` for once-per-second label updates; reserve `FrameAnimation` for smooth dragging
4. Keep the controller layout bounded and simple to avoid unnecessary scenegraph churn

---

## Security Considerations

1. Audio controls must remain completely separate from authentication state
2. Only interact with session-bus media interfaces for the MPRIS path
3. Do not shell-escape or execute metadata content
4. If future fallback code spawns helper commands, use explicit argument arrays rather than shell strings

---

## Future Enhancements

1. Album art display
2. Output-device picker backed by Pipewire
3. Media-key support
4. Source badge or switcher when multiple players are active
5. Optional compact mode for very small displays

---

## Troubleshooting

### Players not detected

- Check available players: `playerctl -l`
- Check session bus names: `busctl --user list | grep org.mpris.MediaPlayer2`

### MPV not detected

- Ensure an MPRIS bridge is installed and loaded for MPV
- Verify MPV appears in `playerctl -l`

### MPD not detected

- Ensure `mpd-mpris` or equivalent bridge is running
- Verify MPD itself is active with `mpc status`

### Controls disabled unexpectedly

- Inspect capability flags on the active player:
  - `canTogglePlaying`
  - `canGoNext`
  - `canGoPrevious`
  - `canSeek`
  - `volumeSupported`

### Controller not showing

- Verify `AudioService.activePlayer` is set
- Verify `AudioService.isActive` is true
- Verify `root.uiRevealed` is true
- Enable debug logging with `LOCKSCREEN_AUDIO_DEBUG=1`

### Slider changes the wrong volume

- MPRIS volume is **player volume**
- If the requirement is **speaker/output volume**, use `volumeMode: "output"` and add Pipewire integration

---

## References

- [Quickshell Mpris Service](https://quickshell.org/docs/types/Quickshell.Services.Mpris/Mpris)
- [Quickshell MprisPlayer](https://quickshell.org/docs/types/Quickshell.Services.Mpris/MprisPlayer)
- [Quickshell ObjectModel](https://quickshell.org/docs/types/Quickshell/ObjectModel)
- [Quickshell Item Size and Position Guide](https://quickshell.org/docs/guide/size-position/)
- [Quickshell QML Language Guide](https://quickshell.org/docs/guide/qml-language/)
- [Qt QML Singletons](https://doc.qt.io/qt-6/qml-singleton.html)
- [Qt Quick Item](https://doc.qt.io/qt-6/qml-qtquick-item.html)
- [Qt Quick Controls Slider](https://doc.qt.io/qt-6/qml-qtquick-controls-slider.html)
- [Qt Quick Accessible](https://doc.qt.io/qt-6/qml-qtquick-accessible.html)
- [MPRIS Specification](https://specifications.freedesktop.org/mpris-spec/latest/)
