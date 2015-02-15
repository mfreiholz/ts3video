(function () {
  "use strict";
  
  // Brite init.
  brite.viewDefaultConfig.loadTmpl = true;
  brite.viewDefaultConfig.loadCss = false;
  
  /////////////////////////////////////////////////////////////////////
  // AppMain
  /////////////////////////////////////////////////////////////////////

  function AppMain(address, port) {
    this.websocket = null;
    this.address = address;
    this.port = port;
  }
  
  AppMain.prototype.connect = function () {
    var that = this;
    if (this.websocket !== null) {
      return;
    }
    
    this.websocket = new WebSocket("ws://" + this.address + ":" + this.port);
    
    this.websocket.onopen = function (ev) {
      that.websocket.send("/status");
    };
    
    this.websocket.onclose = function (ev) {
      console.log("WS CLOSE");
    };
    
    this.websocket.onerror = function (ev) {
      console.log("WS ERROR");
      that.websocket = null;
      jQuery("#AppMain").html("Server connection lost... Reconnecting...");
      setTimeout(function () { that.connect(); }, 3000);
    };
    
    this.websocket.onmessage = function (ev) {
      var data = JSON.parse(ev.data);
      brite.display("MainView", "#AppMain", data, { emptyParent: true })
        .done(function () {
          setTimeout(function () {
            if (that.websocket !== null) {
              that.websocket.send("/status");
            }
          }, 1500); // TODO (The server should send update instead of requesting it!)
        })
        .fail(function () {
        });
    };
  };
  

  /*var ws = new WebSocket("ws://" + window.location.host + ":6002");
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
  };*/

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
  
  /////////////////////////////////////////////////////////////////////
  
  var app = new AppMain(window.location.host, 6002);
  app.connect();

}());