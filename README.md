# Quickshell Lockscreen

A modern, customizable lockscreen for Hyprland on Arch Linux, built with Quickshell and QML.

## Features

- **Secure Session Locking**: Uses `ext_session_lock_v1` Wayland protocol for maximum security
- **PAM Authentication**: Integrates with system PAM for password verification
- **Hot Reloading**: Changes reflect immediately during development
- **Modular Architecture**: Clean separation of services, components, and widgets
- **Theme System**: Centralized theming using Catppuccin Mocha color palette
- **IPC Interface**: External status and lock control via command-line IPC
- **Power Management**: System suspend, reboot, and shutdown controls

## Preview

The lockscreen features:
- Large centered clock with date display
- Password input field with focus indication
- Authentication status messages
- Power action buttons (Suspend, Reboot, Shutdown)

## Requirements

### System Requirements
- Arch Linux (or compatible distribution)
- Hyprland compositor (or other Wayland compositor with `ext_session_lock_v1` support)
- PAM authentication system

### Dependencies

All dependencies should already be installed on your Hyprland setup:

```bash
# Core
quickshell           # Main framework (v0.2.1+)
qt6-base             # Qt6 base libraries
qt6-declarative      # QML runtime

# Wayland support
qt6-wayland          # Wayland platform plugin

# Optional (for enhanced features)
qt6-svg              # SVG support
qt6-multimedia       # Audio/video support
qt6-multimedia-ffmpeg # FFmpeg backend
qt6-shadertools      # Shader effects
qt6-5compat          # Legacy compatibility

# System
systemd              # For power management
pam                  # Authentication
```

Verify installation:
```bash
pacman -Qi quickshell qt6-base qt6-declarative qt6-wayland
```

## Project Structure

```
lockscreen/
├── shell.qml                    # Main entry point
├── run.sh                       # Development launcher
├── lock.sh                      # Production launcher
├── test.sh                      # Test runner
├── README.md                    # This file
├── src/
│   ├── services/                # Business logic singletons
│   │   ├── LockController.qml   # PAM authentication handler
│   │   ├── PowerManager.qml     # System power actions
│   │   └── Theme.qml            # Color palette and constants
│   ├── components/              # Reusable UI components
│   │   ├── Clock.qml            # Time and date display
│   │   ├── PasswordField.qml    # Password input field
│   │   ├── ActionButton.qml     # Power action button
│   │   └── StatusMessage.qml    # Authentication status
│   └── widgets/                 # Composite widgets
│       └── LockScreen.qml       # Main lockscreen UI
└── tests/
    └── manual_test.sh           # Manual test scripts
```

## Architecture

### Services Layer

Services are QML Singletons that handle business logic:

- **LockController**: Manages PAM authentication flow
  - `authenticate(password)`: Start authentication
  - `lock()`: Lock the session
  - Signals: `unlockSuccess`, `unlockFailed`

- **PowerManager**: Handles system power actions
  - `suspend()`: Suspend to RAM
  - `reboot()`: Restart system
  - `shutdown()`: Power off system
  - `hibernate()`: Suspend to disk

- **Theme**: Centralized styling constants
  - `colors`: Catppuccin Mocha palette
  - `fonts`: Typography settings
  - `spacing`: Layout constants
  - `radius`: Border radius values

### Components Layer

Reusable UI components with no business logic:

- **Clock**: Displays time and date using `SystemClock`
- **PasswordField**: Password input with focus states
- **ActionButton**: Clickable button with hover effects
- **StatusMessage**: Displays auth status (success/error)

### Widget Layer

Composite widgets that combine components and services:

- **LockScreen**: Main UI composing all components
  - Connects to `LockController` for auth state
  - Handles user input and feedback
  - Manages focus and keyboard events

### Entry Point

`shell.qml` wires everything together:
- Creates `WlSessionLock` for secure session locking
- Instantiates `LockScreen` for each display
- Registers IPC handlers for external control

## Installation

### Clone/Download

```bash
cd ~/Coding
git clone <your-repo> lockscreen
# or just use the current directory
```

### Verify Structure

```bash
cd lockscreen
./test.sh
```

Expected output:
```
=== File Structure Tests ===
✓ PASS: Main shell.qml exists
✓ PASS: LockController service exists
...
=== Summary ===
Passed: 13
Failed: 0
All tests passed!
```

## Usage

### Development Mode

Run without locking (for testing UI):

