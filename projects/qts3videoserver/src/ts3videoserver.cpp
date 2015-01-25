#include "ts3videoserver.h"

#include "qcorconnection.h"

#include "clientconnectionhandler.h"

///////////////////////////////////////////////////////////////////////

TS3VideoServer::TS3VideoServer(QObject *parent) :
  QObject(parent),
  _corServer(this),
  _connections(),
  _nextClientId(0),
  _clients(),
  _nextChannelId(0),
  _channels(),
  _participants()
{
  // Init QCorServer listening for new client connections.
  _corServer.listen(QHostAddress::Any, 6000);

  // Accepting new connections.
  connect(&_corServer, &QCorServer::newConnection, [this] (QCorConnection *connection) {
    new ClientConnectionHandler(this, connection, this);
  });
}

TS3VideoServer::~TS3VideoServer()
{
}