#include "mediastreamingserver_p.h"
#include "humblelogging/api.h"
#include "QHostAddress"

HUMBLE_LOGGER(HL, "server.streaming");

///////////////////////////////////////////////////////////////////////////////
// MediaStreamingServer
///////////////////////////////////////////////////////////////////////////////

MediaStreamingServer::MediaStreamingServer(const Options &opts, QObject *parent) :
  QObject(parent),
  d(new Private(this))
{
  d->_opts = opts;
}

MediaStreamingServer::~MediaStreamingServer()
{
  delete d;
}

bool MediaStreamingServer::startup()
{
  // Listen for media data.
  if (!d->_handler.listen(QHostAddress::Any, d->_opts.streamingPort)) {
    return false;
  }

  // Connect to master server.
  if (!d->_opts.masterServerAddress.isNull() && d->_opts.masterServerPort > 0) {
    MediaStreamingMasterConnectionHandler::StartOptions opts;
    opts.address = d->_opts.masterServerAddress;
    opts.port = d->_opts.masterServerPort;
    opts.externalStreamingAddress = d->_opts.externalStreamingAddress;
    opts.externalStreamingPort = d->_opts.externalStreamingPort;
    d->_masterConnection.startup(opts);
  }

  return true;
}

bool MediaStreamingServer::shutdown()
{
  d->_handler.close();
  d->_masterConnection.shutdown();
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Private Impl
///////////////////////////////////////////////////////////////////////////////

MediaStreamingServer::Private::Private(MediaStreamingServer *owner) :
  QObject(0),
  _owner(owner),
  _handler(0),
  _masterConnection(owner, 0)
{
}

MediaStreamingServer::Private::~Private()
{
}