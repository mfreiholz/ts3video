(function () {
  "use strict";

  brite.viewDefaultConfig.loadTmpl = true;
  brite.viewDefaultConfig.loadCss = false;

  var ws = new WebSocket("ws://" + window.location.host + ":6002");

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

  function round(f) {
    return parseFloat(Math.round(f * 100) / 100).toFixed(2);
  }

  $.views.helpers({
    bytesAsReadableSize: function (b) {
      if (b > 1073741824) {
        return round(b / 1073741824) + " GB";
      } else if (b > 1048576) {
        return round(b / 1048576) + " MB";
      } else if (b > 1024) {
        return round(b / 1024) + " KB";
      }
      return (b) + " Bytes";
    }
  });

}());