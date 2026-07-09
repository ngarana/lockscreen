# Qypr Implementation Roadmap

## Overview

This roadmap outlines the step-by-step implementation plan for transforming Qypr from a lockscreen-only application to a comprehensive desktop shell system. The plan is organized into phases, each building upon previous work while maintaining system stability.

**Current State**: Lockscreen module with video backgrounds, audio/MPRIS controls, and PAM authentication (Catppuccin Mocha glassmorphic UI). Foundation and infrastructure complete. Shell loads cleanly with zero errors.

**Target State**: Full desktop shell with status bar, application launcher, notification center, and control center.

**Last Updated**: 2026-07-08
**Version**: 1.4
**Status**: Phase 0-4, Phase 10 & Phase 13 Complete ✅ — Ready for Phase 5 (Application Launcher)

---

## Implementation Progress

| Phase | Status | Completion | Notes |
|-------|--------|------------|-------|
| **0: Foundation** | ✅ Done | 100% | Theme system, core utilities, shell.qml, lockscreen module migrated |
| **1: Services** | ✅ Done | 100% | 11 services implemented with full functionality |
| **2: Atomic Components** | ✅ Done | 100% | 13 atoms + 14 molecules + 5 layouts + 4 animations complete (36 total) |
| **3: Composite Components** | ✅ Done | 100% | GlassPanel, Calendar, WeatherWidget, SystemMonitor, QuickSettings created |
| **4: Status Bar** | ✅ Done | 100% | macOS-style status bar with launcher, workspaces, clock, tray, indicators |
| **5: Launcher** | ⏳ Pending | 0% | |
| **6: Notifications** | ⏳ Pending | 0% | |
| **7: Control Center** | ⏳ Pending | 0% | |
| **8: Popups & OSDs** | ⏳ Pending | 0% | |
| **9: Models** | ⏳ Pending | 0% | |
| **10: Scripts & Tooling** | ✅ Done | 100% | All scripts created, theme JSON definitions, comprehensive test suite |
| **11: Testing** | ✅ Done | 70% | Infrastructure test suite ready (126 tests passing) |
| **12: Polish** | ⏳ Pending | 0% | |
| **13: Standalone Locker** | ✅ Done | 100% | Standalone entry point, duplicate consolidation, test updates |

---

## Phase 0: Foundation & Infrastructure

**Goal**: Establish project structure, theme system, and core services that all modules will depend on.

### 0.1 Project Restructuring
- [x] Create directory structure per PROJECT_STRUCTURE.md
- [x] Move existing lockscreen code into `src/modules/lockscreen/`
- [x] Create `qmldir` files for each module/directory
- [x] Update all imports to use relative paths (avoid `root:/`)
- [x] Create `.qmlls.ini` for LSP support

### 0.2 Theme System Implementation
- [x] Create `src/theme/ThemeEngine.qml` (singleton)
- [x] Create `src/theme/ColorPalette.qml`
- [x] Create `src/theme/Typography.qml`
- [x] Create `src/theme/Spacing.qml`
- [x] Create `src/theme/Effects.qml`
- [x] Create `src/theme/AnimationTokens.qml` (renamed from Animation.qml to avoid QtQuick conflict)
- [x] Create theme qmldir with singleton registration
- [x] Create `themes/default/theme.json` (Catppuccin Mocha)
- [x] Create `themes/catppuccin-latte/theme.json`

### 0.3 Core Utilities
- [x] Create `src/core/Constants.qml`
- [x] Create `src/core/Logger.qml`
- [x] Create `src/core/Utils.qml` (singleton)
- [x] Create `src/core/Errors.qml`
- [x] Create core qmldir

### 0.4 Shell Entry Point Refactoring
- [x] Refactor `shell.qml` to use modular architecture
  - Uses `ShellRoot` as root
  - Uses `WlSessionLockSurface` directly (compositor auto-creates per screen)
  - Uses `Scope` for IPC handlers and state persistence
  - Logger configuration in Scope survives hot-reloads
- [x] Create `scripts/` directory with all launcher scripts
- [x] Update backward-compatible wrapper scripts at root
- [x] Test lockscreen functionality (loads cleanly with zero errors)

**Deliverables**: 
- ✅ Complete project structure
- ✅ Working theme system
- ✅ Core utilities
- ✅ Refactored shell.qml
- ✅ All existing tests pass

---

## Phase 1: Services Layer

**Goal**: Implement all business logic services that modules will consume.

### 1.1 Configuration Service ✅
- [x] Create `src/services/ConfigService.qml` (singleton)
- Load/save user preferences (JSON config file)
- Default configuration values
- Config change signals
- Schema validation

### 1.2 Audio & Media Services ✅
- [x] Refactor existing audio code into `src/services/AudioService.qml` (singleton)
- MPRIS media player integration via playerctl
- Media metadata extraction (title, artist, album)
- Playback controls (play, pause, next, previous, stop, seek)
- Volume control
- Multiple player support with active player selection
- Signal: `playbackStateChanged`, `metadataChanged`, `positionChanged`, `playerChanged`, `volumeChanged`, `playersChanged`

### 1.3 Network & Connectivity Services ✅
- [x] Create `src/services/NetworkService.qml` (singleton)
  - WiFi status monitoring via nmcli
  - Network name (SSID) retrieval
  - Signal strength indication (0-100)
  - Connection state changes (WiFi/Ethernet)
  - Internet connectivity checking
  - Available networks scanning
  - Signal: `networkStatusChanged`, `signalStrengthChanged`, `availableNetworksChanged`
