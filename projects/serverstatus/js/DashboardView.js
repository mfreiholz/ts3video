(function () {
  'use strict';

  var _updateTimerId = -1;

  brite.registerView('DashboardView', {}, {

    create: function (data, config) {
      var view = this;
      return jQuery('#tmpl-DashboardView').render();
    },

    postDisplay: function (data, config) {
      if (_updateTimerId === -1) {
        _updateTimerId = setInterval(this.requestUpdate, app.logic.updateInterval);
      }
      app.logic.requestUpdate('appinfo');
      this.requestUpdate();
    },

    destroy: function () {
      if (_updateTimerId !== -1) {
        clearInterval(_updateTimerId);
        _updateTimerId = -1;
      }
    },

    daoEvents: {
      'dataChange; AppInfo; update': function (e) {
        this.renderAppInfo(e.daoEvent.result);
        //console.log('AppInfo updated:' + JSON.stringify(e.daoEvent.result));
      },
      'dataChange; MemoryUsage; update': function (e) {
        this.renderMemoryInfo(e.daoEvent.result);
        //console.log('MemoryUsage updated:' + JSON.stringify(e.daoEvent.result));
      },
      'dataChange; BandwidthUsage; update': function (e) {
        this.renderBandwidthInfo(e.daoEvent.result);
        //console.log('BandwidthUsage updated:' + JSON.stringify(e.daoEvent.result));
      }
    },

    ///////////////////////////////////////////////////////////////////
    // Private methods
    ///////////////////////////////////////////////////////////////////

    requestUpdate: function () {
      app.logic.requestUpdate('memoryusageinfo');
      app.logic.requestUpdate('bandwidthusageinfo');
    },

    renderAppInfo: function (data) {
      var items = [
          { key: app.tr('Name'), value: data.appname },
          { key: app.tr('Version'), value: data.appversion },
          { key: app.tr('Directory'), value: data.appdirectory },
          { key: app.tr('Binary'), value: data.appfilepath }
        ];
      var html = jQuery('#tmpl-hl-info').render({ items: items });
      jQuery('.appinfo').html(html);
    },

    renderMemoryInfo: function (data) {
      var items = [
          { key: app.tr('CPU usage'), value: 'n/A' },
          { key: app.tr('Memory usage'), value: app.logic.bytesAsReadableSize(data.privateusage) }
        ];
      var html = jQuery('#tmpl-hl-info').render({ items: items });
      jQuery('.procinfo').html(html);
    },

    renderBandwidthInfo: function (data) {
      var items = [
          { key: app.tr('Uploaded bytes'), value: app.logic.bytesAsReadableSize(data.byteswritten) },
          { key: app.tr('Upload rate'), value: app.logic.bytesAsReadableSize(data.bandwidthwrite) + '/s' },
          { key: app.tr('Downloaded bytes'), value: app.logic.bytesAsReadableSize(data.bytesread) },
          { key: app.tr('Download rate'), value: app.logic.bytesAsReadableSize(data.bandwidthread) + '/s' }
        ];
      var html = jQuery('#tmpl-hl-info').render({ items: items });
      jQuery('.bandwidthinfo').html(html);
    }

  });
}());
