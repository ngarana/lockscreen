# Qypr Project Structure Documentation

## Overview

This document defines the architecture and project structure for transforming Qypr from a lockscreen-only application to a comprehensive desktop shell system supporting multiple components: **lockscreen**, **status-bar**, **launcher**, **notifications**, and **control-center**.

The architecture follows Qt6/QML best practices, Quickshell framework conventions, and software design patterns focused on modularity, extensibility, and reusability.

---

## Design Principles

### 1. Separation of Concerns
- **Presentation Layer**: UI components, widgets, and visual elements
- **Business Logic Layer**: Services, controllers, and data management
- **Data Access Layer**: Models, external API clients, and system integration

### 2. Component-Based Architecture
- Each major feature (lockscreen, status-bar, etc.) is a **self-contained module**
- Components communicate via **signals/slots** and **shared services**
- **No direct coupling** between unrelated modules

### 3. Reusability Through Composition
- **Atomic components** form the building blocks (buttons, inputs, cards)
- **Composite widgets** combine atoms for specific use cases
- **Module shells** compose widgets into complete features

### 4. Singleton Services Pattern
- Shared state and system integration via QML singletons
- Services require **both** `pragma Singleton` in QML file **and** `singleton` keyword in qmldir
- Prevents multiple instances of system-critical resources
- **Important**: Group related data in one singleton rather than creating many singletons

---

## Project Structure