- [x] Create `src/services/BluetoothService.qml` (singleton)
  - Bluetooth adapter status monitoring
  - Paired devices list management
  - Device connection/disconnection
  - Device discovery support
  - Battery level reporting for devices
  - Signal: `bluetoothStatusChanged`, `deviceConnected`, `deviceDisconnected`, `pairedDevicesChanged`

### 1.4 Power & Display Services ✅
- [x] Create `src/services/BatteryService.qml` (singleton)
  - Battery percentage monitoring (0-100)
  - Charging status detection
  - Time remaining estimation (formatted)
  - Multiple battery support with combined percentage
  - Battery level categories (critical, low, medium, high, full)
  - Low/critical battery warnings
  - Signal: `batteryLevelChanged`, `chargingStatusChanged`, `batteryLow`, `batteryCritical`
- [x] Create `src/services/BrightnessService.qml` (singleton)
  - Display brightness control via brightnessctl/xrandr
  - Automatic control method detection
  - Multiple display support
  - Step-based brightness adjustment
  - Signal: `brightnessChanged`, `displayBrightnessChanged`, `availabilityChanged`
- [x] Create `src/services/PowerManager.qml` (singleton)
  - System power actions via systemctl
  - Shutdown, reboot, suspend, hibernate
  - Signal: `actionTriggered`, `actionFailed`

### 1.5 System Integration Services ✅
- [x] Create `src/services/HyprlandService.qml` (singleton)
  - Workspace management (list, active, switch)
  - Window tracking (open, close, focus, title changes)
  - Monitor information (layout, resolution, scale)
  - Hyprland IPC integration via hyprctl
  - Window-to-workspace mapping
  - Signal: `workspaceChanged`, `windowOpened`, `windowClosed`, `windowFocusChanged`, `monitorChanged`, `windowTitleChanged`
- [x] Create `src/services/SystemTrayService.qml` (singleton)
  - StatusNotifierItem (SNI) protocol support
  - Tray icon management
  - Context menu handling
  - Common tray app detection (nm-applet, blueman, etc.)
  - Signal: `trayItemsChanged`, `trayItemActivated`, `trayItemMenuRequested`
- [x] Create `src/services/NotificationService.qml` (singleton)
  - Notification creation and management
  - Notification queue with max visible limit
  - Do not disturb mode toggle
  - Notification history with grouping
  - Unread notification count
  - Auto-dismiss with timeout support
  - Signal: `notificationReceived`, `notificationDismissed`, `dndModeChanged`, `notificationsCleared`
- [x] Create `src/services/SessionService.qml` (singleton)
  - User session information (name, display name, home)
  - User avatar discovery
  - Session lifecycle events
  - XDG directories support
  - Session duration tracking
  - Signal: `sessionStarted`, `sessionEnding`, `userInfoChanged`

### 1.6 Service Documentation ✅
- [x] API documentation in service headers (JSDoc-style comments)
- [x] All signals and methods documented inline
- [x] Usage examples in service header comments
- [x] All 11 services registered in `src/services/qmldir`

**Deliverables**: 
- ✅ All services implemented as singletons
- ✅ Proper qmldir registrations
- ✅ Signal-based communication
- ✅ Service integration tests

---

## Phase 2: Atomic Component Library

**Goal**: Build reusable primitive components following atomic design principles.

### 2.1 Atoms (Primitives) ✅
- [x] `src/atoms/Button.qml` - Base button with hover, pressed, disabled states
  - Theme-integrated colors (filled, outlined, ghost variants)
  - Size variants (small, medium, large)
  - Optional icon support with text
  - Press animation and shadow effects
- [x] `src/atoms/IconButton.qml` - Icon-only button variant
  - Circular and rounded variants
  - Hover effects with glassmorphism
  - Built-in tooltip support
  - Scale press animation
- [x] `src/atoms/TextButton.qml` - Text button with optional icon
  - Primary, secondary, tertiary variants
  - Destructive action styling
  - Icon left/right positioning
  - Consistent with Button sizes
- [x] `src/atoms/Icon.qml` - Icon wrapper with theme support
  - SVG/PNG support with color overlay
  - Configurable size and rotation
  - Smooth scaling with mipmaps
- [x] `src/atoms/Label.qml` - Text label with theme typography
  - Typography variants (h1-h4, body, caption, button, clock)
  - Text truncation and wrapping support
  - Configurable font size and weight
  - Line height based on variant
- [x] `src/atoms/Input.qml` - Base text input
  - Placeholder text with fade animation
  - Validation states and error styling
  - Clear button option
  - Password mode with masking
  - Left/right icon support
- [x] `src/atoms/Slider.qml` - Custom styled slider
  - Horizontal and vertical orientations
  - Step size support with snap
  - Optional value display label
  - Smooth value animations
  - Hover glow effect on handle
- [x] `src/atoms/ProgressBar.qml` - Linear progress indicator
  - Determinate and indeterminate modes
  - Animated indeterminate sliding
  - Theme color integration
  - Gradient shine overlay
- [x] `src/atoms/Card.qml` - Card/container component
  - Glassmorphic styling option
  - Hover state changes
  - Elevation shadow support
  - Configurable padding and radius
- [x] `src/atoms/Badge.qml` - Notification badge
  - Count display with max limit (99+)
  - Color variants (info, success, warning, error)
  - Dot-only mode
  - Pulse animation for error variant
- [x] `src/atoms/Tooltip.qml` - Tooltip component
  - Auto-positioning (top, bottom, left, right)
  - Show delay configuration
  - Fade and scale animations
  - Automatic boundary detection
