# Qypr Implementation Roadmap

## Overview

This roadmap outlines the step-by-step implementation plan for transforming Qypr from a lockscreen-only application to a comprehensive desktop shell system. The plan is organized into phases, each building upon previous work while maintaining system stability.

**Current State**: Lockscreen module with video backgrounds, audio/MPRIS controls, and PAM authentication (Catppuccin Mocha glassmorphic UI).

**Target State**: Full desktop shell with status bar, application launcher, notification center, and control center.

---

## Phase 0: Foundation & Infrastructure

**Goal**: Establish project structure, theme system, and core services that all modules will depend on.

### 0.1 Project Restructuring
- [ ] Create directory structure per PROJECT_STRUCTURE.md
  ```
  src/
  ├── core/
  ├── theme/
  ├── services/
  ├── models/
  ├── atoms/
  ├── molecules/
  ├── components/
  ├── layouts/
  ├── modules/
  │   ├── lockscreen/    # Existing (move & refactor)
  │   ├── statusbar/     # New
  │   ├── launcher/      # New
  │   ├── notifications/ # New
  │   └── controlcenter/ # New
  ├── popups/
  └── animations/
  ```
- [ ] Move existing lockscreen code into `src/modules/lockscreen/`
- [ ] Create `qmldir` files for each module/directory
- [ ] Update all imports to use relative paths (avoid `root:/`)
- [ ] Create `.qmlls.ini` for LSP support

### 0.2 Theme System Implementation
- [ ] Create `src/theme/ThemeEngine.qml` (singleton)
  - Theme loading from JSON
  - Dark/light mode switching
  - Runtime theme switching support
- [ ] Create `src/theme/ColorPalette.qml`
  - Catppuccin Mocha palette (existing)
  - Token structure: background, surface, primary, secondary, text, glass
- [ ] Create `src/theme/Typography.qml`
  - Font families, sizes, weights
  - Text style presets (headline, body, caption, etc.)
- [ ] Create `src/theme/Spacing.qml`
  - Spacing scale (4px, 8px, 16px, 24px, 32px, 48px)
  - Border radius scale
- [ ] Create `src/theme/Effects.qml`
  - Glassmorphic blur effects
  - Shadow presets
  - Opacity tokens
- [ ] Create `src/theme/Animation.qml`
  - Animation duration presets (fast: 150ms, medium: 300ms, slow: 500ms)
  - Easing curves
- [ ] Create theme qmldir with singleton registration

### 0.3 Core Utilities
- [ ] Create `src/core/Constants.qml`
  - App version, paths, configuration keys
- [ ] Create `src/core/Logger.qml`
  - Log levels (debug, info, warning, error)
  - Structured logging with timestamps
- [ ] Create `src/core/Utils.qml` (singleton)
  - Common utility functions
  - String manipulation
  - Date/time formatting helpers
- [ ] Create `src/core/Errors.qml`
  - Error codes and messages
  - Error handling patterns
- [ ] Create core qmldir

### 0.4 Shell Entry Point Refactoring
- [ ] Refactor `shell.qml` to use modular architecture
  - Use `ShellRoot` as root
  - Implement `Variants` for multi-monitor support
  - Use `Scope` for IPC handlers and state persistence
  - Add module loader infrastructure
- [ ] Update `run.sh` and `lock.sh` scripts
- [ ] Test existing lockscreen functionality still works

**Deliverables**: 
- ✅ Complete project structure
- ✅ Working theme system
- ✅ Core utilities
- ✅ Refactored shell.qml
- ✅ All existing tests pass

---

## Phase 1: Services Layer

**Goal**: Implement all business logic services that modules will consume.

### 1.1 Configuration Service
- [ ] Create `src/services/ConfigService.qml` (singleton)
  - Load/save user preferences (JSON config file)
  - Default configuration values
  - Config change signals
  - Schema validation

### 1.2 Audio & Media Services
- [ ] Refactor existing audio code into `src/services/AudioService.qml` (singleton)
  - Audio device management
  - Volume control
  - Mute/unmute
  - MPRIS media player integration
  - Media metadata extraction
  - Playback controls (play, pause, next, previous)
  - Signal: `deviceChanged`, `volumeChanged`, `mediaChanged`, `playbackStateChanged`

### 1.3 Network & Connectivity Services
- [ ] Create `src/services/NetworkService.qml` (singleton)
  - WiFi status monitoring
  - Network name (SSID)
  - Signal strength
  - Connection state changes
  - Signal: `networkStatusChanged`, `availableNetworksChanged`
- [ ] Create `src/services/BluetoothService.qml` (singleton)
  - Bluetooth adapter status
  - Paired devices list
  - Connect/disconnect devices
  - Signal: `bluetoothStatusChanged`, `deviceConnected`, `deviceDisconnected`

