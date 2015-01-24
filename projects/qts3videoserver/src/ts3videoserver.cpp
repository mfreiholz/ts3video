#include "ts3videoserver.h"

#include <QDebug>
#include <QTcpSocket>

#include "qcorconnection.h"

#include "clientconnectionhandler.h"

///////////////////////////////////////////////////////////////////////

TS3VideoServer::TS3VideoServer(QObject *parent) :
  QObject(parent),
  _corServer(this)
{
  // Init QCorServer listening for new client connections.
  _corServer.listen(QHostAddress::Any, 6000);

  // Accepting new connections.
  connect(&_corServer, &QCorServer::newConnection, [this] (QCorConnection *connection) {
    qDebug() << QString("New client connection (addr=%1; port=%2)").arg(connection->socket()->peerAddress().toString()).arg(connection->socket()->peerPort());
    auto clientConn = new ClientConnectionHandler(this, connection, this);
    _connections.append(clientConn);
  });
}

TS3VideoServer::~TS3VideoServer()
{

}