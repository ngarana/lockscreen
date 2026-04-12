# Qypr - Agent Instructions

## Architecture Overview

Qypr is a modular desktop shell system for Hyprland/Wayland built with [Quickshell](https://quickshell.org/) and Qt6/QML. It supports multiple shell components: **lockscreen**, **status-bar**, **launcher**, **notifications**, and **control-center**.

The codebase follows **Atomic Design** principles (atoms → molecules → organisms → modules) with business logic encapsulated in **QML singletons**. Components communicate via **signals/slots** for loose coupling. Each feature is a self-contained module under `src/modules/`.

The test suite has three layers:
- `tests/manual/` — Visual regression and component showcase tests
- `tests/unit/` — Service and model unit tests (planned)
- `tests/integration/` — Cross-module integration tests (planned)

CI/testing is handled via `scripts/test.sh` with comprehensive validation.

---

# Working with Qypr

When asked to work on Qypr, follow this process:

## Step 1: Understand Current State

Check the roadmap to understand what's implemented:

```bash
cat docs/ROADMAP.md | head -100
```

**Current Status (as of last update)**:
- ✅ **Phase 0 (Foundation)**: Complete — Theme system, core utilities, shell.qml refactored
- ✅ **Phase 1 (Services)**: Complete — 11 singleton services implemented
- ✅ **Phase 2 (Atomic Components)**: Complete — 13 atoms + 14 molecules + 5 layouts + 4 animations
- ✅ **Phase 3 (Composite Components)**: Complete — GlassPanel, Calendar, WeatherWidget, SystemMonitor, QuickSettings
- ✅ **Phase 10 (Scripts & Tooling)**: Complete — All launcher scripts, theme JSON definitions
- ⏳ **Phase 4-7 (Modules)**: Pending — StatusBar, Launcher, Notifications, ControlCenter

## Step 2: Follow Project Conventions

### QML Type Safety (Qt6 Best Practice)

Always use explicit types, not `var`:

```qml
// ✅ Good: Explicit types
property string name: ""
property int count: 0
property bool isActive: false
property color backgroundColor: "#ffffff"

// ❌ Bad: Loses type checking, errors point to declaration not assignment
property var name
property var count
```

### Declarative Bindings

Prefer declarative bindings over imperative assignments:

```qml
// ✅ Good: Declarative binding
Rectangle {
    color: mouseArea.containsMouse ? Theme.colors.primary : Theme.colors.surface
}

// ❌ Bad: Imperative assignment breaks bindings, slower, delays errors
Component.onCompleted: { color = Theme.colors.primary }
```

### Singleton Pattern

Services require **both** `pragma Singleton` in QML file **and** `singleton` keyword in qmldir:

```qml
// src/services/MyService.qml
pragma Singleton
import QtQuick

QtObject {
    id: root
    property bool isActive: false
    signal stateChanged()
}
```

```qmldir
# src/services/qmldir
module Services
singleton MyService 1.0 MyService.qml
```

### Import Paths

**Use relative imports only.** Avoid `root:/` imports (deprecated, breaks LSP):

```qml
// ✅ Good: Relative imports
import "../services"
import "../theme"

// ❌ Bad: Absolute imports cause LSP issues
import "root:/src/services"
```

## Step 3: Component Development

### Adding an Atom (Primitive Component)

1. Create in `src/atoms/` following naming convention (PascalCase)
2. Use the component template header:

```qml
// Button.qml - Base button with hover, pressed, disabled states
//
// Provides theme-integrated button with glassmorphic styling.
// Supports filled, outlined, and ghost variants.
//
// Properties:
// - text: string — Button label
// - variant: string — "filled" | "outlined" | "ghost"
// - size: string — "small" | "medium" | "large"
// - icon: string — Optional icon name
// - enabled: bool — Interactivity state
//
// Signals:
// - clicked() — Emitted when button is clicked

import QtQuick
import "../theme" as Theme

Rectangle {
    id: root
    // Implementation...
}
```

3. Export in `src/atoms/qmldir`
4. Document all properties and signals

### Adding a Module

1. Create directory: `src/modules/<module-name>/`
2. Create `qmldir` with exports:

```qmldir
module StatusBarModule

StatusBar 1.0 StatusBar.qml
StatusBarLeft 1.0 StatusBarLeft.qml
StatusBarCenter 1.0 StatusBarCenter.qml
StatusBarRight 1.0 StatusBarRight.qml

singleton BarController 1.0 BarController.qml

internal BarLayout BarLayout.qml
internal WidgetContainer WidgetContainer.qml
```

3. Implement main component and controller (singleton)
4. Add to `shell.qml` using `Variants` for multi-monitor support:

```qml
Variants {
    model: Quickshell.screens
    delegate: PanelWindow {
        screen: modelData
        StatusBar { anchors.fill: parent }
    }
}
```

## Step 4: Quickshell Integration

### State Persistence (Scope)

Use `Scope` for objects that must survive hot-reloads:

```qml
Scope {
    // IPC handlers, Process, Timer survive UI reloads
    IpcHandler {
        target: "qypr"
        function lock(): void { LockController.lock() }
    }
}
```

### Multi-Monitor (Variants)

Use `Variants` with `Quickshell.screens` for per-screen instances:

```qml
WlSessionLock {
    locked: true
    Variants {
        model: Quickshell.screens
        delegate: WlSessionLockSurface {
            screen: modelData
            LockScreen { anchors.fill: parent }
        }
    }
}
```

### Key Quickshell Components

| Component | Purpose |
|-----------|---------|
| `ShellRoot` | Root element for all configs |
| `PanelWindow` | Wayland/X11 panel (status bar) |
| `FloatingWindow` | Regular desktop window |
| `WlSessionLock` | Wayland session lock |
| `WlSessionLockSurface` | Per-screen lock surface |
| `Variants` | Dynamic multi-screen instantiation |
| `Scope` | State persistence across reloads |
| `IpcHandler` | Inter-process communication |
| `Process` | Run external commands |

## Step 5: Theme System

All UI must use Theme tokens — no hardcoded colors:

```qml
import "../theme" as Theme

Rectangle {
    color: Theme.colors.surface
    radius: Theme.radius.medium
    
    Behavior on color {
        ColorAnimation { duration: Theme.animation.medium }
    }
}
```

### Theme Structure

```qml
Theme.colors.background    // Background color
Theme.colors.surface       // Surface/container color
Theme.colors.primary       // Primary accent
Theme.colors.text          // Primary text
Theme.colors.glass         // Glassmorphic fill
Theme.colors.glassBorder   // Glassmorphic border

Theme.fonts.textSize       // Base text size
Theme.fonts.fontFamily     // Primary font

Theme.spacing.small        // 8px
Theme.spacing.medium       // 16px
Theme.spacing.large        // 24px

Theme.radius.small         // 4px
Theme.radius.medium        // 8px
Theme.radius.large         // 16px

Theme.animation.fast       // 150ms
Theme.animation.medium     // 300ms
Theme.animation.slow       // 500ms
```

## Step 6: Testing

Run tests before committing:

```bash
# Full test suite
./scripts/test.sh

# Development mode (no lock)
./scripts/run.sh

# Lock mode (production)
./scripts/lock.sh

# Specific module
QUICKSHELL_MODE=bar ./scripts/run.sh
```

### Manual Testing

Visual test pages exist in `tests/manual/`:
- `AtomTests.qml` — Atom component tests
- `MoleculeTests.qml` — Molecule component tests
- `ComponentShowcase.qml` — Comprehensive demo

## Step 7: Validation

Before committing, verify:

1. **All imports use relative paths** (no `root:/`)
2. **Singletons have both `pragma Singleton` and `singleton` in qmldir**
3. **Explicit types on all properties** (no `var`)
4. **Theme tokens used** (no hardcoded colors/values)
5. **Component documentation** (properties, signals documented)
6. **Test suite passes**: `./scripts/test.sh`

## Step 8: Commit

Write a descriptive commit message listing:
- What module/component was added/modified
- Phase completion status (if applicable)
- Whether documentation was updated
- Whether tests were added/updated
- Any breaking changes

---

# Key Services

| Service | Purpose | Signals |
|---------|---------|---------|
| `ConfigService` | User preferences | `configChanged` |
| `AudioService` | MPRIS/media players | `playbackStateChanged`, `metadataChanged` |
| `NetworkService` | WiFi/Ethernet | `networkStatusChanged` |
| `BluetoothService` | Bluetooth devices | `bluetoothStatusChanged` |
| `BatteryService` | Battery monitoring | `batteryLevelChanged` |
| `BrightnessService` | Display brightness | `brightnessChanged` |
| `HyprlandService` | WM integration | `workspaceChanged`, `windowOpened` |
| `NotificationService` | Notifications | `notificationReceived` |
| `SystemTrayService` | SNI protocol | `trayItemsChanged` |
| `PowerManager` | Power actions | `actionTriggered` |
| `SessionService` | User session | `sessionStarted` |

---

# Component Hierarchy

Following Atomic Design:

```
Atoms (Primitives)
├── Button, Icon, Label, Input, Slider, etc.
│
Molecules (Combinations)
├── Clock = Label (time) + Label (date)
├── VolumeControl = Icon + Slider
├── NotificationItem = Icon + Label + Button
│
Components/Organisms (Complex)
├── AudioController = Card + VolumeControl + MediaButtons
├── QuickSettings = Grid + Toggles + Sliders
│
Modules (Full Features)
├── LockScreen = VideoBackground + Clock + PasswordField
├── StatusBar = Taskbar + Clock + SystemIndicators
├── ControlCenter = QuickSettings + MediaControls
```

---

# Resources

- **Project Structure**: `docs/PROJECT_STRUCTURE.md`
- **Roadmap**: `docs/ROADMAP.md`
- **Component Reference**: `docs/COMPONENT_REFERENCE.md`
- **Qt6 QML Best Practices**: https://doc.qt.io/qt-6/qtquick-bestpractices.html
- **Qt6 QML Coding Conventions**: https://doc.qt.io/qt-6/qml-codingconventions.html
- **Quickshell Documentation**: https://quickshell.org/docs/
- **Atomic Design**: https://atomicdesign.bradfrost.com/

---

# Notes

- **Catppuccin Mocha** is the default theme palette
- **Glassmorphism** aesthetic: blur, transparency, subtle borders
- **Keep modules independent** — use signals for cross-module communication
- **Never store state in delegates** — store in models/services instead
- **Prefer `onMoved` over `onValueChanged`** for slider interactions
- **LSP Support**: `.qmlls.ini` auto-managed by Quickshell

---

*Last updated: 2026-04-12*
*Version: 1.0*
*Status: Phases 0-3 Complete, Ready for Phase 4 (StatusBar Module)*
