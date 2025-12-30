import QtQuick 2.15

/**
 * MockBackend.qml
 * 
 * Simulates the C++ backend exposed to QML tests (e.g., ImageServerBridge).
 * Provides properties, signals, and methods that mirror the real backend interface.
 * 
 * Usage:
 *   MockBackend { id: mockBackend }
 *   // In test: mockBackend.startServer(); mockBackend.serverStateChanged();
 * 
 * NOTE: This is a pure mock with NO business logic. Tests control state directly.
 */
QtObject {
    id: root

    // ========================================================================
    // Properties - Direct state without validation
    // ========================================================================

    property string serverState: "Idle"              // Idle, Running, Error, etc.
    property string connectionState: "Disconnected"  // Disconnected, Connected, Connecting
    property string statusMessage: ""                // Current status text
    property int currentFps: 0                       // Measured FPS (for display)
    property int configuredFps: 30                   // User-configured FPS
    property string activeClientAlias: ""            // Currently active client alias
    property int clientCount: 0                      // Number of connected clients
    property var clientList: []                      // List of {alias, id} objects

    // ========================================================================
    // Signals - Emitted when state changes (tests can QSignalSpy these)
    // ========================================================================

    signal serverStateChanged(string newState)
    signal connectionStateChanged(string newState)
    signal statusMessageChanged(string message)
    signal fpsChanged(int fps)
    signal configuredFpsChanged(int fps)
    signal activeClientChanged(string alias)
    signal clientCountChanged(int count)
    signal errorOccurred(string errorMessage)

    // ========================================================================
    // Methods - Change state and emit signals (NO validation/business logic)
    // ========================================================================

    /**
     * Start the server (mock only - no actual network operation)
     */
    function startServer() {
        serverState = "Running"
        connectionState = "Connected"
        statusMessage = "Server running"
        serverStateChanged(serverState)
        connectionStateChanged(connectionState)
        statusMessageChanged(statusMessage)
    }

    /**
     * Stop the server
     */
    function stopServer() {
        serverState = "Idle"
        connectionState = "Disconnected"
        statusMessage = "Server stopped"
        activeClientAlias = ""
        clientList = []
        clientCount = 0
        
        serverStateChanged(serverState)
        connectionStateChanged(connectionState)
        statusMessageChanged(statusMessage)
        activeClientChanged(activeClientAlias)
        clientCountChanged(clientCount)
    }

    /**
     * Set configured FPS (mock - no actual device control)
     */
    function setFps(value) {
        configuredFps = value
        currentFps = value
        configuredFpsChanged(configuredFps)
        fpsChanged(currentFps)
        statusMessageChanged("FPS set to " + value)
    }

    /**
     * Set server state explicitly (used in tests)
     */
    function setServerState(newState) {
        serverState = newState
        serverStateChanged(serverState)
    }

    /**
     * Set active client alias explicitly (used in tests)
     */
    function setActiveClient(alias) {
        activeClientAlias = alias
        activeClientChanged(alias)
    }

    /**
     * Set status message explicitly (used in tests)
     */
    function setStatusMessage(message) {
        statusMessage = message
        statusMessageChanged(message)
    }

    /**
     * Simulate an error condition (for error message testing)
     */
    function simulateError(message) {
        serverState = "Error"
        statusMessage = message
        errorOccurred(message)
        serverStateChanged(serverState)
        statusMessageChanged(statusMessage)
    }

    /**
     * Simulate a client connection
     */
    function simulateClientConnect(alias) {
        // Add to client list
        var newClient = { alias: alias, id: clientList.length + 1 }
        clientList.push(newClient)
        
        // Update state
        activeClientAlias = alias
        clientCount = clientList.length
        connectionState = "Connected"
        statusMessage = "Client connected: " + alias
        
        // Emit signals
        clientCountChanged(clientCount)
        activeClientChanged(activeClientAlias)
        connectionStateChanged(connectionState)
        statusMessageChanged(statusMessage)
    }

    /**
     * Simulate a client disconnection
     */
    function simulateClientDisconnect() {
        if (clientList.length > 0) {
            clientList.pop()
            clientCount = clientList.length
            
            // Update active client to next in list (or empty)
            if (clientList.length > 0) {
                activeClientAlias = clientList[clientList.length - 1].alias
            } else {
                activeClientAlias = ""
                connectionState = "Disconnected"
            }
            
            statusMessage = "Client disconnected"
            
            // Emit signals
            clientCountChanged(clientCount)
            activeClientChanged(activeClientAlias)
            connectionStateChanged(connectionState)
            statusMessageChanged(statusMessage)
        }
    }

    /**
     * Clear all state (useful for test cleanup)
     */
    function reset() {
        serverState = "Idle"
        connectionState = "Disconnected"
        statusMessage = ""
        currentFps = 0
        configuredFps = 30
        activeClientAlias = ""
        clientCount = 0
        clientList = []
    }
}
