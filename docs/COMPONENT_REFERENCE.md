# Qypr Component Documentation

This document provides comprehensive documentation for all atomic components, molecules, layouts, and animations in the Qypr shell system.

---

## Table of Contents

1. [Atoms](#atoms)
   - [Button](#button)
   - [IconButton](#iconbutton)
   - [TextButton](#textbutton)
   - [Icon](#icon)
   - [Label](#label)
   - [Input](#input)
   - [Slider](#slider)
   - [ProgressBar](#progressbar)
   - [Card](#card)
   - [Badge](#badge)
   - [Tooltip](#tooltip)
   - [Divider](#divider)
   - [Spinner](#spinner)

2. [Molecules](#molecules)
   - [Clock](#clock)
   - [VolumeControl](#volumecontrol)
   - [BrightnessControl](#brightnesscontrol)
   - [NetworkIndicator](#networkindicator)
   - [BatteryIndicator](#batteryindicator)
   - [WorkspaceIndicator](#workspaceindicator)
   - [MediaWidget](#mediawidget)
   - [SearchInput](#searchinput)
   - [ListItem](#listitem)
   - [AppGridItem](#appgriditem)
   - [NotificationItem](#notificationitem)
   - [PowerMenu](#powermenu)
   - [UserMenu](#usermenu)

3. [Layouts](#layouts)
   - [PanelLayout](#panellayout)
   - [PopupLayout](#popuplayout)
   - [DrawerLayout](#drawerlayout)
   - [GridLayout](#gridlayout)
   - [LayerLayout](#layerlayout)

4. [Animations](#animations)
   - [FadeAnimation](#fadeanimation)
   - [SlideAnimation](#slideanimation)
   - [ScaleAnimation](#scaleanimation)
   - [SpringAnimation](#springanimation)

---

## Atoms

### Button

Foundation button component with hover, pressed, and disabled states.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `text` | `string` | `""` | Button label text |
| `icon` | `string` | `""` | Optional icon source |
| `size` | `string` | `"medium"` | Size: "small", "medium", "large" |
| `variant` | `string` | `"filled"` | Style: "filled", "outlined", "ghost" |
| `enabled` | `bool` | `true` | Whether button is interactive |
| `loading` | `bool` | `false` | Show loading spinner |

**Signals:**
- `clicked()` - Emitted when button is clicked
- `pressed()` - Emitted when button is pressed
- `released()` - Emitted when button is released

**Usage:**
```qml
Button {
    text: "Click Me"
    onClicked: console.log("Clicked")
}
```

---

### IconButton

Icon-only button variant with circular and rounded options.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `icon` | `string` | required | Icon source |
| `size` | `string` | `"medium"` | Size: "small", "medium", "large" |
| `rounded` | `bool` | `false` | Use rounded corners |
| `tooltipText` | `string` | `""` | Optional tooltip |

**Signals:**
- `clicked()` - Emitted when clicked
- `hovered` - Read-only hover state

---

### TextButton

Text button with optional icon and variants.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `text` | `string` | required | Button text |
| `icon` | `string` | `""` | Optional icon |
| `variant` | `string` | `"primary"` | "primary", "secondary", "tertiary", "destructive" |

---

### Icon

Icon wrapper with theme support.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `name` | `string` | required | Icon name |
| `size` | `int` | `24` | Icon size in pixels |
| `color` | `color` | theme.text | Icon color |

---

### Label

Text label with theme typography.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `text` | `string` | required | Text content |
| `variant` | `string` | `"body"` | "h1", "h2", "h3", "h4", "body", "caption" |
| `fontSize` | `int` | auto | Override font size |
| `fontWeight` | `int` | auto | Font weight |
| `color` | `color` | theme.textPrimary | Text color |

---

### Input

Base text input with validation support.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `placeholder` | `string` | `""` | Placeholder text |
| `text` | `string` | `""` | Input text |
| `passwordMode` | `bool` | `false` | Password masking |
| `showClear` | `bool` | `false` | Show clear button |
| `error` | `bool` | `false` | Error state |
| `leftIcon` | `string` | `""` | Left icon |
| `rightIcon` | `string` | `""` | Right icon |

**Signals:**
- `textChanged(string text)` - Emitted on text change
- `accepted()` - Emitted on Enter key
- `cleared()` - Emitted when cleared

---

### Slider

Custom styled slider with value display.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `value` | `real` | `0` | Current value (0-1) |
| `orientation` | `int` | `Qt.Horizontal` | Orientation |
| `stepSize` | `real` | `0.01` | Step increment |
| `showValue` | `bool` | `false` | Show value label |

**Signals:**
- `valueChanged(real value)` - Emitted on value change

---

### ProgressBar

Linear progress indicator.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `value` | `real` | `0` | Progress (0-1), 0 for indeterminate |
| `indeterminate` | `bool` | `false` | Indeterminate mode |

---

### Card

Card/container component.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `glassmorphic` | `bool` | `false` | Glassmorphism style |
| `hoverable` | `bool` | `false` | Hover effects |
| `padding` | `int` | `16` | Content padding |
| `radius` | `int` | theme.medium | Corner radius |

---

### Badge

Notification badge.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `count` | `int` | `0` | Badge count (99+ shown) |
| `variant` | `string` | `"info"` | "info", "success", "warning", "error" |
| `dotOnly` | `bool` | `false` | Show dot only |

---

### Tooltip

Tooltip component with auto-positioning.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `text` | `string` | required | Tooltip text |
| `position` | `string` | `"top"` | "top", "bottom", "left", "right" |
| `visible` | `bool` | `false` | Visibility |
| `delay` | `int` | `500` | Show delay (ms) |

---

### Divider

Visual separator.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `orientation` | `int` | `Qt.Horizontal` | Orientation |
| `label` | `string` | `""` | Optional label |
| `thickness` | `int` | `1` | Line thickness |

---

### Spinner

Loading spinner.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `size` | `int` | `24` | Spinner size |
| `color` | `color` | theme.text | Spinner color |

---

## Molecules

### Clock

Time/date display with live updates.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `showTime` | `bool` | `true` | Show time |
| `showDate` | `bool` | `true` | Show date |
| `format24Hour` | `bool` | `true` | 24-hour format |
| `showSeconds` | `bool` | `false` | Show seconds |
| `fontSize` | `int` | theme.size2xl | Time font size |

**Signals:**
- `clicked()` - Emitted when clicked

---

### VolumeControl

Volume slider with icon.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `orientation` | `int` | `Qt.Horizontal` | Orientation |

---

### BrightnessControl

Brightness slider with icon.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `width` | `int` | `200` | Control width |

---

### NetworkIndicator

Network status icon.

**Signals:**
- `clicked()` - Emitted when clicked

---

### BatteryIndicator

Battery icon with percentage.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `showPercentage` | `bool` | `true` | Show percentage text |
| `criticalThreshold` | `int` | `20` | Critical level |

---

### WorkspaceIndicator

Hyprland workspace dots.

**Signals:**
- `workspaceClicked(int index)` - Emitted on workspace click

---

### MediaWidget

Media player mini widget.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `width` | `int` | `280` | Widget width |

---

### SearchInput

Search with icon and clear.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `placeholder` | `string` | `"Search..."` | Placeholder text |
| `showShortcut` | `bool` | `false` | Show keyboard shortcut |

---

### ListItem

List item with icon and actions.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `title` | `string` | required | Main text |
| `subtitle` | `string` | `""` | Secondary text |
| `icon` | `string` | `""` | Optional icon |
| `showArrow` | `bool` | `false` | Show arrow indicator |
| `selected` | `bool` | `false` | Selected state |

**Signals:**
- `clicked()` - Emitted when clicked

---

### AppGridItem

Application grid tile.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `appName` | `string` | required | App name |
| `appIcon` | `string` | `""` | App icon |

**Signals:**
- `launched()` - Emitted when launched

---

### NotificationItem

Single notification display.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `appName` | `string` | `""` | App name |
| `title` | `string` | required | Notification title |
| `body` | `string` | `""` | Notification body |
| `timestamp` | `string` | `""` | Time string |

**Signals:**
- `dismissed()` - Emitted when dismissed
- `actionTriggered(string action)` - Emitted on action

---

### PowerMenu

Power options dropdown.

**Signals:**
- `shutdown()` - Emitted on shutdown
- `reboot()` - Emitted on reboot
- `suspend()` - Emitted on suspend
- `logout()` - Emitted on logout

---

### UserMenu

User menu with avatar.

**Signals:**
- `settingsClicked()` - Emitted on settings
- `lockClicked()` - Emitted on lock
- `logoutClicked()` - Emitted on logout

---

## Layouts

### PanelLayout

Horizontal panel layout for status bars.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `spacing` | `int` | `4` | Space between items |
| `alignment` | `string` | `"left"` | "left", "center", "right" |
| `leftMargin` | `int` | `0` | Left padding |
| `rightMargin` | `int` | `0` | Right padding |
| `verticalAlignment` | `string` | `"center"` | "top", "center", "bottom" |

---

### PopupLayout

Centered popup/dialog layout.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `visible` | `bool` | `false` | Visibility |
| `modal` | `bool` | `true` | Show backdrop |
| `backdropOpacity` | `real` | `0.5` | Backdrop opacity |
| `closeOnBackdrop` | `bool` | `true` | Close on backdrop click |
| `animationDuration` | `int` | `300` | Animation duration (ms) |

**Signals:**
- `opened()` - Emitted when opened
- `closed()` - Emitted when closed
- `backdropClicked()` - Emitted on backdrop click

---

### DrawerLayout

Slide-out drawer layout.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `visible` | `bool` | `false` | Visibility |
| `position` | `string` | `"left"` | "left", "right", "top", "bottom" |
| `drawerWidth` | `int` | `300` | Width (horizontal) |
| `drawerHeight` | `int` | `400` | Height (vertical) |
| `backdropEnabled` | `bool` | `true` | Show backdrop |
| `backdropOpacity` | `real` | `0.5` | Backdrop opacity |

**Signals:**
- `opened()` - Emitted when opened
- `closed()` - Emitted when closed

---

### GridLayout

Responsive grid layout.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `columns` | `int` | `4` | Number of columns |
| `spacing` | `int` | `8` | Space between items |
| `cellWidth` | `int` | `0` | Fixed cell width (0 = auto) |
| `cellHeight` | `int` | `0` | Fixed cell height (0 = auto) |

---

### LayerLayout

Z-index layering helper.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `defaultLayer` | `string` | `"content"` | Default layer |
| `layers` | `var` | see below | Layer z-index map |

**Layer Z-Indices:**
| Layer | Z-Index |
|-------|---------|
| `background` | 0 |
| `content` | 10 |
| `overlay` | 20 |
| `modal` | 30 |
| `tooltip` | 40 |
| `top` | 50 |

---

## Animations

### FadeAnimation

Fade in/out animation.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `target` | `Item` | required | Item to animate |
| `duration` | `int` | `300` | Duration (ms) |
| `fadeIn` | `bool` | `true` | Animate to visible |
| `fadeOut` | `bool` | `false` | Animate to hidden |
| `fromOpacity` | `real` | `0/1` | Starting opacity |
| `toOpacity` | `real` | `1/0` | Ending opacity |
| `easing` | `int` | `Easing.InOutQuad` | Easing curve |

**Signals:**
- `started()` - Emitted on start
- `finished()` - Emitted on completion

---

### SlideAnimation

Slide transition animation.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `target` | `Item` | required | Item to animate |
| `direction` | `string` | `"left"` | "left", "right", "up", "down" |
| `distance` | `int` | `0` | Distance (0 = auto) |
| `duration` | `int` | `300` | Duration (ms) |
| `easing` | `int` | `Easing.OutQuad` | Easing curve |
| `slideIn` | `bool` | `true` | Slide into view |
| `slideOut` | `bool` | `false` | Slide out of view |

**Signals:**
- `started()` - Emitted on start
- `finished()` - Emitted on completion

---

### ScaleAnimation

Scale effect animation.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `target` | `Item` | required | Item to animate |
| `scaleFrom` | `real` | `0.8` | Starting scale |
| `scaleTo` | `real` | `1.0` | Ending scale |
| `duration` | `int` | `200` | Duration (ms) |
| `easing` | `int` | `Easing.OutBack` | Easing curve |
| `originX` | `real` | `0.5` | Origin X (0-1) |
| `originY` | `real` | `0.5` | Origin Y (0-1) |
| `scaleIn` | `bool` | `true` | Scale into view |
| `scaleOut` | `bool` | `false` | Scale out of view |

**Signals:**
- `started()` - Emitted on start
- `finished()` - Emitted on completion

---

### SpringAnimation

Physics-based spring animation.

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `target` | `Item` | required | Item to animate |
| `property` | `string` | required | Property to animate |
| `from` | `real` | required | Starting value |
| `to` | `real` | required | Ending value |
| `spring` | `real` | `1.5` | Spring stiffness |
| `damping` | `real` | `0.15` | Damping coefficient |
| `mass` | `real` | `1.0` | Object mass |
| `epsilon` | `real` | `0.01` | Convergence threshold |
| `duration` | `int` | `500` | Maximum duration (ms) |

**Signals:**
- `started()` - Emitted on start
- `finished()` - Emitted on completion

---

## Testing

Visual test components are available in `tests/manual/`:

- `AtomTests.qml` - Test all atom components
- `MoleculeTests.qml` - Test all molecule components
- `LayoutTests.qml` - Test all layout components
- `AnimationTests.qml` - Test all animation components
- `ComponentShowcase.qml` - Comprehensive showcase of all components

Run tests with:
```bash
./scripts/run.sh --mode full
```

---

*Last updated: 2026-04-12*