- [x] `src/atoms/Divider.qml` - Visual separator
  - Horizontal and vertical orientations
  - Optional label with positioning
  - Configurable thickness and color
- [x] `src/atoms/Spinner.qml` - Loading spinner
  - Configurable size and color
  - Rotation animation
  - Canvas-based rendering
- [x] `src/atoms/qmldir` - All 13 atoms exported

### 2.2 Molecules (Simple Combinations) ✅
- [x] `src/molecules/Clock.qml` - Time/date display with live updates
  - 12/24 hour format support
  - Configurable date format
  - Click signal for calendar popup
- [x] `src/molecules/VolumeControl.qml` - Volume slider + icon
  - Mute toggle button
  - Integration with AudioService
  - Horizontal/vertical orientations
- [x] `src/molecules/BrightnessControl.qml` - Brightness slider + icon
  - Integration with BrightnessService
  - Step-based adjustment
  - Live value updates
- [x] `src/molecules/NetworkIndicator.qml` - Network status icon
  - Signal strength visualization (WiFi bars)
  - Connection type indicator
  - Click to expand menu
- [x] `src/molecules/BatteryIndicator.qml` - Battery icon + percentage
  - Charging state detection
  - Color changes based on level (critical/low/warning)
  - Customizable critical threshold
- [x] `src/molecules/WorkspaceIndicator.qml` - Workspace dots/buttons
  - Active workspace highlight
  - Window occupancy indicator
  - Click to switch workspaces
- [x] `src/molecules/WindowPreview.qml` - Window thumbnail preview
  - Hover effects with elevation
  - Focus indicator
  - Click to focus window
- [x] `src/molecules/NotificationItem.qml` - Single notification display
  - App icon, title, body
  - Dismiss button and timestamp
  - Action button support
- [x] `src/molecules/MediaWidget.qml` - Media player mini widget
  - Album art placeholder, title, artist
  - Play/pause, next, previous controls
  - Progress bar integration
- [x] `src/molecules/SearchInput.qml` - Search with icon and clear
  - Recent searches dropdown
  - Keyboard shortcut support
  - Auto-complete ready
- [x] `src/molecules/ListItem.qml` - List item with icon, text, actions
  - Hover effects and selection state
  - Primary/subtitle text support
  - Arrow indicator option
- [x] `src/molecules/AppGridItem.qml` - Application grid tile
  - App icon with glassmorphic background
  - Launch on click with scale animation
  - Tooltip support
- [x] `src/molecules/PowerMenu.qml` - Power options dropdown
  - Shutdown, reboot, suspend, logout actions
  - Confirmation dialogs for destructive actions
  - PowerManager integration
- [x] `src/molecules/UserMenu.qml` - User menu with avatar
  - User avatar and name display
  - Settings, lock, logout actions
  - SessionService integration
- [x] `src/molecules/qmldir` - All 14 molecules exported

### 2.3 Layouts (Templates) ✅
- [x] `src/layouts/PanelLayout.qml`
- Horizontal panel layout
- Configurable spacing
- Alignment options (left, center, right)
- Vertical alignment support
- [x] `src/layouts/PopupLayout.qml`
- Centered popup/dialog
- Backdrop overlay with opacity
- Modal and non-modal support
- Close on backdrop click
- Fade and scale animations
- [x] `src/layouts/DrawerLayout.qml`
- Slide-out drawer
- Left, right, top, bottom positions
- Open/close animations
- Backdrop option
- [x] `src/layouts/GridLayout.qml`
- Responsive grid
- Column count configuration
- Item spacing
- Auto-sizing cells
- [x] `src/layouts/LayerLayout.qml`
- Z-index layering helper
- Stacking context management
- Named layers (background, content, overlay, modal, tooltip, top)
- [x] Create layouts qmldir

### 2.4 Animations (Reusable) ✅
- [x] `src/animations/FadeAnimation.qml`
- Fade in/out transitions
- Configurable duration
- Easing curve support
- [x] `src/animations/SlideAnimation.qml`
- Slide transitions
- Direction options (left, right, up, down)
- Distance configuration
- [x] `src/animations/ScaleAnimation.qml`
- Scale effects
- Origin point configuration
- Spring back easing
- [x] `src/animations/SpringAnimation.qml`
- Physics-based animations
- Spring/damping parameters
- Mass and epsilon tuning
- [x] Create animations qmldir

### 2.5 Component Testing ✅
- [x] Create visual test pages for each component
- `tests/manual/AtomTests.qml` - Atom component tests
- `tests/manual/MoleculeTests.qml` - Molecule component tests
- `tests/manual/LayoutTests.qml` - Layout component tests
- `tests/manual/AnimationTests.qml` - Animation component tests
- `tests/manual/ComponentShowcase.qml` - Comprehensive showcase
- [x] Document component properties and signals
- `docs/COMPONENT_REFERENCE.md` - Full component documentation
- [x] Create component showcase demo

**Deliverables**:
- ✅ Complete atomic component library
- ✅ Consistent styling and behavior
- ✅ All components theme-aware
- ✅ Layout components (PanelLayout, PopupLayout, DrawerLayout, GridLayout, LayerLayout)
- ✅ Animation components (FadeAnimation, SlideAnimation, ScaleAnimation, SpringAnimation)
- ✅ Component documentation (COMPONENT_REFERENCE.md)
- ✅ Visual test pages (AtomTests, MoleculeTests, LayoutTests, AnimationTests, ComponentShowcase)

