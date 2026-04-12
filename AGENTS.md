# Qypr - Agent Instructions

## Project Overview

Qypr is a modular desktop shell system for Hyprland/Wayland built with Quickshell and Qt6/QML. It currently supports a lockscreen with video backgrounds and is being extended to support status-bar, launcher, notifications, and control-center modules.

## Architecture

### Design Patterns
- **Atomic Design**: Components organized as atoms → molecules → organisms → modules
- **Singleton Services**: Business logic in QML singletons (`pragma Singleton`)
- **Modular Architecture**: Each feature is a self-contained module
- **Signal-Based Communication**: Loose coupling via signals/slots

### Key Directories

```
src/
├── atoms/           # Primitive UI components (Button, Icon, Label)
├── molecules/       # Simple combinations (Clock, VolumeControl)
├── components/      # Complex components (AudioController, GlassPanel)
├── modules/         # Feature modules
│   ├── lockscreen/  # Lock screen with PAM auth
│   ├── statusbar/   # Top panel with widgets
│   ├── launcher/    # App launcher
│   ├── notifications/ # Notification center
│   └── controlcenter/ # Quick settings panel
├── services/        # Singleton business logic
├── models/          # Data models
├── theme/           # Theme system
└── core/            # Utilities
```

## Coding Standards

### QML Conventions

```qml
// Use explicit types, not 'var'
property string name: ""
property int count: 0
property bool isActive: false

// Prefer declarative bindings
// Good:
color: Theme.colors.primary

// Bad:
Component.onCompleted: { color = Theme.colors.primary }

// Use proper indentation (4 spaces)
// Document with header comments
```

### Component Template

```qml
// ComponentName.qml - Brief description
//
// Detailed description of purpose and usage.
//
// Properties:
//   - propertyName: Description
//
// Signals:
//   - signalName: Description

import QtQuick
import "../services" as Services
import "../theme" as Theme

Item {
    id: root

    // Public API
    property string title: ""
    signal clicked()

    // Private
    property bool _isHovered: false

    // Implementation...
}
```

### Module Definition (qmldir)

```
module ModuleName
ComponentName 1.0 ComponentName.qml
singleton ServiceName 1.0 ServiceName.qml
```

## Services Pattern

All services are singletons in `src/services/`:

```qml
// src/services/MyService.qml
pragma Singleton
import QtQuick

QtObject {
    id: root

    // Signals for state changes
    signal stateChanged

    // Properties with change notification
    property bool isActive: false

    // Public methods
    function doSomething() { ... }
}
```

## Current State

### Implemented
- ✅ Lockscreen with video backgrounds
- ✅ Audio/MRPIS controls
- ✅ PAM authentication
- ✅ Glassmorphic UI (Catppuccin Mocha)

### Planned
- 🔄 Status bar module
- 🔄 Application launcher
- 🔄 Notification center
- 🔄 Control center
- 🔄 Theme system

## Testing

```bash
# Run tests
./test.sh

# Manual test
./tests/manual_audio_test.sh

# Development mode
./run.sh
```

## Resources

- **Project Structure**: `docs/PROJECT_STRUCTURE.md`
- **Qt6 QML**: https://doc.qt.io/qt-6/qtquick-bestpractices.html
- **Quickshell**: https://quickshell.org/

## Common Tasks

### Adding a Component
1. Identify level: atom, molecule, or component
2. Create in appropriate directory
3. Export in `qmldir`
4. Document properties and signals

### Adding a Module
1. Create directory in `src/modules/`
2. Create `qmldir` with exports
3. Implement main component and controller
4. Add loader in `shell.qml`

### Modifying Services
1. Service must remain singleton
2. Maintain backward compatibility
3. Update all consumers
4. Test module interactions

## Notes

- Use Catppuccin Mocha palette for UI consistency
- Maintain glassmorphic aesthetic (blur, transparency)
- Follow atomic design principles
- Keep modules independent and loosely coupled
