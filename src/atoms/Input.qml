// Input.qml - Text Input Component
//
// Base text input with placeholder, validation states, and clear button.
// Supports password mode and theme integration.
//
// Usage:
//   Input {
//       placeholder: "Enter your name"
//       onTextChanged: console.log(text)
//   }
//
// Properties:
//   - text: string - Input text (two-way binding)
//   - placeholder: string - Placeholder text
//   - password: bool - Mask input as password
//   - clearable: bool - Show clear button
//   - validator: var - Input validator object
//   - maxLength: int - Maximum character length
//   - size: string - "small", "medium", "large"
//   - leftIcon: string - Optional icon on left side
//   - rightIcon: string - Optional icon on right side
//
// Signals:
//   - accepted(): Emitted when Enter/Return is pressed
//   - textChanged(text: string): Emitted when text changes
//   - editingFinished(): Emitted when focus is lost

import QtQuick
import QtQuick.Controls.Basic
import "../theme" as Theme

Rectangle {
    id: root

    // Public API
    property alias text: textInput.text
    property string placeholder: ""
    property bool password: false
    property bool clearable: false
    property var validator: null
    property int maxLength: 0
    property string size: "medium" // small, medium, large
    property string leftIcon: ""
    property string rightIcon: ""
    property bool readOnly: false

    signal accepted()
    signal textChanged(string text)
    signal editingFinished()

    // Internal state
    property bool _isFocused: false
    property bool _isHovered: false
    property bool _hasError: false

    // Size configuration
    readonly property var _sizes: ({
        small: { height: 32, padding: 12, iconSize: 14, fontSize: Theme.ThemeEngine.typography.sizeSm },
        medium: { height: 40, padding: 16, iconSize: 16, fontSize: Theme.ThemeEngine.typography.sizeMd },
        large: { height: 48, padding: 20, iconSize: 18, fontSize: Theme.ThemeEngine.typography.sizeLg }
    })

    readonly property var _currentSize: _sizes[size] || _sizes.medium

    // Layout
    implicitWidth: 200
    implicitHeight: _currentSize.height
    radius: Theme.ThemeEngine.radius.medium

    // Visual state
    color: {
        if (_hasError) return Qt.rgba(0.95, 0.3, 0.3, 0.1)
        if (_isFocused) return Theme.ThemeEngine.colors.glassActive
        if (_isHovered) return Theme.ThemeEngine.colors.glassHover
        return Theme.ThemeEngine.colors.glass
    }

    border.width: 1
    border.color: {
        if (_hasError) return Theme.ThemeEngine.colors.error
        if (_isFocused) return Theme.ThemeEngine.colors.primary
        return Theme.ThemeEngine.colors.glassBorder
    }

    Behavior on color {
        ColorAnimation { duration: Theme.ThemeEngine.animation.fast }
    }

    Behavior on border.color {
        ColorAnimation { duration: Theme.ThemeEngine.animation.fast }
    }

    // Row layout for icon + input + clear button
    Row {
        id: row
        anchors.fill: parent
        anchors.leftMargin: leftIcon !== "" ? _currentSize.padding / 2 : _currentSize.padding
        anchors.rightMargin: (clearable || rightIcon !== "") ? _currentSize.padding / 2 : _currentSize.padding
        spacing: 8

        // Left icon
        Icon {
            id: leftIconItem
            visible: root.leftIcon !== ""
            anchors.verticalCenter: parent.verticalCenter
            source: root.leftIcon
            size: root._currentSize.iconSize
            color: root._isFocused ? Theme.ThemeEngine.colors.primary : Theme.ThemeEngine.colors.textSecondary
        }

        // Text input
        TextInput {
            id: textInput
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - (leftIconItem.visible ? leftIconItem.width + 8 : 0) -
                   (clearButton.visible || rightIconItem.visible ? clearButton.width + 8 : 0) -
                   (parent.spacing * ((leftIconItem.visible ? 1 : 0) + (clearButton.visible || rightIconItem.visible ? 1 : 0)))
            height: parent.height
            verticalAlignment: Text.AlignVCenter

            font.pixelSize: root._currentSize.fontSize
            font.family: Theme.ThemeEngine.typography.fontFamily
            color: Theme.ThemeEngine.colors.textPrimary
            selectionColor: Theme.ThemeEngine.colors.primary
            selectedTextColor: Theme.ThemeEngine.colors.crust

            echoMode: root.password ? TextInput.Password : TextInput.Normal
            passwordCharacter: "•"
            maximumLength: root.maxLength > 0 ? root.maxLength : 32767
            readOnly: root.readOnly

            validator: root.validator

            onAccepted: root.accepted()
            onTextChanged: root.textChanged(text)
            onEditingFinished: root.editingFinished()

            // Cursor
            cursorDelegate: Rectangle {
                width: 2
                color: Theme.ThemeEngine.colors.primary
                visible: textInput.cursorVisible
            }
        }

        // Placeholder
        Label {
            id: placeholderLabel
            anchors.verticalCenter: parent.verticalCenter
            x: textInput.x
            width: textInput.width
            text: root.placeholder
            color: Theme.ThemeEngine.colors.textMuted
            fontSize: root._currentSize.fontSize
            visible: textInput.text === "" && !root._isFocused
        }

        // Right icon (if no clear button)
        Icon {
            id: rightIconItem
            visible: root.rightIcon !== "" && !root.clearable
            anchors.verticalCenter: parent.verticalCenter
            source: root.rightIcon
            size: root._currentSize.iconSize
            color: Theme.ThemeEngine.colors.textSecondary
        }

        // Clear button
        IconButton {
            id: clearButton
            visible: root.clearable && textInput.text !== ""
            anchors.verticalCenter: parent.verticalCenter
            size: root._currentSize.height - 8
            iconSize: root._currentSize.iconSize - 2
            icon: "close" // Use system icon
            tooltip: "Clear"
            circular: true

            onClicked: {
                textInput.text = ""
                textInput.forceActiveFocus()
            }
        }
    }

    // Focus and hover handling
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.IBeamCursor

        onClicked: {
            textInput.forceActiveFocus()
            root._isFocused = true
        }

        onEntered: root._isHovered = true
        onExited: root._isHovered = false
    }

    // Focus monitoring
    on_ActiveFocusChanged: {
        _isFocused = textInput.activeFocus
    }

    // Helper to set error state
    function setError(hasError) {
        _hasError = hasError
    }

    // Helper to clear error state
    function clearError() {
        _hasError = false
    }

    // Helper to select all text
    function selectAll() {
        textInput.selectAll()
    }
}
