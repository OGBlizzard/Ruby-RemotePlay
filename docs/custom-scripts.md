# Custom scripts

Ruby RemotePlay supports local JavaScript automation. Open **Settings → Config → Custom Scripts** to create or import a `.js` file, enable it, and run it manually.

Scripts are stored in the application data directory and run in a fresh JavaScript engine. The default API is intentionally small:

```javascript
api.log("message");
api.warn("warning");
api.error("error");
api.notify("Title", "Message");
api.appVersion();
api.scriptsDirectory();
```

A script can implement any of these hooks:

```javascript
function onAppStart(api, event) {}
function onBeforeConnect(api, event) {}
function onSessionStarted(api, event) {}
function onBeforeDisconnect(api, event) {}
```

The event object is a plain JavaScript object. `onBeforeConnect` receives `hostIndex` and `nickname`. `onSessionStarted` receives `nickname` and `connected`. `onBeforeDisconnect` receives `sleep`.

Scripts do not receive shell execution, filesystem APIs, network APIs, controller injection, PSN tokens, or console credentials. Do not paste secrets into scripts. A script error is shown in the script manager and does not stop Remote Play.

## Example: session notifier

```javascript
function onSessionStarted(api, event) {
    if (event.connected) {
        api.notify("Ruby RemotePlay", "Connected to " + event.nickname);
    }
}

function onBeforeDisconnect(api, event) {
    api.log(event.sleep ? "Console will sleep" : "Session stopping");
}
```
