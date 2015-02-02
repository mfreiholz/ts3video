#include "ts3videoserver.h"

#include "humblelogging/api.h"

#include "qcorconnection.h"

#include "cliententity.h"
#include "channelentity.h"

#include "clientconnectionhandler.h"

HUMBLE_LOGGER(HL, "server");

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

}

TS3VideoServer::~TS3VideoServer()
{
  delete _mediaSocketHandler;
}

bool TS3VideoServer::init()
{
  // Init QCorServer listening for new client connections.
  const quint16 port = TS3VIDEOSERVER_PORT;
  if (!_corServer.listen(QHostAddress::Any, port)) {
    HL_ERROR(HL, QString("Can not bind to TCP port (port=%1)").arg(port).toStdString());
    return false;
  }
  HL_INFO(HL, QString("Listening for new client connections (protocol=TCP; port=%1)").arg(port).toStdString());
  // Accepting new connections.
  connect(&_corServer, &QCorServer::newConnection, [this](QCorConnection *connection) {
    new ClientConnectionHandler(this, connection, this);
  });

  // Init media socket.
  _mediaSocketHandler = new MediaSocketHandler(port, this);
  if (!_mediaSocketHandler->init()) {
    return false;
  }
  HL_INFO(HL, QString("Listening for media data (protocol=UDP; port=%1)").arg(port).toStdString());
  // Handle media authentications.
  // Note: This lambda slot is not thread-safe. If MediaSocketHandler should run in a separate thread, we need to reimplement this function.
  connect(_mediaSocketHandler, &MediaSocketHandler::tokenAuthentication, [this](const QString &token, const QHostAddress &address, quint16 port) {
    if (!_tokens.contains(token)) {
      HL_WARN(HL, QString("Received invalid media auth token (token=%1; address=%2; port=%3)").arg(token).arg(address.toString()).arg(port).toStdString());
      return;
    }
    // Update client-info with address and port.
    auto clientId = _tokens.take(token);
    auto clientEntity = _clients.value(clientId);
    if (!clientEntity) {
      HL_WARN(HL, QString("No matching ClientEntity for auth token (token=%1; client-id=%2)").arg(token).arg(clientId).toStdString());
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
  return true;
}

void TS3VideoServer::updateMediaRecipients()
{
  // TODO Currently everyone sends to everyone, thats not the end scenario!
  bool sendBackOwnVideo = true;
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
      if (!client2 || (!sendBackOwnVideo && client2 == client) || client2->mediaAddress.isEmpty() || client2->mediaPort <= 0) {
        continue;
      }
      MediaReceiverEntity receiver;
      receiver.clientId = client2->id;
      receiver.address = QHostAddress(client2->mediaAddress);
      receiver.port = client2->mediaPort;
      sender.receivers.append(receiver);

      if (!recips.clientid2receiver.contains(receiver.clientId)) {
        recips.clientid2receiver.insert(receiver.clientId, receiver);
      }
    }
    recips.id2sender.insert(sender.id, sender);
  }
  _mediaSocketHandler->setRecipients(recips);
}