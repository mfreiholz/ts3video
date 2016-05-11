(function () {
  'use strict';

  var _updateTimerId = -1;

  brite.registerView('ConnectionsView', {}, {

	create: function (data, config) {
	  var view = this;
	  return jQuery('#tmpl-ConnectionsView').render();
	},

	postDisplay: function (data, config) {
	  if (_updateTimerId === -1) {
		_updateTimerId = setInterval(this.requestUpdate, app.logic.updateInterval);
	  }
	  this.requestUpdate();
	},

	destroy: function () {
	  if (_updateTimerId !== -1) {
		clearInterval(_updateTimerId);
		_updateTimerId = -1;
	  }
	},

	daoEvents: {
	  'dataChange; Clients; update': function (e) {
		this.renderClientList(e.daoEvent.result);
		//console.log('Clients updated:' + JSON.stringify(e.daoEvent.result));
	  },
	  'dataChange; WSClients; update': function (e) {
		this.renderWSClientList(e.daoEvent.result);
		//console.log('WSClients updated:' + JSON.stringify(e.daoEvent.result));
	  },
	  'dataChange; Channels; update': function (e) {
		this.renderChannelList(e.daoEvent.result);
		//console.log('Channels updated:' + JSON.stringify(e.daoEvent.result));
	  }
	},

	///////////////////////////////////////////////////////////////////
	// Private methods
	///////////////////////////////////////////////////////////////////

	requestUpdate: function () {
	  app.logic.requestUpdate('clients');
	  app.logic.requestUpdate('wsclients');
	  app.logic.requestUpdate('channels');
	},

	renderClientList: function (data) {
		data.sort(function (a, b) {
			if (a.id < b.id)
				return -1;
			else if (a.id > b.id)
				return 1;
			return 0;
		});
	  var html = jQuery('#tmpl-ConnectionsView-ClientList').render({ clients: data });
	  jQuery('.clientcount').html(data.length);
	  jQuery('.clients').html(html);
	},

	renderWSClientList: function (data) {
	  var html = jQuery('#tmpl-ConnectionsView-WSClientList').render({ clients: data });
	  jQuery('.wsclientcount').html(data.length);
	  jQuery('.wsclients').html(html);
	},

	renderChannelList: function (data) {
	  var html = jQuery('#tmpl-ConnectionsView-ChannelList').render({ channels: data });
	  jQuery('.channelcount').html(data.length);
	  jQuery('.channels').html(html);
	}

  });
}());