```
qypr/
├── shell.qml                      # Main entry point - ShellRoot configuration
├── .qmlls.ini                     # LSP configuration (auto-managed by Quickshell)
├── README.md                      # Project documentation
├── AGENTS.md                      # Agent-specific instructions
│
├── docs/                          # Documentation
│   ├── ARCHITECTURE.md            # Architecture decisions
│   ├── COMPONENT_GUIDE.md         # Component usage guide
│   ├── API_REFERENCE.md           # Service API documentation
│   └── PROJECT_STRUCTURE.md       # This file
│
├── scripts/                       # Utility scripts
│   ├── run.sh                     # Development launcher
│   ├── lock.sh                    # Production lock launcher
│   ├── bar.sh                     # Status bar launcher
│   ├── launcher.sh                # Application launcher
│   └── test.sh                    # Test suite runner
│
├── assets/                        # Shared static assets
│   ├── fonts/                     # Custom font files
│   ├── icons/                     # SVG/PNG icons
│   │   ├── actions/               # Action icons (play, pause, etc.)
│   │   ├── apps/                  # Application icons
│   │   ├── categories/            # Category icons
│   │   ├── status/                # Status indicators
│   │   └── system/                # System icons (power, wifi, etc.)
│   ├── wallpapers/                # Static wallpaper images
│   │   ├── @2x/                   # High DPI variants (2x)
│   │   ├── @3x/                   # High DPI variants (3x)
│   │   └── @4x/                   # High DPI variants (4x)
│   ├── videos/                    # Video backgrounds (symlink to external)
│   └── sounds/                    # Audio feedback files
│
├── playlists/                     # Video playlist configurations
│   ├── day.m3u                    # Daytime video playlist
│   └── night.m3u                  # Nighttime video playlist
│
├── themes/                        # Theme definitions
│   ├── default/                   # Default Catppuccin Mocha theme
│   │   ├── theme.json             # Theme token definitions
│   │   └── preview.png            # Theme preview
│   └── catppuccin-latte/          # Alternative themes...
│
├── src/                           # Source code
│   ├── core/                      # Core framework utilities
│   │   ├── qmldir                 # Module definition
│   │   ├── Constants.qml          # System constants
│   │   ├── Errors.qml             # Error definitions
│   │   ├── Logger.qml             # Logging utility
│   │   └── Utils.qml              # General utilities (singleton)
│   │
│   ├── theme/                     # Theme system (singleton)
│   │   ├── qmldir
│   │   ├── ThemeEngine.qml        # Theme loading & management
│   │   ├── ColorPalette.qml       # Color definitions
│   │   ├── Typography.qml         # Font and text styles
│   │   ├── Spacing.qml            # Spacing scale
│   │   ├── Effects.qml            # Visual effects (blur, shadow)
│   │   └── Animation.qml          # Animation presets
│   │
│   ├── services/                  # Business logic services (singletons)
│   │   ├── qmldir                 # Module definition
│   │   ├── ConfigService.qml      # Configuration management
│   │   ├── AudioService.qml       # Audio/MPRIS integration
│   │   ├── NetworkService.qml     # Network status monitoring
│   │   ├── BatteryService.qml     # Battery/Power monitoring
│   │   ├── BluetoothService.qml   # Bluetooth management
│   │   ├── BrightnessService.qml  # Display brightness control
│   │   ├── NotificationService.qml # Notification management
│   │   ├── SystemTrayService.qml  # System tray/protocols
│   │   ├── PowerManager.qml       # System power actions
│   │   ├── HyprlandService.qml    # Hyprland WM integration
│   │   └── SessionService.qml     # Session management
│   │
│   ├── models/                    # Data models
│   │   ├── qmldir
│   │   ├── ApplicationModel.qml   # Installed applications
│   │   ├── NotificationModel.qml  # Notification data
│   │   ├── WorkspaceModel.qml     # Hyprland workspaces
│   │   ├── WindowModel.qml        # Window list
│   │   ├── CalendarModel.qml      # Calendar events
│   │   └── MediaModel.qml         # Media player data
│   │
│   ├── atoms/                     # Atomic UI components (primitives)
│   │   ├── qmldir
│   │   ├── Button.qml             # Base button component
│   │   ├── IconButton.qml         # Icon-only button
│   │   ├── TextButton.qml         # Text button with optional icon
│   │   ├── Icon.qml               # Icon wrapper with theme support
│   │   ├── Label.qml              # Text with theme typography
│   │   ├── Input.qml              # Base text input
│   │   ├── Slider.qml             # Custom styled slider
│   │   ├── ProgressBar.qml        # Progress indicator
│   │   ├── Card.qml               # Card/container component
│   │   ├── Badge.qml              # Notification badge
│   │   ├── Tooltip.qml            # Tooltip component
│   │   ├── Divider.qml            # Visual separator
│   │   └── Spinner.qml            # Loading spinner
│   │
│   ├── molecules/                 # Molecular components (atom combinations)
│   │   ├── qmldir
│   │   ├── Clock.qml              # Time/date display
│   │   ├── VolumeControl.qml      # Volume slider + icon
│   │   ├── BrightnessControl.qml  # Brightness slider + icon
│   │   ├── NetworkIndicator.qml   # Network status icon + menu
│   │   ├── BatteryIndicator.qml   # Battery icon + percentage
│   │   ├── WorkspaceIndicator.qml # Workspace dots/buttons
│   │   ├── WindowPreview.qml      # Window thumbnail preview
│   │   ├── NotificationItem.qml   # Single notification display
│   │   ├── MediaWidget.qml        # Media player mini widget
│   │   ├── SearchInput.qml        # Search with icon and clear
│   │   ├── ListItem.qml           # List item with icon, text, actions
│   │   ├── AppGridItem.qml        # Application grid tile
│   │   ├── PowerMenu.qml          # Power options dropdown
│   │   └── UserMenu.qml           # User menu with avatar
│   │
│   ├── components/                # Composite components (organisms)
│   │   ├── qmldir
│   │   ├── VideoBackground.qml    # Video background player
│   │   ├── GlassPanel.qml         # Glassmorphic panel container
│   │   ├── PasswordField.qml      # Secure password input
│   │   ├── StatusMessage.qml      # Status/error message
│   │   ├── ActionButton.qml       # Animated action button
│   │   ├── AudioMetadata.qml      # Audio info display
│   │   ├── AudioPlayerButton.qml  # Media control button
│   │   ├── AudioController.qml    # Full audio controls
│   │   ├── Calendar.qml           # Calendar widget
│   │   ├── WeatherWidget.qml      # Weather display
│   │   ├── SystemMonitor.qml      # CPU/RAM usage display
│   │   └── QuickSettings.qml      # Quick toggles grid
│   │
│   ├── layouts/                   # Layout templates
│   │   ├── qmldir
│   │   ├── PanelLayout.qml        # Horizontal panel layout
│   │   ├── PopupLayout.qml        # Centered popup/dialog
│   │   ├── DrawerLayout.qml       # Slide-out drawer
│   │   ├── GridLayout.qml         # Responsive grid
│   │   └── LayerLayout.qml        # Z-index layering helper
│   │
│   ├── modules/                   # Feature modules (shells)
│   │   ├── lockscreen/            # Lockscreen module
│   │   │   ├── qmldir
│   │   │   ├── LockScreen.qml     # Main lockscreen component
│   │   │   ├── LockController.qml # PAM auth controller (singleton)
│   │   │   ├── LockSurface.qml    # Lock surface per monitor
│   │   │   └── VideoConfig.qml    # Background video config
│   │   │
│   │   ├── statusbar/             # Status bar module
│   │   │   ├── qmldir
│   │   │   ├── StatusBar.qml      # Main status bar component
│   │   │   ├── StatusBarLeft.qml  # Left section (workspaces)
│   │   │   ├── StatusBarCenter.qml # Center section (clock)
│   │   │   ├── StatusBarRight.qml # Right section (system indicators)
│   │   │   ├── Taskbar.qml        # Window/task indicators
│   │   │   └── BarController.qml  # Bar visibility controller
│   │   │
│   │   ├── launcher/              # Application launcher module
│   │   │   ├── qmldir
│   │   │   ├── Launcher.qml       # Main launcher component
│   │   │   ├── LauncherGrid.qml   # App grid view
│   │   │   ├── LauncherList.qml   # App list view
│   │   │   ├── LauncherSearch.qml # Search functionality
│   │   │   ├── CategoryView.qml   # Category browsing
│   │   │   ├── FavoritesPanel.qml # Pinned favorites
│   │   │   └── LauncherController.qml # Launch state management
│   │   │
│   │   ├── notifications/         # Notification module
│   │   │   ├── qmldir
│   │   │   ├── NotificationCenter.qml # Notification panel
│   │   │   ├── NotificationPopup.qml    # Toast notifications
│   │   │   ├── NotificationList.qml     # Historical notifications
│   │   │   ├── NotificationSettings.qml # Do not disturb, etc.
│   │   │   └── NotificationController.qml # Notification management
│   │   │
│   │   └── controlcenter/         # Control center module
│   │       ├── qmldir
│   │       ├── ControlCenter.qml  # Main control center panel
│   │       ├── QuickToggles.qml   # Quick settings toggles
│   │       ├── MediaControls.qml  # Media player controls
│   │       ├── SlidersPanel.qml   # Volume/brightness sliders
│   │       ├── ConnectionsPanel.qml # Network/Bluetooth panel
│   │       └── ControlController.qml # Panel state management
│   │
│   ├── popups/                    # Shared popup components
│   │   ├── qmldir
│   │   ├── BasePopup.qml          # Base popup with animations
│   │   ├── ConfirmDialog.qml      # Confirmation dialog
│   │   ├── InputDialog.qml        # Text input dialog
│   │   ├── VolumeOSD.qml          # Volume on-screen display
│   │   ├── BrightnessOSD.qml      # Brightness OSD
│   │   └── ScreenshotPreview.qml  # Screenshot preview
│   │
│   └── animations/                # Reusable animation definitions
│       ├── qmldir
│       ├── FadeAnimation.qml      # Fade in/out
│       ├── SlideAnimation.qml     # Slide transitions
│       ├── ScaleAnimation.qml     # Scale effects
│       └── SpringAnimation.qml    # Physics-based animations
│
└── tests/                         # Test suite
    ├── unit/                      # Unit tests
    ├── integration/               # Integration tests
    └── manual/                    # Manual test scripts
```