---

## Phase 3: Composite Components

**Goal**: Build complex organism components that combine molecules and atoms.

### 3.1 Existing Components (Refactor)
- [x] Review `src/components/VideoBackground.qml` - Already well-implemented
  - Dual MediaPlayer for seamless crossfade transitions
  - Custom JS playlist manager (parses .m3u files via FileView)
  - Time-of-day playlist selection (day/night)
  - Random shuffle on load
- [x] Create `src/components/GlassPanel.qml`
  - Glassmorphic panel container
  - Configurable blur intensity
  - Border and shadow options
  - Elevation support (0-3 levels)
- [x] Review `src/components/PasswordField.qml` - Already well-implemented
  - Secure password input with glassmorphic styling
  - Focus glow effect
  - Theme integration
- [x] Review `src/components/StatusMessage.qml` - Already well-implemented
  - Status/error message display
  - Text shadow for video readability
  - Fade animation
- [x] Review `src/components/ActionButton.qml` - Already well-implemented
  - Circular glassmorphic button
  - Hover animations
  - Tooltip support
- [x] Review `src/components/AudioMetadata.qml` - Already well-implemented
  - Track, artist, album display
  - Progress bar
  - Live stream detection
- [x] Review `src/components/AudioPlayerButton.qml` - Already well-implemented
  - Transport controls (play, pause, next, previous)
  - Accessibility support
  - Theme integration
- [x] Review `src/components/AudioController.qml` - Already well-implemented
  - Full audio controls
  - Volume, metadata, playback
  - Auto-show when audio active

### 3.2 New Composite Components
- [x] `src/components/Calendar.qml`
  - Calendar widget with month/year navigation
  - Event indicators (optional integration)
  - Today highlight
  - Date selection
- [x] `src/components/Calendar.qml`
  - Calendar widget with month/year navigation
  - Event indicators (optional integration)
  - Today highlight
  - Date selection
- [x] `src/components/WeatherWidget.qml`
  - Weather display with location, temperature, condition
  - Current conditions with humidity and wind
  - Forecast row (optional, up to 5 days)
  - Loading and error states
- [x] `src/components/SystemMonitor.qml`
  - CPU/RAM usage display with progress bars
  - Real-time graph visualization
  - Network and disk stats
  - Configurable update interval
- [x] `src/components/QuickSettings.qml`
  - Quick toggles grid (WiFi, Bluetooth, DND, Airplane, Night Light)
  - Volume/brightness sliders
  - Extensible tile system
  - Service availability checks

**Deliverables**: 
- ✅ All composite components refactored/created
- ✅ Integration with services
- ✅ Consistent with atomic design
- ✅ Updated components qmldir with all exports

---

## Phase 4: Status Bar Module

**Goal**: Implement the status bar module with all standard widgets.

### 4.1 Module Structure
- [x] Create `src/modules/statusbar/` directory
- [x] Create `src/modules/statusbar/qmldir`
  ```qmldir
  module StatusBarModule

  StatusBar 1.0 StatusBar.qml
  StatusBarLeft 1.0 StatusBarLeft.qml
  StatusBarCenter 1.0 StatusBarCenter.qml
  StatusBarRight 1.0 StatusBarRight.qml
  TrayItem 1.0 TrayItem.qml

  singleton BarController 1.0 BarController.qml
  ```

### 4.2 Controller
- [x] Create `src/modules/statusbar/BarController.qml` (singleton)
  - Bar visibility control
  - Module configuration
  - Auto-hide behavior
  - Layout mode selection (macOS/Windows)
  - Multi-monitor support
  - Signal: `visibilityChanged`, `heightChanged`, `layoutModeChanged`, `moduleVisibilityChanged`

### 4.3 Main Component
- [x] Create `src/modules/statusbar/StatusBar.qml`
  - Main status bar component
  - macOS-style floating pill design
  - Left, center, right sections
  - Configurable height
  - Multi-monitor support via Variants
  - Glassmorphic background
  - Auto-hide behavior with hover detection
  - Integration with shell.qml

### 4.4 Sections
- [x] Create `src/modules/statusbar/StatusBarLeft.qml`
  - App launcher button (macOS style)
  - Hyprland workspace indicator
  - Active window title (optional)
  - Divider separators
- [x] Create `src/modules/statusbar/StatusBarCenter.qml`
  - Clock display (using live timer)
  - Date display (optional)
  - 12/24 hour format support
  - Seconds display option
  - Hover effects
  - Calendar popup trigger (TODO)
- [x] Create `src/modules/statusbar/StatusBarRight.qml`
  - System tray integration
  - Network indicator
  - Battery indicator
  - Volume indicator (with scroll to adjust)
  - Bluetooth indicator
  - Control center toggle
  - Notification center toggle with badge
  - Dividers between sections

### 4.5 Tray Item Component
- [x] Create `src/modules/statusbar/TrayItem.qml`
  - System tray item display
  - Icon with emoji/unicode support
  - Tooltip on hover
  - Left-click activation
  - Right-click context menu
  - Attention animation (pulse)

### 4.6 Integration
- [x] Integrate status bar into `shell.qml`
  - Use `Variants` with `Quickshell.screens`
  - Use `PanelWindow` for proper Wayland behavior
  - Layer: QuickshellLayer.top
- [x] Test multi-monitor behavior
- [x] Add configuration options (height, position, modules to show)
- [x] Add Theme icons property for emoji/unicode icons

