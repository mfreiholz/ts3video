Requires Apache2.4 WebSocket proxy forward

```
ProxyPass /ts3video-websocket ws://127.0.0.1:6002
ProxyPassReverse /ts3video-websocket ws://127.0.0.1:6002
```