---

## QML Module System (qmldir)

The `qmldir` file defines a QML module and its exported types. Each command must be on a separate line.

### qmldir Syntax Reference

```qmldir
# Module identifier (required, must be first line)
module <ModuleIdentifier>

# Object type declaration
[singleton] <TypeName> <InitialVersion> <File>

# Internal type (not exported to module users)
internal <TypeName> <File>

# JavaScript resource
<ResourceIdentifier> <InitialVersion> <File>

# Plugin declaration (C++ integration)
[optional] plugin <Name> [<Path>]

# Plugin class name (for static linking)
classname <C++ plugin class>

# Type description file (for tooling/LSP support)
typeinfo <File>

# Module dependency
depends <ModuleIdentifier> <InitialVersion>

# Module import
import <ModuleIdentifier> [<Version>]

# Designer support flag
designersupported

# Preferred path for compiled resources
prefer <Path>
```

### Complete qmldir Example

```qmldir
# src/services/qmldir
module Services

# Singletons - require pragma Singleton in QML file
singleton AudioService 1.0 AudioService.qml
singleton NetworkService 1.0 NetworkService.qml
singleton PowerManager 1.0 PowerManager.qml

# Regular types
ConfigHelper 1.0 ConfigHelper.qml

# Internal types (used by other types but not exported)
internal PrivateHelper PrivateHelper.qml

# Type info for LSP/tooling support
typeinfo services.qmltypes
```

