import Cocoa
import SystemExtensions

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate, OSSystemExtensionRequestDelegate {

    let driverID = "com.example.apple-samplecode.KeyboardDriver5R9PKFT37Q"

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Initialize the application and activate the driver.
        activateDriver()
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Tear down the application.
    }
    
    // Method to activate the system extension
    func activateDriver() {
        let request = OSSystemExtensionRequest.activationRequest(forExtensionWithIdentifier: driverID, queue: DispatchQueue.main)
        request.delegate = self
        let extensionManager = OSSystemExtensionManager.shared
        extensionManager.submitRequest(request)
    }
    
    // Called when the request finishes successfully
    func request(_ request: OSSystemExtensionRequest, didFinishWithResult result: OSSystemExtensionRequest.Result) {
        Swift.print("Request finished successfully")
    }
    
    // Called when the request fails with an error
    func request(_ request: OSSystemExtensionRequest, didFailWithError error: Error) {
        Swift.print("Request failed. \(error)")
        
        // Handle the failure in a more meaningful way
        // For example, show an alert to the user or try to reconnect
        handleError(error)
    }
    
    // Called when the request needs user approval
    func requestNeedsUserApproval(_ request: OSSystemExtensionRequest) {
        Swift.print("Request needs approval.")
        // Optionally, inform the user they need to approve the system extension in the Security & Privacy settings
    }
    
    // Handle catastrophic errors when the extension is replaced or fails
    func request(_ request: OSSystemExtensionRequest,
                 actionForReplacingExtension existing: OSSystemExtensionProperties,
                 withExtension ext: OSSystemExtensionProperties) -> OSSystemExtensionRequest.ReplacementAction {
        Swift.print("Request needs to replace the extension")
        
        // Provide options for how to handle the extension replacement
        return .replace
    }
    
    // Handle catastrophic errors (e.g., system extension termination)
    func handleError(_ error: Error) {
        // Log the error for debugging
        Swift.print("Catastrophic error occurred: \(error.localizedDescription)")

        // Optionally show an alert to the user
        let alert = NSAlert()
        alert.messageText = "A critical error occurred"
        alert.informativeText = "The system extension failed to activate. Please try again later."
        alert.alertStyle = .critical
        alert.addButton(withTitle: "OK")
        alert.runModal()

        // Attempt to retry or reconnect, if applicable
        // activateDriver()  // Uncomment to attempt reactivation
    }
}