**Deliverables**:
- ✅ Working status bar module
- ✅ macOS-style floating pill design
- ✅ Multi-monitor support
- ✅ All standard widgets (clock, workspaces, tray, indicators)
- ✅ Configurable layout (macOS/Windows modes)
- ✅ Auto-hide behavior
- ✅ Glassmorphic UI consistent with lockscreen
- ✅ Integration with all services (Hyprland, Audio, Network, Battery, SystemTray, Notification)

---

## Phase 5: Application Launcher Module

**Goal**: Implement the application launcher with search, grid/list views, and categories.

### 5.1 Model
- [ ] Create `src/models/ApplicationModel.qml`
  - Installed applications list
  - Application metadata (name, icon, description, categories)
  - Search/filter functionality
  - Usage tracking (for favorites)

### 5.2 Module Structure
- [ ] Create `src/modules/launcher/` directory
- [ ] Create `src/modules/launcher/qmldir`
  ```qmldir
  module LauncherModule

  Launcher 1.0 Launcher.qml
  LauncherGrid 1.0 LauncherGrid.qml
  LauncherList 1.0 LauncherList.qml
  LauncherSearch 1.0 LauncherSearch.qml
  CategoryView 1.0 CategoryView.qml
  FavoritesPanel 1.0 FavoritesPanel.qml

  singleton LauncherController 1.0 LauncherController.qml
  ```

### 5.3 Controller
- [ ] Create `src/modules/launcher/LauncherController.qml` (singleton)
  - Launcher visibility control
  - Show/hide with animation
  - Keyboard shortcut handling
  - Recent apps tracking
  - Signal: `visibilityChanged`, `appLaunched`

### 5.4 Main Component
- [ ] Create `src/modules/launcher/Launcher.qml`
  - Main launcher component
  - FloatingWindow integration
  - Search bar at top
  - Grid/list view toggle
  - Favorites panel
  - Category sidebar

### 5.5 Views
- [ ] Create `src/modules/launcher/LauncherGrid.qml`
  - App grid view
  - Responsive column count
  - Icon + name display
  - Hover/click animations
- [ ] Create `src/modules/launcher/LauncherList.qml`
  - App list view
  - Icon + name + description
  - Keyboard navigation
- [ ] Create `src/modules/launcher/LauncherSearch.qml`
  - Search functionality
  - Real-time filtering
  - Recent searches
  - Keyboard navigation through results

### 5.6 Additional Features
- [ ] Create `src/modules/launcher/CategoryView.qml`
  - Category browsing
  - Category icons
  - Filter by category
- [ ] Create `src/modules/launcher/FavoritesPanel.qml`
  - Pinned favorites
  - Drag to reorder
  - Quick access section

### 5.7 Integration
- [ ] Integrate launcher into `shell.qml`
  - Use `Loader` for lazy loading
  - Use `FloatingWindow`
  - Keyboard shortcut to toggle (e.g., Super+Space)
- [ ] Test application launching
- [ ] Add configuration options

**Deliverables**: 
- ✅ Working application launcher
- ✅ Search functionality
- ✅ Grid and list views
- ✅ Favorites and categories
- ✅ Keyboard navigation

---

## Phase 6: Notification Module

**Goal**: Implement notification management with toast popups and notification center.

### 6.1 Model
- [ ] Create `src/models/NotificationModel.qml`
  - Notification data structure
  - App info, icon, title, body, actions
  - Timestamp and expiry
  - Read/unread status

### 6.2 Module Structure
- [ ] Create `src/modules/notifications/` directory
- [ ] Create `src/modules/notifications/qmldir`
  ```qmldir
  module NotificationModule

  NotificationCenter 1.0 NotificationCenter.qml
  NotificationPopup 1.0 NotificationPopup.qml
  NotificationList 1.0 NotificationList.qml
  NotificationSettings 1.0 NotificationSettings.qml

  singleton NotificationController 1.0 NotificationController.qml
  ```

### 6.3 Controller
- [ ] Create `src/modules/notifications/NotificationController.qml` (singleton)
  - Notification queue management
  - Show/hide notification center
  - Do not disturb mode
  - Notification history
  - Clear/dismiss actions
  - Signal: `notificationReceived`, `notificationDismissed`, `centerVisibilityChanged`

### 6.4 Components
- [ ] Create `src/modules/notifications/NotificationPopup.qml`
  - Toast notifications
  - Auto-dismiss timer
  - Slide-in animation
  - Stack multiple notifications
  - Action buttons
- [ ] Create `src/modules/notifications/NotificationCenter.qml`
  - Main notification panel
  - Slide-out drawer or floating panel
  - Notification list
  - Clear all button
  - Settings toggle
- [ ] Create `src/modules/notifications/NotificationList.qml`
  - Historical notifications
  - Grouped by day
  - Dismiss individual/all
  - Scrollable list
- [ ] Create `src/modules/notifications/NotificationSettings.qml`
  - Do not disturb toggle
  - Per-app notification settings
  - History retention
  - Sound toggle

### 6.5 Integration
- [ ] Integrate with `NotificationService`
- [ ] Add to status bar (notification count badge)
- [ ] Integrate into `shell.qml`
- [ ] Test with various notification sources

**Deliverables**: 
- ✅ Toast notification popups
- ✅ Notification center panel
- ✅ Notification history
- ✅ Do not disturb mode
- ✅ Per-app settings

---

## Phase 7: Control Center Module

**Goal**: Implement control center with quick toggles, media controls, and connection panels.

