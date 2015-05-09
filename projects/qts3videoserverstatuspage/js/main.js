var app = app || {};
window.app = app;

///////////////////////////////////////////////////////////////////////
// ServerStatusDaoHandler
///////////////////////////////////////////////////////////////////////

(function () {
  'use strict';

  function SingleObjectDaoHandler(entityType) {
    this._entityType = entityType;
    this._obj = {};
  }

  SingleObjectDaoHandler.prototype.entityType = function () {
    return this._entityType;
  };

  SingleObjectDaoHandler.prototype.update = function (entity) {
    this._obj = entity;
    //return jQuery.extend({}, this._obj); // Destroys array!!!!
    return entity;
  };

  SingleObjectDaoHandler.prototype.get = function (id) {
    return this._obj;
  };

  app.SingleObjectDaoHandler = SingleObjectDaoHandler;
}());

(function () {
  'use strict';

  /////////////////////////////////////////////////////////////////////
  // AppMain
  /////////////////////////////////////////////////////////////////////

  function AppMain(address, port) {
    this.websocket = null;
    this.address = address;
    this.port = port;
    this.updateInterval = 5000;
  }

  AppMain.prototype.bootstrap = function () {
    // Register DAOs for server status information.
    brite.registerDao(new app.SingleObjectDaoHandler('ServerStatus'));
    brite.registerDao(new app.SingleObjectDaoHandler('AppInfo'));
    brite.registerDao(new app.SingleObjectDaoHandler('MemoryUsage'));
    brite.registerDao(new app.SingleObjectDaoHandler('BandwidthUsage'));
    brite.registerDao(new app.SingleObjectDaoHandler('Clients'));
    brite.registerDao(new app.SingleObjectDaoHandler('WSClients'));

    // Route to view based on "view" parameter.
    this.routeByUrl(window.location.href);
  };

  AppMain.prototype.routeByUrl = function (url) {
    var pos = -1;
    var viewName = 'DashboardView';
    if ((pos = url.indexOf('view=')) !== -1) {
      viewName = url.substr(pos + 5);
    }
    brite.display(viewName, '#AppMain', {}, { emptyParent: true })
      .done(function () {})
      .fail(function () {});
  };

  AppMain.prototype.connect = function () {
    var that = this;
    var def = new jQuery.Deferred();

    if (this.websocket !== null) {
      def.reject();
      return def.promise();
    }

    this.websocket = new WebSocket("ws://" + this.address + ":" + this.port + "/ts3video-websocket");

    this.websocket.onopen = function (ev) {
      def.resolve();
    };

    this.websocket.onclose = function (ev) {
      that.websocket = null;
    };

    this.websocket.onerror = function (ev) {
      that.websocket = null;
      def.reject();
      //jQuery("#AppMain").html("Server connection lost... Reconnecting...");
      //setTimeout(function () { that.connect(); }, 3000);
    };


    this.websocket.onmessage = function (ev) {
      var data = JSON.parse(ev.data);
      if (data.action === 'appinfo') {
        brite.dao('AppInfo').update(data.data);
      } else if (data.action === 'memoryusageinfo') {
        brite.dao('MemoryUsage').update(data.data);
      } else if (data.action === 'bandwidthusageinfo') {
        brite.dao('BandwidthUsage').update(data.data);
      } else if (data.action === 'clients') {
        brite.dao('Clients').update(data.data);
      } else if (data.action === 'wsclients') {
        brite.dao('WSClients').update(data.data);
      } else {
        brite.dao('ServerStatus').update(data.data);
      }
    };

    return def.promise();
  };

  AppMain.prototype.requestUpdate = function (action, data) {
    if (this.websocket === null) {
      return false;
    }
    this.websocket.send(JSON.stringify({ action: action, data: data }));
    return true;
  };

  AppMain.prototype.tr = function (str, args) {
    return str;
  };

  AppMain.prototype.round = function (f) {
    return parseFloat(Math.round(f * 100) / 100).toFixed(2);
  };

  AppMain.prototype.bytesAsReadableSize = function (b) {
    if (b > 1073741824) {
      return this.round(b / 1073741824) + " GB";
    } else if (b > 1048576) {
      return this.round(b / 1048576) + " MB";
    } else if (b > 1024) {
      return this.round(b / 1024) + " KB";
    }
    return (b) + " Bytes";
  };

  /////////////////////////////////////////////////////////////////////

  $.views.helpers({
    bytesAsReadableSize: function (b) {
      return app.logic.bytesAsReadableSize(b);
    },
    tr: function (str, args) {
      return app.logic.tr(str, args);
    }
  });

  /////////////////////////////////////////////////////////////////////

  brite.viewDefaultConfig.loadTmpl = true;
  brite.viewDefaultConfig.loadCss = false;

  app.logic = new AppMain(window.location.host, 13375);
  app.tr = app.logic.tr;

  // First we connect and than we show the content.
  app.logic.connect()
    .done(function () {
      app.logic.bootstrap();
    });

}());

jQuery(document).ready(function () {
  'use strict';

  // DEV (BEGIN)
  // Handle top navigation (Move this handling into MainView).
  jQuery('a').click(function (e) {
    var link = jQuery(e.currentTarget).attr('href');
    if (link.indexOf('view=') !== -1) {
      e.preventDefault();
      app.logic.routeByUrl(link);
      history.pushState({}, "", link);
    }
  });
  // DEV (END)

});
