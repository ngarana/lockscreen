// SearchInput.qml - Search with Icon and Clear Molecule
//
// Search input field with search icon, clear button,
// and keyboard shortcut support.
//
// Usage:
//   SearchInput {
//       placeholder: "Search applications..."
//       onSearch: performSearch(text)
//   }
//
// Properties:
//   - text: string - Search text (two-way binding)
//   - placeholder: string - Placeholder text
//   - showClearButton: bool - Show clear button (default: true)
//   - showRecentSearches: bool - Show recent searches dropdown
//   - recentSearches: var - Array of recent search strings
//
// Signals:
//   - search(text): Emitted when search is submitted
//   - textChanged(text): Emitted when text changes
//   - cleared(): Emitted when search is cleared

import QtQuick
import QtQuick.Layouts
import "../atoms" as Atoms
import "../theme" as Theme

ColumnLayout {
    id: root

    // Public API
    property alias text: input.text
    property string placeholder: "Search..."
    property bool showClearButton: true
    property bool showRecentSearches: false
    property var recentSearches: []

    signal search(string text)
    signal textChanged(string text)
    signal cleared()

    // Internal
    property bool _showingRecent: false

    // Layout
    spacing: 4

    // Search input field
    Atoms.Input {
        id: input
        Layout.fillWidth: true
        leftIcon: "search"
        clearable: root.showClearButton
        placeholder: root.placeholder

        onAccepted: {
            root.search(text)
            if (text !== "" && !root.recentSearches.includes(text)) {
                root.recentSearches.unshift(text)
                if (root.recentSearches.length > 5) {
                    root.recentSearches.pop()
                }
            }
        }

        onTextChanged: {
            root.textChanged(text)
            root._showingRecent = root.showRecentSearches && text === "" && root.recentSearches.length > 0
        }
    }

    // Recent searches dropdown
    Atoms.Card {
        id: recentDropdown
        Layout.fillWidth: true
        visible: root._showingRecent
        padding: 8
        glassmorphic: true
        elevated: true

        ColumnLayout {
            anchors.fill: parent
            spacing: 4

            Atoms.Label {
                text: "Recent Searches"
                fontSize: Theme.Theme.typography.sizeXs
                color: Theme.Theme.colors.textMuted
            }

            Repeater {
                model: root.recentSearches

                Rectangle {
                    Layout.fillWidth: true
                    height: 28
                    radius: Theme.Theme.radius.small
                    color: mouseArea.containsMouse ? Theme.Theme.colors.glassHover : "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 8

                        Atoms.Icon {
                            source: "history"
                            size: 14
                            color: Theme.Theme.colors.textMuted
                        }

                        Atoms.Label {
                            Layout.fillWidth: true
                            text: modelData
                            fontSize: Theme.Theme.typography.sizeSm
                            color: Theme.Theme.colors.textSecondary
                            truncate: true
                        }
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: {
                            input.text = modelData
                            root.search(modelData)
                            root._showingRecent = false
                        }
                    }
                }
            }
        }
    }

    // Clear search
    function clear() {
        input.text = ""
        root._showingRecent = false
        root.cleared()
    }

    // Focus input
    function focusSearch() {
        input.forceActiveFocus()
    }
}