### 7.1 Module Structure
- [ ] Create `src/modules/controlcenter/` directory
- [ ] Create `src/modules/controlcenter/qmldir`
  ```qmldir
  module ControlCenterModule

  ControlCenter 1.0 ControlCenter.qml
  QuickToggles 1.0 QuickToggles.qml
  MediaControls 1.0 MediaControls.qml
  SlidersPanel 1.0 SlidersPanel.qml
  ConnectionsPanel 1.0 ConnectionsPanel.qml

  singleton ControlController 1.0 ControlController.qml
  ```

### 7.2 Controller
- [ ] Create `src/modules/controlcenter/ControlController.qml` (singleton)
  - Panel visibility control
  - Panel state management
  - Quick settings state persistence
  - Signal: `panelVisibilityChanged`, `toggleStateChanged`

### 7.3 Main Component
- [ ] Create `src/modules/controlcenter/ControlCenter.qml`
  - Main control center panel
  - FloatingWindow or drawer
  - Tabbed or sectioned layout
  - Quick toggles section
  - Media controls section
  - Sliders section
  - Connections panel section

### 7.4 Sections
- [ ] Create `src/modules/controlcenter/QuickToggles.qml`
  - Quick settings toggles
  - WiFi, Bluetooth, DND, Airplane mode
  - Custom toggle support
  - Grid layout
  - State persistence
- [ ] Create `src/modules/controlcenter/MediaControls.qml`
  - Media player controls
  - Now playing display
  - Play/pause, next, previous
  - Progress bar
  - Volume control
  - Player selector
- [ ] Create `src/modules/controlcenter/SlidersPanel.qml`
  - Volume/brightness sliders
  - Label with values
  - Icon indicators
  - Smooth updates
- [ ] Create `src/modules/controlcenter/ConnectionsPanel.qml`
  - Network/Bluetooth panel
  - Available networks/devices list
  - Connect/disconnect actions
  - Connection status

### 7.5 Integration
- [ ] Integrate with relevant services (Network, Bluetooth, Audio, Brightness)
- [ ] Add toggle button to status bar
- [ ] Integrate into `shell.qml`
- [ ] Test all toggles and controls

**Deliverables**: 
- ✅ Control center panel
- ✅ Quick toggles
- ✅ Media controls
- ✅ Volume/brightness sliders
- ✅ Network/Bluetooth panel

---

## Phase 8: Popups & OSDs

**Goal**: Create shared popup components for system feedback.

### 8.1 Base Components
- [ ] Create `src/popups/BasePopup.qml`
  - Base popup with animations
  - Backdrop overlay
  - Close on backdrop click
  - Keyboard escape
- [ ] Create `src/popups/ConfirmDialog.qml`
  - Confirmation dialog
  - Title, message
  - Confirm/cancel buttons
  - Destructive action variant
- [ ] Create `src/popups/InputDialog.qml`
  - Text input dialog
  - Label and placeholder
  - Validation
  - Submit/cancel

### 8.2 On-Screen Displays
- [ ] Create `src/popups/VolumeOSD.qml`
  - Volume on-screen display
  - Appears on volume change
  - Auto-dismiss
  - Icon + slider + percentage
- [ ] Create `src/popups/BrightnessOSD.qml`
  - Brightness OSD
  - Appears on brightness change
  - Auto-dismiss
  - Icon + slider + percentage
- [ ] Create `src/popups/ScreenshotPreview.qml`
  - Screenshot preview
  - Save/copy actions
  - Auto-dismiss timer

### 8.3 Integration
- [ ] Wire OSDs to service events
- [ ] Add keyboard shortcuts for screenshot
- [ ] Test OSD behavior

**Deliverables**: 
- ✅ Reusable popup components
- ✅ Volume/brightness OSDs
- ✅ Screenshot preview
- ✅ Consistent OSD styling

---

## Phase 9: Models & Data Layer

**Goal**: Implement data models for complex data structures.

### 9.1 Models
- [ ] Create `src/models/WorkspaceModel.qml`
  - Hyprland workspaces
  - Active workspace tracking
  - Workspace names/icons
- [ ] Create `src/models/WindowModel.qml`
  - Window list
  - Window metadata (title, class, workspace)
  - Active window tracking
- [ ] Create `src/models/CalendarModel.qml`
  - Calendar events (optional)
  - Integration with calendar services
- [ ] Create `src/models/MediaModel.qml`
  - Media player data
  - Multiple player support
  - Player metadata

### 9.2 Integration
- [ ] Connect models to services
- [ ] Connect models to UI components
- [ ] Test data flow

**Deliverables**: 
- ✅ All data models
- ✅ Service integration
- ✅ UI bindings

---

## Phase 10: Scripts & Tooling

**Goal**: Create utility scripts for development and production.

### 10.1 Scripts
- [x] Create `scripts/run.sh` - Development launcher (supports `--mode full|bar|lock`)
- [x] Create `scripts/lock.sh` - Production lock launcher (IPC check + auto-lock)
- [x] Create `scripts/bar.sh` - Status bar launcher
- [x] Create `scripts/launcher.sh` - Application launcher trigger
- [x] Create `scripts/test.sh` - Test suite runner (86 tests, comprehensive coverage)

### 10.2 Theme System
- [x] Create `themes/default/` directory with `theme.json` (Catppuccin Mocha)
- [x] Create `themes/catppuccin-latte/theme.json`
- [ ] Create theme loading mechanism (future - ThemeEngine will parse JSON)
- [ ] Add theme switcher in control center (future)

### 10.3 Assets
- [x] Organize `assets/` directory with proper structure (icons/actions, apps, categories, status, system)
- [x] Verify playlists (day.m3u, night.m3u)
- [x] Organize videos, sounds, fonts, wallpapers directories

