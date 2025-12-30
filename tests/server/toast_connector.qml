import QtQuick 2.15

Item {
    id: root
    property var bridge

    // Load Theme and ToastNotification directly from resources to avoid relying on the module import
    Loader {
        id: themeLoader
        source: "qrc:/ImageSocket/Theme.qml"
    }

    Loader {
        id: toastLoader
        source: "qrc:/ImageSocket/ToastNotification.qml"
        onLoaded: {
            if (toastLoader.item && themeLoader.item) {
                toastLoader.item.activeTheme = themeLoader.item
            }
        }
    }

    // Convenience alias for test code (searches children for visible item)
    property alias toast: toastLoader.item

    Connections { target: bridge
        function onClientConnectedWithAlias(clientId, alias) {
            if (toast) toast.show("connected: " + alias, 1500)
        }
        function onClientDisconnectedWithAlias(clientId, alias) {
            if (toast) toast.show("disconnected: " + alias, 1500)
        }
        function onEventOccurred(code, details) {
            if (toast) toast.show("event:" + code, 1500)
        }
    }
}