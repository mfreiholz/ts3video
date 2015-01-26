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
  // Note: This lambda slot is not thread-safe. If MediaSocketHandler should run in a separate thread, we need to reimplement this function.
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

    // Notify client about the successful media authentication.
    auto conn = _connections.value(clientId);
    if (conn) {
      conn->sendMediaAuthSuccessNotify();
    }
    this->updateMediaRecipients();
  });
}

TS3VideoServer::~TS3VideoServer()
{
  delete _mediaSocketHandler;
}

void TS3VideoServer::updateMediaRecipients()
{
  // TODO Currently everyone sends to everyone, thats not the end scenario!
  MediaRecipients recips;
  auto clients = _clients.values();
  foreach(auto client, clients) {
    if (!client || client->mediaAddress.isEmpty() || client->mediaPort <= 0) {
      continue;
    }
    MediaSenderEntity sender;
    sender.clientId = client->id;
    sender.address = QHostAddress(client->mediaAddress);
    sender.port = client->mediaPort;
    sender.id = MediaSenderEntity::createID(sender.address, sender.port);
    foreach(auto client2, clients) {
      if (!client2 || client2 == client || client2->mediaAddress.isEmpty() || client2->mediaPort <= 0) {
        continue;
      }
      MediaReceiverEntity receiver;
      receiver.clientId = client2->id;
      receiver.address = QHostAddress(client2->mediaAddress);
      receiver.port = client2->mediaPort;
      sender.receivers.append(receiver);
    }
    recips.id2sender.insert(sender.id, sender);
  }
  _mediaSocketHandler->setRecipients(recips);
}