**Deliverables**: 
- ✅ All utility scripts
- ✅ Theme definitions
- ✅ Organized assets

---

## Phase 11: Testing & Quality Assurance

**Goal**: Comprehensive testing across all modules.

### 11.1 Unit Tests
- [ ] Create `tests/unit/` directory
- [ ] Service unit tests
  - ConfigService tests
  - AudioService tests
  - NetworkService tests
  - All other services
- [ ] Model unit tests
  - Data validation
  - Filtering/sorting

### 11.2 Integration Tests
- [ ] Create `tests/integration/` directory
- [ ] Module integration tests
  - Status bar + services
  - Launcher + ApplicationModel
  - Notifications + NotificationService
  - Control center + services
- [ ] Cross-module communication tests

### 11.3 Manual Tests
- [ ] Create `tests/manual/` directory
- [ ] Visual regression tests
- [ ] Multi-monitor tests
- [ ] Performance tests
- [ ] Memory leak tests

### 11.4 Documentation
- [ ] Update AGENTS.md with final architecture
- [ ] Create COMPONENT_GUIDE.md
- [ ] Create API_REFERENCE.md
- [ ] Create ARCHITECTURE.md with decisions

**Deliverables**: 
- ✅ Comprehensive test suite
- ✅ All tests passing
- ✅ Complete documentation

---

## Phase 13: Standalone Locker Extraction

**Goal**: Extract the locker into an independently runnable module with its own entry point, eliminating duplicate files and ensuring clean separation from the full shell.

### 13.1 Duplicate Consolidation
- [x] Re-export `LockController` from module in `src/services/qmldir`, remove `src/services/LockController.qml`
- [x] Re-export `LockScreen` from module in `src/widgets/qmldir`, remove `src/widgets/LockScreen.qml`
- [x] Fix missing `setLockInstance(sessionLock)` call in `shell.qml`

### 13.2 Standalone Entry Point
- [x] Create `locker/shell.qml` — minimal `ShellRoot` + `WlSessionLock` + `LockScreen`
- [x] Create `locker/qmldir` — module declaration for standalone usage
- [x] Create `scripts/locker.sh` — launcher script pointing to `locker/` entry point

### 13.3 Test Updates
- [x] Update `test_services()` to verify re-export instead of file existence
- [x] Add standalone locker structure tests (entry point, qmldir, launch script)
- [x] Fix `.qmlls.ini` symlink detection in test suite

**Deliverables**:
- ✅ Standalone locker launchable via `./scripts/locker.sh` or `qs -p ./locker/`
- ✅ Zero duplicate files — canonical implementations in `src/modules/lockscreen/`
- ✅ All 220 tests passing
- ✅ Backward compatible — `Services.LockController` and `Widgets.LockScreen` imports still resolve

---

## Phase 12: Polish & Optimization

**Goal**: Final polish, performance optimization, and bug fixes.

### 12.1 Performance
- [ ] Profile startup time
- [ ] Optimize component rendering
- [ ] Reduce memory footprint
- [ ] Optimize animations
- [ ] Lazy loading for heavy components

### 12.2 UX Polish
- [ ] Consistent animations across modules
- [ ] Keyboard navigation improvements
- [ ] Accessibility considerations
- [ ] Error handling and user feedback
- [ ] Loading states

### 12.3 Bug Fixes
- [ ] Fix all known issues
- [ ] Edge case handling
- [ ] Race condition prevention
- [ ] State synchronization

### 12.4 Final Testing
- [ ] Full system test
- [ ] Multi-monitor validation
- [ ] Long-running stability test
- [ ] Hot-reload testing

**Deliverables**: 
- ✅ Optimized performance
- ✅ Polished UX
- ✅ All bugs fixed
- ✅ Production-ready build

---

## Dependencies & Ordering

```
Phase 0 (Foundation) ✅ COMPLETE
↓
Phase 10 (Scripts & Tooling) ✅ COMPLETE (ran parallel)
↓
Phase 1 (Services) ✅ COMPLETE - 11 services implemented
↓
Phase 2.1 (Atoms) ✅ COMPLETE - 13 atom components
↓
Phase 2.2 (Molecules) ✅ COMPLETE - 14 molecule components
↓
Phase 2.3 (Layouts) ✅ COMPLETE - 5 layout components
↓
Phase 2.4 (Animations) ✅ COMPLETE - 4 animation components
↓
Phase 2.5 (Testing) ✅ COMPLETE - Visual tests and documentation
↓
Phase 3 (Components) ← NEXT PRIORITY
    ↓
Phase 4 (Status Bar) ──┬── Phase 5 (Launcher)
                       ├── Phase 6 (Notifications)
                       └── Phase 7 (Control Center)
    ↓
Phase 8 (Popups & OSDs)
Phase 9 (Models) [can run parallel to Phases 4-7]
    ↓
Phase 11 (Testing)
    ↓
Phase 12 (Polish)
    ↑
Phase 13 (Standalone Locker) [depends on Phase 0#Lockscreen]
```

---

## Key Milestones