### 1.4 Power & Display Services
- [ ] Create `src/services/BatteryService.qml` (singleton)
  - Battery percentage
  - Charging status
  - Time remaining estimation
  - Signal: `batteryLevelChanged`, `chargingStatusChanged`
- [ ] Create `src/services/BrightnessService.qml` (singleton)
  - Display brightness control
  - Brightness level persistence
  - Signal: `brightnessChanged`
- [ ] Create `src/services/PowerManager.qml` (singleton)
  - System power actions (shutdown, reboot, suspend, logout)
  - Session management integration
  - Signal: `powerActionRequested`

### 1.5 System Integration Services
- [ ] Create `src/services/HyprlandService.qml` (singleton)
  - Workspace management
  - Window tracking
  - Monitor information
  - Hyprland IPC integration
  - Signal: `workspaceChanged`, `windowOpened`, `windowClosed`, `monitorChanged`
- [ ] Create `src/services/SystemTrayService.qml` (singleton)
  - System tray protocol implementation
  - Tray icon management
  - Context menu handling
  - Signal: `trayItemsChanged`, `trayItemActivated`
- [ ] Create `src/services/NotificationService.qml` (singleton)
  - Notification daemon integration
  - Notification creation/reception
  - Do not disturb mode
  - Notification history
  - Signal: `notificationReceived`, `notificationDismissed`, `dndModeChanged`
- [ ] Create `src/services/SessionService.qml` (singleton)
  - User session information
  - User avatar/name
  - Session lifecycle events
  - Signal: `sessionStarted`, `sessionEnding`

### 1.6 Service Documentation
- [ ] Create API documentation for each service
- [ ] Document all signals and methods
- [ ] Create usage examples

**Deliverables**: 
- ✅ All services implemented as singletons
- ✅ Proper qmldir registrations
- ✅ Signal-based communication
- ✅ Service integration tests

---

## Phase 2: Atomic Component Library

**Goal**: Build reusable primitive components following atomic design principles.

### 2.1 Atoms (Primitives)
- [ ] `src/atoms/Button.qml`
  - Base button with hover, pressed, disabled states
  - Theme-integrated colors
  - Size variants (small, medium, large)
  - Animation on interaction
- [ ] `src/atoms/IconButton.qml`
  - Icon-only button variant
  - Hover effects
  - Tooltip support
- [ ] `src/atoms/TextButton.qml`
  - Text button with optional icon
  - Primary/secondary/tertiary variants
- [ ] `src/atoms/Icon.qml`
  - Icon wrapper with theme support
  - SVG/PNG support
  - Size presets
  - Color inheritance
- [ ] `src/atoms/Label.qml`
  - Text label with theme typography
  - Text truncation support
  - Color variants
- [ ] `src/atoms/Input.qml`
  - Base text input
  - Placeholder text
  - Validation states
  - Clear button option
  - Password mode
- [ ] `src/atoms/Slider.qml`
  - Custom styled slider
  - Theme colors
  - Step increments
  - Value display option
- [ ] `src/atoms/ProgressBar.qml`
  - Linear progress indicator
  - Determinate/indeterminate modes
  - Theme colors
- [ ] `src/atoms/Card.qml`
  - Card/container component
  - Glassmorphic styling (optional)
  - Hover effects
  - Border radius
- [ ] `src/atoms/Badge.qml`
  - Notification badge
  - Count display
  - Color variants (info, warning, error)
- [ ] `src/atoms/Tooltip.qml`
  - Tooltip component
  - Auto-positioning
  - Fade animations
- [ ] `src/atoms/Divider.qml`
  - Visual separator
  - Horizontal/vertical
  - Optional label
- [ ] `src/atoms/Spinner.qml`
  - Loading spinner
  - Size variants
  - Animation loop
- [ ] Create atoms qmldir

### 2.2 Molecules (Simple Combinations)
- [ ] `src/molecules/Clock.qml`
  - Time/date display
  - 12/24 hour format
  - Live updates
- [ ] `src/molecules/VolumeControl.qml`
  - Volume slider + icon
  - Mute button
  - Device selector
- [ ] `src/molecules/BrightnessControl.qml`
  - Brightness slider + icon
  - Live preview
- [ ] `src/molecules/NetworkIndicator.qml`
  - Network status icon
  - Signal strength visualization
  - Click to expand menu
- [ ] `src/molecules/BatteryIndicator.qml`
  - Battery icon + percentage
  - Charging indicator
  - Color changes based on level
- [ ] `src/molecules/WorkspaceIndicator.qml`
  - Workspace dots/buttons
  - Active workspace highlight
  - Click to switch
- [ ] `src/molecules/WindowPreview.qml`
  - Window thumbnail preview
  - Hover to preview
  - Click to focus
