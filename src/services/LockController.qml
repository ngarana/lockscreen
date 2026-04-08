pragma Singleton
import Quickshell
import Quickshell.Services.Pam
import QtQuick

QtObject {
    id: root

    readonly property bool isLocked: root.lockInstance ? root.lockInstance.locked : false
    readonly property bool isSecure: root.lockInstance ? root.lockInstance.secure : false
    readonly property bool isAuthenticating: pamContext.active
    property string statusMessage: ""
    property bool hasError: false
    property var lockInstance: null
    property bool pamHadError: false

    signal unlockSuccess()
    signal unlockFailed(string message)
    signal authenticationStarted()
    signal authenticationFinished()

    function authenticate(password) {
        if (root.isAuthenticating || password.length === 0) {
            return false
        }

        root.statusMessage = "Authenticating..."
        root.hasError = false
        root.pamHadError = false
        root.authenticationStarted()

        pamContext.passwordBuffer = password
        const started = pamContext.start()

        if (!started) {
            pamContext.passwordBuffer = ""
            root.statusMessage = "Unable to start authentication"
            root.hasError = true
            root.authenticationFinished()
            root.unlockFailed(root.statusMessage)
        }

        return started
    }

    function lock() {
        if (root.lockInstance && !root.lockInstance.locked) {
            root.lockInstance.locked = true
        }

        root.statusMessage = ""
        root.hasError = false
    }

    function unlock() {
        if (root.lockInstance && root.lockInstance.locked) {
            root.lockInstance.locked = false
        }

        root.statusMessage = ""
        root.hasError = false
    }

    function setLockInstance(lock) {
        root.lockInstance = lock
    }

    property var pamContext: PamContext {
        id: pamContext
        config: "login"

        property string passwordBuffer: ""

        onPamMessage: {
            if (responseRequired) {
                respond(passwordBuffer)
            }
        }

        onCompleted: function(result) {
            root.authenticationFinished()

            if (result === PamResult.Success) {
                root.statusMessage = "Unlocking..."
                root.hasError = false
                root.unlockSuccess()
                root.unlock()
            } else if (result === PamResult.Failed) {
                root.statusMessage = "Authentication failed"
                root.hasError = true
                root.unlockFailed("Authentication failed")
            } else if (!root.pamHadError) {
                root.statusMessage = "Authentication error"
                root.hasError = true
                root.unlockFailed("Authentication error")
            }

            root.pamHadError = false
            passwordBuffer = ""
        }

        onError: function(error) {
            root.pamHadError = true
            root.statusMessage = "Error: " + error
            root.hasError = true
            root.unlockFailed(error.toString())
            passwordBuffer = ""
        }
    }
}
