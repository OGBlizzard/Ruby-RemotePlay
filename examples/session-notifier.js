// Import this file from Settings -> Config -> Custom Scripts.
function onSessionStarted(api, event) {
    if (event.connected)
        api.notify("Ruby RemotePlay", "Connected to " + event.nickname);
}

function onBeforeDisconnect(api, event) {
    api.log(event.sleep ? "Console will sleep" : "Session stopping");
}