- [ ] `src/molecules/NotificationItem.qml`
  - Single notification display
  - App icon, title, body
  - Dismiss button
  - Timestamp
- [ ] `src/molecules/MediaWidget.qml`
  - Media player mini widget
  - Album art, title, artist
  - Play/pause, next buttons
- [ ] `src/molecules/SearchInput.qml`
  - Search with icon and clear
  - Keyboard shortcut support
  - Recent searches (optional)
- [ ] `src/molecules/ListItem.qml`
  - List item with icon, text, actions
  - Hover effects
  - Selection state
- [ ] `src/molecules/AppGridItem.qml`
  - Application grid tile
  - App icon, name
  - Launch on click
- [ ] `src/molecules/PowerMenu.qml`
  - Power options dropdown
  - Shutdown, reboot, suspend, logout
  - Confirmation dialog
- [ ] `src/molecules/UserMenu.qml`
  - User menu with avatar
  - User name display
  - Session actions
- [ ] Create molecules qmldir

### 2.3 Layouts (Templates)
- [ ] `src/layouts/PanelLayout.qml`
  - Horizontal panel layout
  - Configurable spacing
  - Alignment options
- [ ] `src/layouts/PopupLayout.qml`
  - Centered popup/dialog
  - Backdrop overlay
  - Animation support
- [ ] `src/layouts/DrawerLayout.qml`
  - Slide-out drawer
  - Open/close animations
  - Backdrop option
- [ ] `src/layouts/GridLayout.qml`
  - Responsive grid
  - Column count configuration
  - Item spacing
- [ ] `src/layouts/LayerLayout.qml`
  - Z-index layering helper
  - Stacking context management
- [ ] Create layouts qmldir

### 2.4 Animations (Reusable)
- [ ] `src/animations/FadeAnimation.qml`
  - Fade in/out
  - Configurable duration
- [ ] `src/animations/SlideAnimation.qml`
  - Slide transitions
  - Direction options
- [ ] `src/animations/ScaleAnimation.qml`
  - Scale effects
  - Origin point
- [ ] `src/animations/SpringAnimation.qml`
  - Physics-based animations
  - Spring/damping parameters
- [ ] Create animations qmldir

### 2.5 Component Testing
- [ ] Create visual test pages for each component
- [ ] Document component properties and signals
- [ ] Create component showcase demo

**Deliverables**: 
- ✅ Complete atomic component library
- ✅ Consistent styling and behavior
- ✅ All components theme-aware
- ✅ Component documentation

---

## Phase 3: Composite Components

**Goal**: Build complex organism components that combine molecules and atoms.

### 3.1 Existing Components (Refactor)
- [ ] Refactor `src/components/VideoBackground.qml`
  - Move from current location to new structure
  - Support playlists
  - Video switching
- [ ] Refactor `src/components/GlassPanel.qml`
  - Glassmorphic panel container
  - Configurable blur intensity
  - Border and shadow options
- [ ] Refactor `src/components/PasswordField.qml`
  - Secure password input
  - Visibility toggle
  - Integration with PAM
- [ ] Refactor `src/components/StatusMessage.qml`
  - Status/error message display
  - Auto-dismiss option
  - Icon variants
- [ ] Refactor `src/components/ActionButton.qml`
  - Animated action button
  - Icon + label
  - Loading state
- [ ] Refactor `src/components/AudioMetadata.qml`
  - Audio info display
  - Track, artist, album
  - Album art
- [ ] Refactor `src/components/AudioPlayerButton.qml`
  - Media control button
  - Play, pause, next, previous
  - MPRIS integration
- [ ] Refactor `src/components/AudioController.qml`
  - Full audio controls
  - Volume, metadata, playback
  - Media player selector

### 3.2 New Composite Components
- [ ] `src/components/Calendar.qml`
  - Calendar widget
  - Month/year navigation
  - Event indicators (optional integration)
- [ ] `src/components/WeatherWidget.qml`
  - Weather display
  - Current conditions
  - Forecast (optional API integration)
- [ ] `src/components/SystemMonitor.qml`
  - CPU/RAM usage display
  - Real-time updates
  - Graph visualization
- [ ] `src/components/QuickSettings.qml`
  - Quick toggles grid
  - WiFi, Bluetooth, DND toggles
  - Brightness/volume sliders
  - Extensible tile system

**Deliverables**: 
- ✅ All composite components refactored/created
- ✅ Integration with services
- ✅ Consistent with atomic design

---

## Phase 4: Status Bar Module

**Goal**: Implement the status bar module with all standard widgets.

