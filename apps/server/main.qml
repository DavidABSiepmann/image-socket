import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import ImageSocket 1.0
import ImageServerBridge 1.0

/// Main Application Window: Server UI orchestration.
///
/// This window composes the refactored UI components (VideoDisplayArea, ControlPanel, StatusBar)t.
/// Uses Theme for consistent styling and responsive layout with dynamic sizing.
///
/// Architecture:
/// - Theme: Centralized styling (colors, fonts, spacing).
/// - VideoDisplayArea: Displays live images from the server (expands proportionally).
/// - ControlPanel: Manages Start/Stop/Reset buttons (state-driven).
/// - StatusBar: Displays status messages and error feedback.
///
/// Layout: Responsive ColumnLayout with no fixed size; window is resizable.
///
ApplicationWindow {
    id: appWindow
    visible: true
    
    property alias themeMode: mainTheme.themeMode

    // Window sizing: responsive with minimum constraints
    minimumWidth: mainTheme.windowMinWidth
    minimumHeight: mainTheme.windowMinHeight
    width: 800
    height: 600
    title: "Image Socket Server"
    color: mainTheme.backgroundColor

    function toogleTheme() {
        mainTheme.themeMode = (mainTheme.themeMode === "light") ? "dark" : "light"
    }

    // Use Theme for all styling
    Theme {
        id: mainTheme
    }
    
    /// Main layout: video display on top, controls and status at bottom.
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        /// Video display area (takes remaining space after controls).
        VideoDisplayArea {
            id: videoArea
            themeMode: mainTheme.themeMode
            statusBar: statusBar
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        
        /// Control panel: Start, Stop, Reset buttons.
        ControlPanel {
            id: controlPanel
            themeMode: mainTheme.themeMode
            Layout.fillWidth: true
            Layout.preferredHeight: mainTheme.controlPanelHeight

            onToogleTheme: {
                appWindow.toogleTheme()
            }
        }
        
        /// Status bar: Displays messages and error feedback.
        StatusBar {
            id: statusBar
            themeMode: mainTheme.themeMode
            Layout.fillWidth: true
            Layout.preferredHeight: mainTheme.statusBarHeight
        }
        // UI state control object for Clients panel
        QtObject {
            id: clients
            property bool panelVisible: false
        }

        QtObject {
            id: eventAggregator
            property var pending: ({})
            property int aggregationWindowMs: 1200

            function aggregateEvent(code, message) {
                var key = code.toString();
                if (!pending[key]) pending[key] = { count: 0, msgs: [] };
                pending[key].count++;
                pending[key].msgs.push(message);
                aggregationTimer.restart();
            }

            function flushEvents() {
                for (var k in pending) {
                    var entry = pending[k];
                    if (entry.count > 1) {
                        appWindow.toast.show(entry.count + " events", 2000);
                    } else {
                        appWindow.toast.show(entry.msgs[0], 2000);
                    }
                }
                pending = {};
            }
        }

        Timer {
            id: aggregationTimer
            interval: eventAggregator.aggregationWindowMs
            onTriggered: eventAggregator.flushEvents()
            repeat: false
        }

        // Also listen to high-level client signals to show immediate toasts
        Connections {
            target: imageSocket
            function onClientConnectedWithAlias(clientId, alias) {
                var msg = EventMessages.getMessage(ImageServerBridge.ClientConnected);
                msg = msg.replace("{alias}", alias ? alias : clientId);
                appWindow.toast.show(msg, 2400);
            }
            function onClientDisconnectedWithAlias(clientId, alias) {
                var msg = EventMessages.getMessage(ImageServerBridge.ClientDisconnected);
                msg = msg.replace("{alias}", alias ? alias : clientId);
                appWindow.toast.show(msg, 2400);
            }
        }

        /// Diagnostics panel (hidden by default)
        DiagnosticsPanel {
            id: diagnosticsPanel
            themeMode: mainTheme.themeMode
            visible: diagnostics.panelVisible
            Layout.fillWidth: true
            Layout.preferredHeight: mainTheme.controlPanelHeight * 3
            onVisibleChanged: {
                if (visible) clients.panelVisible = false
            }
        }

        /// Clients panel (hidden by default)
        ClientsPanel {
            id: clientsPanel
            themeMode: mainTheme.themeMode
            visible: clients.panelVisible
            Layout.fillWidth: true
            Layout.preferredHeight: mainTheme.controlPanelHeight * 3
            onVisibleChanged: {
                if (visible) diagnostics.panelVisible = false
            }
        }
    }

    ToastNotification {
        id: toast
        activeTheme: mainTheme
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 20
        anchors.bottomMargin: 24
        z: 2000
    }
    
    /// Connect C++ signals to the state manager and UI components.
    Connections {
        target: imageSocket
        
            /// Transition to IDLE state when connection is lost and reset metrics.
        function onConnectionLost() {
            videoArea.hasImage = false
            statusBar.reset()
        }

        // Receive generic events from backend and log and display toast via aggregator
        function onEventOccurred(code, details) {
            // Map code -> localized message
            var msg = (typeof EventMessages !== 'undefined') ? EventMessages.getMessage(code, details) : ("Event " + code);
            if (details && details.alias) {
                msg = msg.replace("{alias}", details.alias);
            }

            // Aggregation: group connect/disconnect events
            if (code === ImageServerBridge.ClientConnected || code === ImageServerBridge.ClientDisconnected) {
                eventAggregator.aggregateEvent(code, msg);
            } else {
                toast.show(msg, 2500);
            }

            console.log("EVENT:", code, JSON.stringify(details), "->", msg)
        }
    }
    
    /// Auto-start the server on application launch.
    Component.onCompleted: {
        // Start the WebSocket bridge (use ephemeral port 0 so OS picks available port)
        imageSocket.start()
    }
}