### Module Structure Pattern

Each feature module follows a consistent internal structure:

```
src/modules/<module-name>/
├── qmldir                    # Module exports
├── <ModuleName>.qml          # Main entry component
├── <ModuleName>Controller.qml # State controller (singleton)
├── <SubComponent>.qml        # Child components
└── internal/                 # Internal/private components
    └── ...
```

### Example: StatusBar Module qmldir

```qmldir
# src/modules/statusbar/qmldir
module StatusBarModule

# Main components
StatusBar 1.0 StatusBar.qml
StatusBarLeft 1.0 StatusBarLeft.qml
StatusBarCenter 1.0 StatusBarCenter.qml
StatusBarRight 1.0 StatusBarRight.qml
Taskbar 1.0 Taskbar.qml

# Singleton controller
singleton BarController 1.0 BarController.qml

# Internal components
internal BarLayout BarLayout.qml
internal WidgetContainer WidgetContainer.qml
```

---

## Singleton Pattern

### Requirements

A QML singleton requires **two steps**:

1. **`pragma Singleton`** at the top of the QML file
2. **`singleton`** keyword in the qmldir file

```qml
// src/services/Theme.qml
pragma Singleton  // Step 1: Declare as singleton
import QtQuick

QtObject {
    id: root

    // Properties are globally accessible
    property color primary: "#89b4fa"
    property color background: "#1e1e2e"
}
```

```qmldir
# src/theme/qmldir
module Theme

# Step 2: Register as singleton in qmldir
singleton Theme 1.0 Theme.qml
```

### Singleton Best Practices

Based on Qt documentation:

1. **Group related data** - Instead of many singletons, use one with multiple properties:
   ```qml
   // Good: One singleton with grouped data
   pragma Singleton
   QtObject {
       property var colors: QtObject { ... }
       property var fonts: QtObject { ... }
       property var spacing: QtObject { ... }
   }
   
   // Avoid: Many separate singletons
   // singleton Colors 1.0 Colors.qml
   // singleton Fonts 1.0 Fonts.qml
   // singleton Spacing 1.0 Spacing.qml
   ```

2. **Singletons stay alive** - They persist until engine destruction, so avoid heavy initialization

3. **Use instantiable types when possible** - If data is only needed in one place, use a regular component instead

4. **Passing initial state** - For one-time setup, prefer `setInitialProperties` over singletons

### Accessing Singletons

```qml
import "../theme"  // Import the module

Rectangle {
    // Direct access by name
    color: Theme.primary
    
    // For bindings on singleton properties, use Binding element
    Binding {
        target: Theme
        property: "currentMode"
        value: "dark"
    }
}
```

---

## Component Hierarchy

Following Atomic Design principles:

```
Atoms (Primitives)
    │
    ├── Button, Icon, Label, Input, etc.
    │
Molecules (Simple Combinations)
    │
    ├── Clock = Label (time) + Label (date)
    ├── VolumeControl = Icon + Slider
    ├── NotificationItem = Icon + Label + Button
    │
Components/Organisms (Complex Combinations)
    │
    ├── AudioController = Card + VolumeControl + MediaButtons
    ├── QuickSettings = Grid + Toggles + Sliders
    │
Modules/Templates (Full Features)
    │
    ├── LockScreen = VideoBackground + Clock + PasswordField + PowerMenu
    ├── StatusBar = Taskbar + Clock + SystemIndicators
    ├── ControlCenter = QuickSettings + MediaControls + Connections
```

---

## Quickshell Integration

### Multi-Monitor Management (`Variants`)

Top-level window objects (like `PanelWindow`, `FloatingWindow`, and `WlSessionLockSurface`) are not visual `Item`s in the QML scene graph. To dynamically instantiate them across multiple displays, use the `Variants` component with the `Quickshell.screens` model.

### State Persistence and Hot-Reload (`Scope`)

