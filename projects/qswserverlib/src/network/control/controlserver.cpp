#include "QHostAddress"
#include "humblelogging/api.h"
#include "controlserver_p.h"
#include "controlconnectionhandler.h"
#include "module/controlmodule.h"

HUMBLE_LOGGER(HL, "server.control");

///////////////////////////////////////////////////////////////////////////////
// ControlServer
///////////////////////////////////////////////////////////////////////////////

ControlServer::ControlServer(QObject *parent) :
  QObject(parent),
  d(new Private(this))
{
}

ControlServer::~ControlServer()
{
  delete d;
}

bool ControlServer::startup(const Options &opts)
{
  d->_opts = opts;

  // Register modules.
  d->_modules.append(new DemoControlModule());
  foreach (ControlModule *m, d->_modules) {
    m->initialize();
    d->_id2module.insert(m->getName(), m);
  }

  // Listen for client connections.
  if (!d->_clientListener.listen(QHostAddress::Any, d->_opts.clientPort)) {
    HL_ERROR(HL, QString("Can not listen for client connections (port=%1)").arg(d->_opts.clientPort).toStdString());
    return false;
  }
  HL_INFO(HL, QString("Listen for client connections (port=%1)").arg(d->_opts.clientPort).toStdString());

  // Connect to master server.
  if (!opts.masterServerAddress.isEmpty() && opts.masterServerPort > 0) {
    ControlToMasterConnectionHandler::StartupOptions sopts;
    sopts.address = opts.masterServerAddress;
    sopts.port = opts.masterServerPort;
    d->_masterConnection.startup(sopts);
  }

  return true;
}

bool ControlServer::shutdown()
{
  d->_clientListener.close();
  d->_masterConnection.shutdown();
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Private Impl
///////////////////////////////////////////////////////////////////////////////

ControlServer::Private::Private(ControlServer *owner) :
  QObject(owner),
  _owner(owner),
  _masterConnection(owner)
{
  connect(&_clientListener, SIGNAL(newConnection()), SLOT(onNewClientConnection()));
}

ControlServer::Private::~Private()
{
  while (!_clientConnections.isEmpty()) {
    delete _clientConnections.takeFirst();
  }

  _id2module.clear();
  while (!_modules.isEmpty()) {
    delete _modules.takeFirst();
  }
}

void ControlServer::Private::onNewClientConnection()
{
  while (_clientListener.hasPendingConnections()) {
    QTcpSocket *socket = _clientListener.nextPendingConnection();
    ControlConnectionHandler *handler = new ControlConnectionHandler(_owner, socket, this);
    connect(handler, SIGNAL(disconnected()), SLOT(onClientDisconnected()));
    _clientConnections.append(handler);
  }
}

void ControlServer::Private::onClientDisconnected()
{
  ControlConnectionHandler *handler = qobject_cast<ControlConnectionHandler*>(sender());
  _clientConnections.removeAll(handler);
  handler->deleteLater();
}