```bash
./run.sh
```

Features:
- Loads lockscreen UI
- Does NOT lock the session
- Hot reload on file changes
- Password field accepts input but won't lock you out

### Lock Mode

Run with session locking:

```bash
./lock.sh
```

This will:
1. Reuse the running lockscreen instance if one already exists
2. Otherwise start a background Quickshell instance with lock enabled
3. Lock your session immediately via `WlSessionLock`
4. Show the lockscreen on all displays
5. Require PAM authentication to unlock

**Important**: The lockscreen must be run from within a Wayland compositor session (e.g., Hyprland) that supports the `ext_session_lock_v1` protocol. Running from a TTY or unsupported compositor will cause a `wl_display` error.

### IPC Control

Control the lockscreen from command line:

```bash
# Check status
qs ipc -p /home/arch/Coding/lockscreen show
qs ipc -p /home/arch/Coding/lockscreen call lockscreen status
# Output: "unlocked", "locking", or "locked"

# Lock the screen
qs ipc -p /home/arch/Coding/lockscreen call lockscreen lock

# Stop the background instance when you are done testing
qs kill -p /home/arch/Coding/lockscreen
```

`unlock` and `toggle` are intentionally not exposed over IPC because they would bypass PAM and weaken the lockscreen.

### Hyprland Integration

Add to `~/.config/hypr/hyprland.conf`:

```ini
# Bind SUPER+L to lock screen
bind = SUPER, L, exec, /home/arch/Coding/lockscreen/lock.sh

# Auto-lock after timeout (optional)
exec-once = xidle-handler --timeout 300 "/home/arch/Coding/lockscreen/lock.sh"
```

Disable `hyprlock` if you want to use this exclusively:

```ini
# Comment out or remove
# exec-once = hyprlock
```

## Customization

### Theme Colors

Edit `src/services/Theme.qml`:

```qml
property var colors: QtObject {
    readonly property color background: "#1e1e2e"    // Main background
    readonly property color surface: "#313244"      // Input background
    readonly property color surfaceHover: "#45475a" // Hover states
    readonly property color primary: "#89b4fa"      // Accent color
    readonly property color text: "#cdd6f4"         // Primary text
    readonly property color textSubtle: "#a6adc8"   // Secondary text
    readonly property color error: "#f38ba8"        // Error states
}
```

### Clock Format

Edit `src/components/Clock.qml`:

```qml
property string timeFormat: "HH:mm"         // 24-hour format
property string timeFormat: "h:mm AP"       // 12-hour format

property string dateFormat: "dddd, MMMM d"  // Full day name
property string dateFormat: "MM/dd/yyyy"    // Numeric format
```

### Background

Add a background image to `src/widgets/LockScreen.qml`:

```qml
Image {
    anchors.fill: parent
    source: "../../assets/background.jpg"
    fillMode: Image.PreserveAspectCrop
}

// Add blur effect
FastBlur {
    anchors.fill: parent
    source: backgroundImage
    radius: 50
}
```

### Additional Widgets

Create new components in `src/components/`:

```qml
// src/components/BatteryStatus.qml
import QtQuick
import "../services"

Row {
    spacing: Theme.spacing.small
    
    Text {
        text: "Battery: "
        color: Theme.colors.textSubtle
    }
    
    Text {
        text: "85%"
        color: Theme.colors.text
    }
}
```

Then import in `LockScreen.qml`:

```qml
import "../components"

// Add to layout
BatteryStatus {
    Layout.alignment: Qt.AlignHCenter
}
```

## Testing

### Automated Tests

Run the test suite:

```bash
./test.sh
```

Tests:
- File structure integrity
- Dependency availability
- Service/component existence

### Manual Testing

Run specific test scenarios:

```bash
# Test lockscreen loading
./tests/manual_test.sh load

# Test power commands
./tests/manual_test.sh power

# Test IPC interface
./tests/manual_test.sh ipc

# Run all manual tests
./tests/manual_test.sh all
```

### Testing Checklist

1. **UI Loading**
   - [ ] Lockscreen renders without errors
   - [ ] Clock displays current time
   - [ ] Password field accepts input
   - [ ] Buttons are clickable

2. **Authentication**
   - [ ] Correct password unlocks screen
   - [ ] Incorrect password shows error
   - [ ] Error message clears on retry
   - [ ] Password field clears after attempt