### 4.1 Module Structure
- [ ] Create `src/modules/statusbar/` directory
- [ ] Create `src/modules/statusbar/qmldir`
  ```qmldir
  module StatusBarModule

  StatusBar 1.0 StatusBar.qml
  StatusBarLeft 1.0 StatusBarLeft.qml
  StatusBarCenter 1.0 StatusBarCenter.qml
  StatusBarRight 1.0 StatusBarRight.qml
  Taskbar 1.0 Taskbar.qml

  singleton BarController 1.0 BarController.qml

  internal BarLayout BarLayout.qml
  internal WidgetContainer WidgetContainer.qml
  ```

### 4.2 Controller
- [ ] Create `src/modules/statusbar/BarController.qml` (singleton)
  - Bar visibility control
  - Module configuration
  - Auto-hide behavior
  - Signal: `visibilityChanged`, `heightChanged`

### 4.3 Main Component
- [ ] Create `src/modules/statusbar/StatusBar.qml`
  - Main status bar component
  - Left, center, right sections
  - Configurable height
  - Multi-monitor support via Variants
  - Integration with shell.qml

### 4.4 Sections
- [ ] Create `src/modules/statusbar/StatusBarLeft.qml`
  - Hyprland workspace indicator
  - Window/task indicators
  - Custom widget slot
- [ ] Create `src/modules/statusbar/StatusBarCenter.qml`
  - Clock display (using Clock molecule)
  - Date display
  - Calendar popup on click
- [ ] Create `src/modules/statusbar/StatusBarRight.qml`
  - System tray
  - Network indicator
  - Battery indicator
  - Volume indicator
  - Power menu
  - Control center toggle
  - Notification center toggle

### 4.5 Taskbar
- [ ] Create `src/modules/statusbar/Taskbar.qml`
  - Window/task indicators
  - Active window highlight
  - Click to focus/switch
  - Window previews on hover

### 4.6 Integration
- [ ] Integrate status bar into `shell.qml`
  - Use `Variants` with `Quickshell.screens`
  - Use `PanelWindow` for proper Wayland behavior
- [ ] Test multi-monitor behavior
- [ ] Add configuration options (height, position, modules to show)

**Deliverables**: 
- ✅ Working status bar module
- ✅ Multi-monitor support
- ✅ All standard widgets
- ✅ Configurable layout

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
- [ ] Create `scripts/run.sh` - Development launcher (update existing)
- [ ] Create `scripts/lock.sh` - Production lock launcher (update existing)
- [ ] Create `scripts/bar.sh` - Status bar launcher
- [ ] Create `scripts/launcher.sh` - Application launcher trigger
- [ ] Create `scripts/test.sh` - Test suite runner (update existing)

### 10.2 Theme System
- [ ] Create `themes/default/` directory
  - `theme.json` - Catppuccin Mocha tokens
  - `preview.png` - Theme preview
- [ ] Create theme loading mechanism
- [ ] Add theme switcher in control center

### 10.3 Assets
- [ ] Organize `assets/` directory per structure
  - Fonts, icons, wallpapers, videos, sounds
  - Create placeholder icons if needed

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
Phase 0 (Foundation)
    ↓
Phase 1 (Services)
    ↓
Phase 2 (Atoms & Molecules) ──┬── Phase 3 (Components)
                               │
                               ├── Phase 4 (Status Bar)
                               ├── Phase 5 (Launcher)
                               ├── Phase 6 (Notifications)
                               └── Phase 7 (Control Center)
    ↓
Phase 8 (Popups & OSDs)
Phase 9 (Models) [can run parallel to Phases 4-7]
Phase 10 (Scripts & Tooling) [can run parallel]
    ↓
Phase 11 (Testing)
    ↓
Phase 12 (Polish)
```

---

## Key Milestones

| Milestone | Phase | Description |
|-----------|-------|-------------|
| M0: Foundation Complete | 0-1 | Core infrastructure and all services working |
| M1: Component Library | 2 | All atomic components built and tested |
| M2: Status Bar Live | 4 | Status bar working on all monitors |
| M3: All Modules Functional | 4-7 | All five modules implemented |
| M4: Feature Complete | 8-10 | All features implemented and integrated |
| M5: Tested & Documented | 11 | Comprehensive tests and documentation |
| M6: Production Ready | 12 | Optimized, polished, bug-free |

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

- [ ] All 5 modules (lockscreen, statusbar, launcher, notifications, controlcenter) functional
- [ ] Multi-monitor support working correctly
- [ ] Theme system supports runtime switching
- [ ] All services working as singletons with proper signals
- [ ] Test suite passes (unit, integration, manual)
- [ ] Documentation complete and accurate
- [ ] No memory leaks after 24-hour test
- [ ] Startup time < 2 seconds
- [ ] Hot-reload works without state loss
- [ ] Catppuccin Mocha theme applied consistently

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

*Last updated: 2026-04-12*
*Version: 1.0*
*Status: Ready for implementation*
