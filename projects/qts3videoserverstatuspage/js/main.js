(function () {
  "use strict";
  
  brite.viewDefaultConfig.loadTmpl = true;
  brite.viewDefaultConfig.loadCss = false;
  
  var ws = new WebSocket("ws://127.0.0.1:6002");
  
  ws.onopen = function (ev) {
    ws.send("/status");
  };
  
  ws.onclose = function (ev) {
  };
  
  ws.onerror = function (ev) {
  };
  
  ws.onmessage = function (ev) {
    var data = JSON.parse(ev.data);
    brite.display("MainView", "#AppMain", data, { emptyParent: true })
      .done(function () {
        setTimeout(function () { ws.send("/status"); }, 1500);
      })
      .fail(function () {
      });
  };
  
}());