| Milestone | Phase | Status | Description |
|-----------|-------|--------|-------------|
| M0: Foundation Complete | 0, 10 | ✅ Done | Core infrastructure, theme system, all scripts, lockscreen loads cleanly |
| M1: Component Library | 2 | ✅ Done | 36 components (13 atoms + 14 molecules + 5 layouts + 4 animations) complete |
| M2: Services Layer | 1 | ✅ Done | All 11 business logic services implemented and tested |
| M3: Status Bar Live | 4 | ⏳ Pending | Status bar working on all monitors |
| M4: All Modules Functional | 4-7 | ⏳ Pending | All five modules implemented |
| M5: Feature Complete | 8-9 | ⏳ Pending | All features implemented and integrated |
| M6: Tested & Documented | 11 | ⏳ Pending | Comprehensive tests and documentation |
| M7: Production Ready | 12 | ⏳ Pending | Optimized, polished, bug-free |

---

## Risk Mitigation

### Technical Risks
- **Quickshell API changes**: Keep Quickshell updated, test after updates
- **Performance issues**: Profile early, optimize hot paths, use lazy loading
- **Multi-monitor complexity**: Test on multiple monitors throughout development

### Architecture Risks
- **Service coupling**: Enforce signal-based communication, avoid direct dependencies
- **State management**: Use Scope for persistence, centralize state in services
- **Import path issues**: Enforce relative imports, update AGENTS.md with rules

### Process Risks
- **Scope creep**: Stick to roadmap, defer nice-to-haves to future versions
- **Testing debt**: Write tests alongside features, not after
- **Documentation lag**: Update docs as you go, not at the end

---

## Success Criteria

### Phase 0, 1 & 10 (Complete ✅)
- [x] Complete modular directory structure per PROJECT_STRUCTURE.md
- [x] All qmldir files with proper module directives
- [x] Theme system with ThemeEngine singleton (colors, typography, spacing, effects, animation)
- [x] Core utilities (Constants, Logger, Utils, Errors)
- [x] shell.qml refactored with Scope for state persistence
- [x] All utility scripts (run.sh, lock.sh, bar.sh, launcher.sh, test.sh)
- [x] Theme JSON definitions (Catppuccin Mocha + Latte)
- [x] 99 tests passing with zero failures
- [x] Lockscreen loads cleanly with zero errors
- [x] Video playback working
- [x] Lock screen activates correctly via lock.sh
- [x] Phase 1: All 11 services implemented as singletons
  - ConfigService: JSON-based configuration with schema validation
  - AudioService: MPRIS media player integration
  - NetworkService: WiFi/Ethernet monitoring via nmcli
  - BluetoothService: Bluetooth management via bluetoothctl
  - BatteryService: Multi-battery monitoring with sysfs
  - BrightnessService: Backlight control via brightnessctl/xrandr
  - HyprlandService: Hyprland IPC integration (workspaces, windows, monitors)
  - SystemTrayService: SNI protocol support
  - NotificationService: Notification queue with DND mode
  - SessionService: User session and XDG directory management
  - PowerManager: System power actions via systemctl

### Phase 2.1 Atoms (Complete ✅)
- [x] 13 atom components implemented
- [x] All atoms theme-aware with Catppuccin Mocha
- [x] Glassmorphic styling throughout
- [x] Consistent API patterns (properties, signals, methods)
- [x] Full JSDoc-style documentation in each file

### Phase 2.2 Molecules (Complete ✅)
- [x] 14 molecule components implemented
- [x] Service integration (AudioService, NetworkService, BatteryService, etc.)
- [x] Atoms composition pattern demonstrated
- [x] All molecules theme-aware

### Phase 2.3 Layouts (Complete ✅)
- [x] 5 layout components implemented
- [x] PanelLayout for status bars
- [x] PopupLayout for dialogs
- [x] DrawerLayout for slide-out panels
- [x] GridLayout for responsive grids
- [x] LayerLayout for z-index management

### Phase 2.4 Animations (Complete ✅)
- [x] 4 animation components implemented
- [x] FadeAnimation for opacity transitions
- [x] SlideAnimation for directional movement
- [x] ScaleAnimation for size effects
- [x] SpringAnimation for physics-based motion

### Phase 2.5 Component Testing (Complete ✅)
- [x] Visual test pages for atoms, molecules, layouts, animations
- [x] ComponentShowcase.qml for comprehensive demo
- [x] COMPONENT_REFERENCE.md documentation

### Remaining Phases (Pending)
- [ ] Phase 3: Composite/organism components
- [ ] Phase 4-7: All 5 modules (lockscreen, statusbar, launcher, notifications, controlcenter) functional
- [ ] Multi-monitor support working correctly
- [ ] Theme system supports runtime switching
- [ ] Comprehensive test suite (unit, integration, manual)
- [ ] No memory leaks after 24-hour test
- [ ] Startup time < 2 seconds
- [ ] Hot-reload works without state loss

---

## Notes

- **Atomic Design**: Always check if a component can be built from existing atoms/molecules before creating new ones
- **Singleton Pattern**: Remember both `pragma Singleton` AND `singleton` in qmldir are required
- **Import Paths**: Use relative imports only (`import "../services"`, not `root:/...`)
- **Quickshell**: Use `Variants` for non-visual elements across screens, `Scope` for state persistence
- **Signals**: Prefer signals over polling for state changes
- **Theme**: All UI must use Theme tokens, no hardcoded colors/values
- **Glassmorphism**: Maintain glassmorphic aesthetic throughout (blur, transparency, subtle borders)

---
## References

- [Project Structure](PROJECT_STRUCTURE.md)

---
*Last updated: 2026-04-12*
*Version: 1.4*
*Status: Phase 0 ✅, Phase 1 ✅, Phase 2 (Atoms, Molecules, Layouts, Animations, Testing) ✅ & Phase 10 ✅ Complete — Ready for Phase 3 (Composite Components)*
