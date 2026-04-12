// LockController.qml - Session Lock and PAM Authentication Controller
//
// This singleton manages the session lock state and handles PAM authentication.
// It serves as the bridge between the UI layer and the system's security mechanisms.
//
// Responsibilities:
// - Manage session lock/unlock state via WlSessionLock
// - Handle PAM authentication flow
// - Provide authentication status to UI components
// - Emit signals for authentication events
//
// Usage:
//   LockController.authenticate(password)  // Start authentication
//   LockController.lock()                   // Lock the session
//   LockController.unlock()                 // Unlock the session
//
// Signals:
//   unlockSuccess()       - Emitted when authentication succeeds
//   unlockFailed(msg)     - Emitted when authentication fails
//   authenticationStarted()  - Emitted when auth begins
//   authenticationFinished() - Emitted when auth completes

pragma Singleton
import Quickshell
import Quickshell.Services.Pam
import QtQuick

QtObject {
    id: root

    // ========================================================================
    // Public Properties (Read-only for external use)
    // ========================================================================

    // Indicates if the session is currently locked
    // Maps to WlSessionLock.locked state
    readonly property bool isLocked: root.lockInstance ? root.lockInstance.locked : false

    // Indicates if the compositor has confirmed the lock is secure
    // True when all screens are covered by lock surfaces
    readonly property bool isSecure: root.lockInstance ? root.lockInstance.secure : false

    // Indicates if an authentication attempt is in progress
    // Use this to disable UI during authentication
    readonly property bool isAuthenticating: pamContext.active

    // Current status message to display to the user
    // e.g., "Authenticating...", "Authentication failed"
    property string statusMessage: ""

    // Indicates if the status message represents an error
    // Use for styling (e.g., red text for errors)
    property bool hasError: false

    // ========================================================================
    // Internal Properties
    // ========================================================================

    // Reference to the WlSessionLock instance (set by shell.qml)
    property var lockInstance: null

    // Tracks if PAM emitted an error (to avoid duplicate error handling)
    property bool pamHadError: false

    // ========================================================================
    // Signals
    // ========================================================================

    // Emitted when authentication succeeds and the session unlocks
    signal unlockSuccess()

    // Emitted when authentication fails
    // message: Description of the failure
    signal unlockFailed(string message)

    // Emitted when an authentication attempt starts
    signal authenticationStarted()

    // Emitted when an authentication attempt completes (success or failure)
    signal authenticationFinished()

    // ========================================================================
    // Public Methods
    // ========================================================================

    // Start PAM authentication with the provided password
    // password: The user's password input
    // Returns: true if authentication started, false if already authenticating or empty password
    function authenticate(password) {
        // Prevent duplicate authentication attempts
        if (root.isAuthenticating || password.length === 0) {
            return false
        }

        // Update UI state
        root.statusMessage = "Authenticating..."
        root.hasError = false
        root.pamHadError = false
        root.authenticationStarted()

        // Store password for PAM callback
        pamContext.passwordBuffer = password
        const started = pamContext.start()

        // Handle failure to start authentication
        if (!started) {
            pamContext.passwordBuffer = ""
            root.statusMessage = "Unable to start authentication"
            root.hasError = true
            root.authenticationFinished()
            root.unlockFailed(root.statusMessage)
        }

        return started
    }

    // Lock the session
    // Requires lockInstance to be set via setLockInstance()
    function lock() {
        if (root.lockInstance && !root.lockInstance.locked) {
            root.lockInstance.locked = true
        }
        root.statusMessage = ""
        root.hasError = false
    }

    // Unlock the session
    // Should typically be called after successful authentication
    function unlock() {
        if (root.lockInstance && root.lockInstance.locked) {
            root.lockInstance.locked = false
        }
        root.statusMessage = ""
        root.hasError = false
    }

    // Set the WlSessionLock instance reference
    // Called from shell.qml during Component.onCompleted
    // lock: The WlSessionLock object from shell.qml
    function setLockInstance(lock) {
        root.lockInstance = lock
    }

    // ========================================================================
    // PAM Context (Internal)
    // ========================================================================

    property var pamContext: PamContext {
        id: pamContext

        // Use the 'login' PAM configuration
        // This is the standard configuration for local authentication
        // Custom configs can be placed in /etc/pam.d/
        config: "login"

        // Temporary storage for password during auth flow
        // Cleared immediately after use for security
        property string passwordBuffer: ""

        // Called when PAM sends a message (e.g., "Password:")
        // We respond with the stored password when a response is required
        onPamMessage: {
            if (responseRequired) {
                respond(passwordBuffer)
            }
        }

        // Called when authentication completes
        // result: PamResult.Success, PamResult.Failed, or PamResult.Error
        onCompleted: function(result) {
            root.authenticationFinished()

            if (result === PamResult.Success) {
                // Authentication successful
                root.statusMessage = "Unlocking..."
                root.hasError = false
                root.unlockSuccess()
                root.unlock()
            } else if (result === PamResult.Failed) {
                // Wrong password
                root.statusMessage = "Authentication failed"
                root.hasError = true
                root.unlockFailed("Authentication failed")
            } else if (!root.pamHadError) {
                // Other error (not already handled by onError)
                root.statusMessage = "Authentication error"
                root.hasError = true
                root.unlockFailed("Authentication error")
            }

            // Cleanup
            root.pamHadError = false
            passwordBuffer = ""
        }

        // Called when PAM encounters an error
        // error: PamError enum describing the error
        onError: function(error) {
            root.pamHadError = true
            root.statusMessage = "Error: " + error
            root.hasError = true
            root.unlockFailed(error.toString())
            passwordBuffer = ""
        }
    }
}