To keep background tasks stable during configuration reloads (like IPC Handlers, `Process`, or `Timer` components), place them inside a `Scope`. A `Scope` prevents these objects from being destroyed and recreated on UI reloads. *Do not put visual components inside a `Scope`.*

### Module Imports

**Avoid `root:/` imports.** Always use relative paths (e.g., `import "../services"`). Absolute "root:/..." imports are deprecated and will break Language Server (LSP) auto-completion and cause singletons to misbehave.

### Shell Entry Point (shell.qml)

Quickshell uses `ShellRoot` as the root element. The entry point should delegate to modules, utilizing `Variants` for multi-screen support and `Scope` for state.

```qml
// shell.qml
import Quickshell
import Quickshell.Wayland
import Quickshell.Io
import QtQuick

// Use RELATIVE imports, avoid root:/
import "src/modules/lockscreen"
import "src/modules/statusbar"
import "src/services"

ShellRoot {
    id: root

    // Environment configuration
    readonly property bool autoLock: Quickshell.env("QUICKSHELL_AUTO_LOCK") === "1"
    readonly property string mode: Quickshell.env("QUICKSHELL_MODE", "full")

    // State Persistence - won't restart on UI reload
    Scope {
        IpcHandler {
            target: "qypr"
            function lock(): void { LockController.lock() }
            function unlock(): void { LockController.unlock() }
            function status(): string { return LockController.isLocked ? "locked" : "unlocked" }
        }
    }

    // Session Lock across all screens
    WlSessionLock {
        id: sessionLock
        locked: root.autoLock

        Variants {
            model: Quickshell.screens
            delegate: WlSessionLockSurface {
                // modelData is the screen from Quickshell.screens
                screen: modelData

                LockScreen {
                    anchors.fill: parent
                    onUnlockRequested: sessionLock.locked = false
                }
            }
        }
    }

    // Status Bar across all screens
    Variants {
        model: root.mode === "full" || root.mode === "bar" ? Quickshell.screens : []
        delegate: PanelWindow {
            screen: modelData
            // Status bar configuration...
        }
    }
}
```

### LSP Support

Create an empty `.qmlls.ini` file next to `shell.qml`. Quickshell will auto-manage it:

```bash
touch .qmlls.ini
echo ".qmlls.ini" >> .gitignore
```

### Quickshell Components

| Component | Purpose |
|-----------|---------|
| `ShellRoot` | Root element for all Quickshell configs |
| `PanelWindow` | Wayland/X11 panel (status bar, dock) |
| `FloatingWindow` | Regular desktop window |
| `WlSessionLock` | Wayland session lock (secure lockscreen) |
| `WlSessionLockSurface` | Per-screen lock surface |
| `Variants` | dynamic instantiation of non-visual/window elements |
| `Scope` | persist state and logic across hot-reloads |
| `IpcHandler` | Inter-process communication |
| `Process` | Run external commands |

---

## Communication Patterns

### 1. Service-to-UI Communication

Use `Connections` to listen to service signals:

```qml
Connections {
    target: NotificationService
    function onNotificationReceived(notification) {
        // Handle new notification
    }
}
```

### 2. Cross-Module Communication

Use signals and shared services; avoid direct coupling:

```qml
// Module A emits signal
ControlController.panelOpened()

// Module B listens via Connections
Connections {
    target: ControlController
    function onPanelOpened() {
        LauncherController.hide()
    }
}
```

### 3. Parent-Child Communication

```qml
// Parent to child: property binding
AudioController {
    revealed: root.uiRevealed
}

// Child to parent: signal
AudioController {
    signal mediaControlsClicked(string action)
    onMediaControlsClicked: root.handleMediaAction(action)
}
```

---

## Entry Point Architecture

### Main Shell (shell.qml)

When integrating multiple UI modules and keeping state intact across reloads, separate logical controllers using `Scope` and map window components using `Variants`.

