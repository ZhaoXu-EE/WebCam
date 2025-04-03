document.addEventListener('DOMContentLoaded', () => {
    const streamButton = document.getElementById('streamButton');
    const timestampDiv = document.getElementById('timestamp');
    const webcamImage = document.getElementById('webcamImage');
    const msgDiv = document.getElementById('msg');
    
    let websocket = null;
    let localhost = "";
    let buttonClicked = false;
  
    function init() {
        if(window.location.hostname !== "") {
            localhost = window.location.hostname;
        }
    }
  
    function doConnect() {
        if (streamButton.textContent === "Start Webcam") {
            writeToScreen("Connecting to ws://" + localhost + ":81/...");
            // Removed button disabling here
            // streamButton.disabled = true;
            websocket = new WebSocket("ws://" + localhost + ":81/");
            
            websocket.onopen = function(evt) {
                // No need to re-enable since it's always enabled
                streamButton.textContent = "Stop Webcam";
                streamButton.title = "Click to stop webcam";
                writeToScreen("Connected");
                buttonClicked = false;
            };
  
            websocket.onclose = function(evt) {
                streamButton.textContent = "Start Webcam";
                streamButton.title = "Click to start webcam";
                if (buttonClicked) {
                    writeToScreen("Webcam stopped.");
                } else {
                    writeToScreen("Disconnected unexpectedly. Error: " + evt);
                    doConnect(); // Optionally auto-reconnect if not user-initiated.
                }
                buttonClicked = false;
            };
  
            websocket.onmessage = function(evt) {
                const now = new Date();
                timestampDiv.textContent = "Last Update: " + now.toLocaleTimeString();
                
                const reader = new FileReader();
                reader.onload = function(e) {
                    webcamImage.src = e.target.result;
                };
                reader.readAsDataURL(evt.data);
            };
  
            websocket.onerror = function(evt) {
                writeToScreen("WebSocket error");
                websocket.close();
                streamButton.textContent = "Start Webcam";
                streamButton.title = "Click to start webcam";
                // Removed button disabling here as well
                // streamButton.disabled = false;
                buttonClicked = false;
            };
        } else {
            writeToScreen("Disconnecting...");
            websocket.close();
        }
    }
  
    streamButton.addEventListener('click', () => {
        // Removed button disabling on click
        // streamButton.disabled = true;
        buttonClicked = true;
        if (websocket && websocket.readyState === WebSocket.OPEN) {
            websocket.close();
        } else {
            doConnect();
        }
    });
  
    function writeToScreen(message) {
        msgDiv.innerHTML += `<div>${new Date().toLocaleTimeString()}: ${message}</div>`;
        msgDiv.scrollTop = msgDiv.scrollHeight;
    }
  
    // Initialize on load
    init();
});
