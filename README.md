# Quickshell Lockscreen

A modern, high-performance, and visually stunning lockscreen for Hyprland on Arch Linux. Built with Quickshell and QML, featuring dynamic video backgrounds and a glassmorphic user interface.

## Highlights

- **Dynamic Video Backgrounds**: Seamless cross-fading video backgrounds inspired by the Aerial SDDM theme.
- **Time-Aware Playlists**: Automatically switches between day and night video playlists (6:30 AM - 6:30 PM).
- **Glassmorphic UI**: Ultra-modern aesthetic using backdrop blur, semi-transparent surfaces, and Catppuccin Mocha colors.
- **Audio Control**: Integrated MPRIS-based media player with play/pause controls and metadata display.
- **Reveal-on-Interaction**: Minimalist idle state (Clock & Power only) that reveals the authentication UI upon movement or interaction.
- **Secure**: Implements the `ext_session_lock_v1` protocol and system PAM authentication.

## Preview

The lockscreen features:
- **Idle View**: Large floating clock with date and a subtle glassmorphic power button.
- **Interaction View**: Smoothly fades in a pill-shaped password field, expanded power options, and audio controls.
- **Visuals**: Continuous video playback with 2-second crossfade transitions between clips.

## Requirements

### Dependencies

Ensure these are installed (Arch Linux):

```bash
# Core framework
sudo pacman -S quickshell

# Qt6 Dependencies
sudo pacman -S qt6-base qt6-declarative qt6-wayland qt6-shadertools

# Media Support (Required for Video Background)
sudo pacman -S qt6-multimedia qt6-multimedia-ffmpeg
```

## Project Structure

```
lockscreen/
├── shell.qml                    # Main entry point
├── run.sh                       # Development launcher (no lock)
├── lock.sh                      # Production launcher (full lock)
├── test.sh                      # Test suite runner
├── playlists/                   # M3U playlists for day/night videos
├── videos/                      # Symlink to video assets
├── tests/
│   └── manual_audio_test.sh     # Manual audio testing script
├── src/
│   ├── audio/
│   │   ├── AudioService.qml      # MPRIS player model wrapper
│   │   └── qmldir                # QML module definition
│   ├── services/                # Logic Singletons
│   │   ├── LockController.qml   # PAM Auth management
│   │   ├── VideoConfig.qml      # Playlist & Timing config
│   │   ├── PowerManager.qml     # System actions
│   │   ├── AudioService.qml     # Audio service singleton
│   │   └── Theme.qml            # Modern styling tokens (Glassmorphism & Audio)
│   ├── components/              # UI Building Blocks
│   │   ├── VideoBackground.qml  # Dual-MediaPlayer engine
│   │   ├── Clock.qml            # Typography-focused clock
│   │   ├── PasswordField.qml    # Reactive input field
│   │   ├── ActionButton.qml     # Animated power icons
│   │   ├── AudioController.qml  # Audio player controls
│   │   ├── AudioMetadata.qml    # Media metadata display
│   │   ├── AudioPlayerButton.qml # Play/pause button
│   │   └── qmldir               # QML module definition
│   └── widgets/
│       └── LockScreen.qml       # Main layout & interaction logic (with audio)
```

## Configuration

### Video Playlists

The background system looks for M3U files in `playlists/day.m3u` and `playlists/night.m3u`. Video files should be located in (or symlinked to) the `videos/` directory.

### UI Customization

The design system is centralized in `src/services/Theme.qml`. You can adjust:
- **Blur Strength**: Modify backdrop blur intensity.
- **Glass Opacity**: Control the transparency of UI elements.
- **Reveal Timing**: Adjust how fast the UI appears on interaction.

### Time-of-Day Logic

Edit `src/services/VideoConfig.qml` to change sunrise/sunset hours or crossfade speed:

```qml
readonly property int dayStartHour: 6
readonly property int dayStartMinute: 30
readonly property int nightStartHour: 18
readonly property int nightStartMinute: 30
readonly property int crossfadeDuration: 2000 // ms
```

## Usage

### Development Mode
Test the UI without locking yourself out:
```bash
./run.sh
```

### Locking the Screen
Trigger the actual session lock:
```bash
./lock.sh
```

### IPC Interface
Status checks and manual control:
```bash
# Get lock status
qs ipc -p . call lockscreen status
# Force lock
qs ipc -p . call lockscreen lock
```

## Testing

Run the automated test suite:
```bash
./test.sh
```

For manual audio testing:
```bash
./tests/manual_audio_test.sh
```

## Hyprland Setup

Add the following to your `hyprland.conf`:

```ini
# Bind lock key
bind = SUPER, L, exec, /path/to/lockscreen/lock.sh

# Idle handling (e.g., with hypridle)
# exec-once = hypridle
```

## Security & Reliability

- **Persistence**: If the UI process crashes, the `WlSessionLock` protocol ensures the screen remains blocked by a fallback color until the process is restarted.
- **PAM Integration**: Uses standard system authentication. No passwords are stored locally or in memory beyond the auth attempt.

## License
MIT