```qml
import Quickshell
import Quickshell.Wayland
import Quickshell.Io
import QtQuick

// Relies on explicit relative imports
import "src/modules/lockscreen"
import "src/modules/statusbar"
import "src/modules/launcher"
import "src/modules/notifications"
import "src/modules/controlcenter"
import "src/services"

ShellRoot {
    id: root

    // Configuration: which modules to enable
    readonly property var enabledModules: [
        "lockscreen",
        "statusbar",
        "launcher",
        "notifications",
        "controlcenter"
    ]

    // IPC Handler in a Scope to survive reloads
    Scope {
        IpcHandler {
            target: "qypr"
            
            function showModule(module: string): void { /* ... */ }
            function hideModule(module: string): void { /* ... */ }
            function toggleModule(module: string): void { /* ... */ }
        }
    }

    // Wrap windows inside Variants to spawn on every available screen
    Variants {
        model: enabledModules.includes("statusbar") ? Quickshell.screens : []
        delegate: PanelWindow {
            screen: modelData
            StatusBar { 
                anchors.fill: parent
            }
        }
    }

    // Some modules might be FloatingWindows initialized as needed
    Loader {
        id: launcherLoader
        active: enabledModules.includes("launcher") && LauncherController.isVisible
        sourceComponent: FloatingWindow {
             Launcher { anchors.fill: parent }
        }
    }

    // ... other loaders or variants
}
```

---

## Styling System

### Theme Engine

The theme system supports:
- **CSS-like tokens**: colors, spacing, typography, effects
- **Dark/Light variants**: Automatic or manual switching
- **Custom themes**: JSON-based theme definitions
- **Runtime switching**: Hot-swap themes without restart

### Theme Structure

```qml
// src/theme/Theme.qml
pragma Singleton
import QtQuick

QtObject {
    id: root

    // Current theme variant
    property string currentTheme: "catppuccin-mocha"
    property bool isDark: true

    // Grouped theme tokens
    property var colors: QtObject {
        readonly property color background: "#1e1e2e"
        readonly property color surface: "#313244"
        readonly property color primary: "#89b4fa"
        readonly property color text: "#cdd6f4"
        // Glassmorphism
        readonly property color glass: "#40313244"
        readonly property color glassBorder: "#30cdd6f4"
    }

    property var fonts: QtObject {
        readonly property int textSize: 16
        readonly property int textSizeLarge: 24
        readonly property string fontFamily: "Inter"
    }

    property var spacing: QtObject {
        readonly property int small: 8
        readonly property int medium: 16
        readonly property int large: 24
    }

    property var radius: QtObject {
        readonly property int small: 4
        readonly property int medium: 8
        readonly property int large: 16
    }

    property var animation: QtObject {
        readonly property int fast: 150
        readonly property int medium: 300
        readonly property int slow: 500
    }
}
```

### Usage in Components

```qml
import "../theme"

Rectangle {
    color: Theme.colors.surface
    radius: Theme.radius.medium

    Behavior on color {
        ColorAnimation { duration: Theme.animation.medium }
    }
}
```

---

## Best Practices (Qt6/QML Official)

### Type Safety

**Always use explicit types** instead of `var`:

```qml
// Good: Explicit types
property string name: ""
property int count: 0
property bool isActive: false
property color backgroundColor: "#ffffff"
property Item container: null

// Bad: var types (loses type checking)
property var name
property var count
property var isActive
```

Benefits:
- Errors point to assignment location, not declaration
- Static analysis can catch type errors
- Code is more readable

### Declarative Bindings

**Prefer declarative bindings** over imperative assignments:

```qml
// Good: Declarative binding
Rectangle {
    color: mouseArea.containsMouse ? "blue" : "red"
}

// Bad: Imperative assignment
Rectangle {
    MouseArea {
        onClicked: parent.color = "blue"  // Breaks bindings!
    }
}
```

Issues with imperative assignments:
- Slower (evaluates twice: default then assigned)
- Delays build-time errors to runtime
- Overwrites declarative bindings
- Breaks tooling support

### Property Change Signals

**Use explicit interaction signals** instead of `onChanged` signals:

```qml
// Good: Use interaction signal
Slider {
    onMoved: pushToBackend(value)  // Only on user interaction
}

// Bad: Use valueChanged
Slider {
    onValueChanged: pushToBackend(value)  // Also fires on clamping/rounding
}
```

### State Storage

**Never store state in delegates** (ListView, Repeater items):

```qml
// Bad: State lost when delegate is destroyed
ListView {
    delegate: Item {
        property bool expanded: false  // Lost on scroll!
        onClicked: expanded = true
    }
}

// Good: Store state in model
ListView {
    delegate: Item {
        onClicked: model.expanded = true
    }
}
```

