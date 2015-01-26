#include "ts3videoserver.h"

#include "qcorconnection.h"

#include "cliententity.h"
#include "channelentity.h"

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
  _participants(),
  _mediaSocketHandler(nullptr),
  _tokens()
{
  // Init QCorServer listening for new client connections.
  _corServer.listen(QHostAddress::Any, 6000);

  // Accepting new connections.
  connect(&_corServer, &QCorServer::newConnection, [this] (QCorConnection *connection) {
    new ClientConnectionHandler(this, connection, this);
  });

  // Init media socket.
  _mediaSocketHandler = new MediaSocketHandler(this);

  // Handle media authentications.
  connect(_mediaSocketHandler, &MediaSocketHandler::tokenAuthentication, [this] (const QString &token, const QHostAddress &address, quint16 port) {
    if (!_tokens.contains(token)) {
      qDebug() << QString("Received invalid media auth token (token=%1; address=%2; port=%3)").arg(token).arg(address.toString()).arg(port);
      return;
    }
    // Update client-info with address and port.
    auto clientId = _tokens.take(token);
    auto clientEntity = _clients.value(clientId);
    if (!clientEntity) {
      qDebug() << QString("No matching ClientEntity for auth token (token=%1; client-id=%2)").arg(token).arg(clientId);
      return;
    }
    clientEntity->mediaAddress = address.toString();
    clientEntity->mediaPort = port;

    // TODO Notify client about the successful media authentication.

    // Recreate recipients list.
    MediaRecipients recips;
    _mediaSocketHandler->setRecipients(recips);
  });
}

TS3VideoServer::~TS3VideoServer()
{
  delete _mediaSocketHandler;
}