3. **Power Actions**
   - [ ] Suspend button works (test with caution)
   - [ ] Reboot button works (test with caution)
   - [ ] Shutdown button works (test with caution)

4. **Multi-Monitor**
   - [ ] Lockscreen appears on all displays
   - [ ] Each display shows the UI correctly
   - [ ] Unlocking works from any display

5. **IPC**
   - [ ] `qs ipc -p /home/arch/Coding/lockscreen show` lists the `lockscreen` target
   - [ ] `status` command reports `unlocked`, `locking`, or `locked`
   - [ ] `lock` command locks screen

## Security

### WlSessionLock Protocol

Uses the `ext_session_lock_v1` Wayland protocol for secure session locking:

- **Compositor-Level Security**: Locking happens at compositor level
- **Crash Protection**: If quickshell crashes, screen stays locked
- **Solid Color Fallback**: Failed locks show solid color, not your session
- **Multi-Monitor**: All displays are covered simultaneously

### PAM Integration

Uses system PAM for authentication:

- **Default Config**: Uses `/etc/pam.d/login` by default
- **Secure Password Handling**: Passwords never stored in memory longer than needed
- **Custom Config**: Can use custom PAM configs if needed

### Important Notes

1. **Crash Behavior**: If quickshell exits while locked, your session stays locked (shows solid color). This is intentional security behavior.

2. **Password Storage**: Passwords are handled by PAM and never logged or stored.

3. **File Permissions**: Ensure your config files are readable only by your user:
   ```bash
   chmod 700 ~/Coding/lockscreen
   chmod 600 ~/Coding/lockscreen/shell.qml
   ```

## Troubleshooting

### Common Issues

**Lockscreen doesn't appear**
```bash
# Check if compositor supports ext_session_lock_v1
hyprctl version

# Check quickshell logs
qs -p /home/arch/Coding/lockscreen 2>&1 | less
```

**Authentication fails immediately**
```bash
# Check PAM config
cat /etc/pam.d/login

# Test PAM manually
su - $USER
```

**Qt version warning**
```
WARN: Quickshell was built against Qt 6.10.2 but system has 6.11.0
```
This is a known Arch issue. Usually works fine. If crashes occur:
```bash
sudo pacman -S quickshell
```

**Hot reload not working**
```bash
# Quickshell auto-reloads on file save
# Make sure you're editing files in the correct location
pwd  # Should be /home/arch/Coding/lockscreen
```

### Debug Mode

Run with verbose output:

```bash
qs -p /home/arch/Coding/lockscreen 2>&1 | tee debug.log
```

### Check Logs

```bash
# Recent logs
ls -lt /run/user/$UID/quickshell/by-id/ | head -5

# View latest log
cat /run/user/$UID/quickshell/by-id/*/log.qslog | tail -50
```

## Contributing

### Code Style

- Follow QML naming conventions
- Components: PascalCase (e.g., `PasswordField.qml`)
- Properties: camelCase (e.g., `placeholderText`)
- Use `pragma Singleton` for services
- Import relative paths for components

### Adding Features

1. Create component in `src/components/`
2. Add business logic to service in `src/services/`
3. Integrate in `src/widgets/LockScreen.qml`
4. Update tests
5. Update documentation

### Testing Changes

```bash
# Run tests
./test.sh

# Visual test
./run.sh
```

## Resources

### Documentation

- [Quickshell Docs](https://quickshell.org/docs/)
- [WlSessionLock Reference](https://quickshell.org/docs/types/Quickshell.Wayland/WlSessionLock/)
- [PamContext Reference](https://quickshell.org/docs/types/Quickshell.Services.Pam/PamContext/)
- [Qt QML Reference](https://doc.qt.io/qt-6/qtquick-qmlmodule.html)

### Related Projects

- [Noctalia](https://github.com/nic Bliss/nic-bliss/nic-bliss/nic-bliss/nic-bliss/nic-bliss/nic) - Full shell with lockscreen
- [Qylock](https://github.com/qyminu/qylock) - SDDM themes and lockscreens

### Community

- [Quickshell Matrix](https://matrix.to/#/#quickshell:outfoxxed.me)
- [Quickshell Discord](https://discord.gg/UtZeT3xNyT)

## License

MIT License - Feel free to use and modify.

## Credits

- Built with [Quickshell](https://quickshell.org/)
- Color scheme: [Catppuccin Mocha](https://github.com/catppuccin/catppuccin)
- Inspired by modern lockscreen designs