### Layout Usage

When using Qt Quick Layouts:

```qml
RowLayout {
    anchors.fill: parent
    
    Rectangle {
        // Good: Use Layout attached properties
        Layout.fillWidth: true
        Layout.minimumWidth: 50
        Layout.preferredWidth: 100
        
        // Bad: Don't use anchors in layout children
        // anchors.left: parent.left
    }
}
```

- **Do**: Use `Layout.fillWidth`, `Layout.preferredWidth`, etc.
- **Don't**: Use anchors on immediate children of layouts

### Scalable UI

For high-DPI support:

1. Use anchors or layouts for positioning
2. Don't specify explicit width/height
3. Provide `@2x`, `@3x`, `@4x` image variants
4. Use SVG for small icons
5. Use font-based icons (Font Awesome) for scalability

---

## Performance Guidelines

### Efficient Loading

1. **Lazy load modules** with `Loader`:
   ```qml
   Loader {
       active: false  // Set true when needed
       sourceComponent: HeavyComponent { }
   }
   ```

2. **Limit complex bindings** - They re-evaluate on every dependency change

3. **Avoid creating objects in loops** - Pre-create or use models

4. **Profile regularly** - Use Qt Creator profiler

### Component Guidelines

1. **Single responsibility**: One component = one purpose
2. **Configurable via props**: Avoid hardcoded values
3. **Document public API**: Props, signals, methods
4. **Use implicit sizing**: Let content determine size
5. **Handle edge cases**: Empty states, loading states, errors

---

## Extension Points

### Adding a New Module

1. Create module directory under `src/modules/`
2. Define `qmldir` with exports
3. Create main component and controller
4. Add to `shell.qml` module loading
5. Update documentation

### Adding a New Component

1. Identify level: atom, molecule, or organism
2. Create in appropriate directory
3. Export in `qmldir`
4. Document props/signals

### Adding a New Service

1. Create in `src/services/`
2. Add `pragma Singleton` at top
3. Add to `qmldir` with `singleton` keyword
4. Implement signal/property pattern
5. Consider grouping with existing singletons

---

## Build and Run

### Development Mode

```bash
# Run full shell
qs -p .

# Run specific module
QUICKSHELL_MODE=bar qs -p .

# Development with auto-reload (file watcher)
# Quickshell auto-reloads on file save
```

### Production Mode

```bash
# Lockscreen
QUICKSHELL_AUTO_LOCK=1 qs -p .

# Full desktop shell
qs -p .
```

---

## Migration from Current Structure

### Current → Target Mapping

| Current Path | New Path |
|--------------|----------|
| `shell.qml` | `shell.qml` (expanded) |
| `src/services/Theme.qml` | `src/theme/Theme.qml` |
| `src/services/LockController.qml` | `src/modules/lockscreen/LockController.qml` |
| `src/services/PowerManager.qml` | `src/services/PowerManager.qml` |
| `src/services/VideoConfig.qml` | `src/modules/lockscreen/VideoConfig.qml` |
| `src/components/Clock.qml` | `src/molecules/Clock.qml` |
| `src/components/PasswordField.qml` | `src/components/PasswordField.qml` |
| `src/widgets/LockScreen.qml` | `src/modules/lockscreen/LockScreen.qml` |
| `src/audio/AudioService.qml` | `src/services/AudioService.qml` |
| `src/components/Audio*.qml` | `src/components/Audio*.qml` |

---

## References

- [Qt6 QML Best Practices](https://doc.qt.io/qt-6/qtquick-bestpractices.html) - Official Qt guidelines
- [Singletons in QML](https://doc.qt.io/qt-6/qml-singleton.html) - Singleton pattern documentation
- [Module Definition qmldir Files](https://doc.qt.io/qt-6/qtqml-modules-qmldir.html) - qmldir syntax reference
- [Quickshell Documentation](https://quickshell.org/) - Quickshell guides and API
- [Atomic Design Methodology](https://atomicdesign.bradfrost.com/) - Component hierarchy approach

---

## Changelog

| Version | Date | Changes |
|---------|------|---------|
| 1.1.0 | 2026-04-12 | Added Qt6/QML official best practices, qmldir syntax reference, singleton pattern details, Quickshell integration guide, LSP support, type safety guidelines |
| 1.0.0 | 2025-04-12 | Initial documentation |