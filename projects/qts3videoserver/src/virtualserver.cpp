#include "virtualserver_p.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

#include "humblelogging/api.h"

#include "qcorconnection.h"

#include "servercliententity.h"
#include "serverchannelentity.h"

#include "clientconnectionhandler.h"

HUMBLE_LOGGER(HL, "server");

///////////////////////////////////////////////////////////////////////

VirtualServer::VirtualServer(const VirtualServerOptions &opts, QObject *parent) :
  QObject(parent),
  d(new VirtualServerPrivate(this)),
  _opts(opts),
  _corServer(this),
  _connections(),
  _nextClientId(0),
  _clients(),
  _nextChannelId(0),
  _channels(),
  _participants(),
  _mediaSocketHandler(nullptr),
  _tokens(),
  _wsStatusServer(nullptr)
{
  d->registerAction(ActionPtr(new AuthenticationAction()));
  d->registerAction(ActionPtr(new GoodbyeAction()));
  d->registerAction(ActionPtr(new HeartbeatAction()));
  d->registerAction(ActionPtr(new JoinChannelAction()));
  d->registerAction(ActionPtr(new JoinChannel2Action()));
  d->registerAction(ActionPtr(new EnableVideoAction()));
  d->registerAction(ActionPtr(new DisableVideoAction()));
}

VirtualServer::~VirtualServer()
{
  delete _mediaSocketHandler;
  delete _wsStatusServer;
}

bool VirtualServer::init()
{
  // Init QCorServer listening for new client connections.
  const quint16 port = _opts.port;
  if (!_corServer.listen(_opts.address, port)) {
    HL_ERROR(HL, QString("Can not bind to TCP port (port=%1)").arg(port).toStdString());
    return false;
  }
  HL_INFO(HL, QString("Listening for client connections (protocol=TCP; address=%1; port=%2)").arg(_opts.address.toString()).arg(port).toStdString());
  // Accepting new connections.
  connect(&_corServer, &QCorServer::newConnection, [this](QCorConnection *connection) {
    auto conn = new ClientConnectionHandler(this, connection, this);
  });

  // Init media socket.
  _mediaSocketHandler = new MediaSocketHandler(port, this);
  if (!_mediaSocketHandler->init()) {
    return false;
  }
  HL_INFO(HL, QString("Listening for media data (protocol=UDP; address=%1; port=%2)").arg(_opts.address.toString()).arg(port).toStdString());
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
      HL_WARN(HL, QString("No matching client-entity for auth token (token=%1; client-id=%2)").arg(token).arg(clientId).toStdString());
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
  QObject::connect(_mediaSocketHandler, &MediaSocketHandler::networkUsageUpdated, [this](const NetworkUsageEntity &networkUsage) {
    _networkUsageMediaSocket = networkUsage;
  });

  // Init status web-socket.
  WebSocketStatusServer::Options wsopts;
  wsopts.address = _opts.wsStatusAddress;
  wsopts.port = _opts.wsStatusPort;
  _wsStatusServer = new WebSocketStatusServer(wsopts, this);
  if (!_wsStatusServer->init()) {
    HL_ERROR(HL, QString("Can not bind to TCP port (port=%1)").arg(wsopts.port).toStdString());
    return false;
  }
  HL_INFO(HL, QString("Listening for web-socket status connections (protocol=TCP; address=%1; port=%2)").arg(wsopts.address.toString()).arg(wsopts.port).toStdString());

  return true;
}

const VirtualServerOptions& VirtualServer::options() const
{
  return _opts;
}

void VirtualServer::updateMediaRecipients()
{
  bool sendBackOwnVideo = false;
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

    auto siblingClientIds = getSiblingClientIds(client->id);
    foreach(auto siblingClientId, siblingClientIds) {
      auto client2 = _clients.value(siblingClientId);
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

ServerChannelEntity* VirtualServer::addClientToChannel(int clientId, int channelId)
{
  Q_ASSERT(_channels.value(channelId) != 0);
  auto channelEntity = _channels.value(channelId);
  _participants[channelEntity->id].insert(clientId);
  _client2channels[clientId].insert(channelEntity->id);
  return channelEntity;
}

void VirtualServer::removeClientFromChannel(int clientId, int channelId)
{
  // Remove from channel.
  _participants[channelId].remove(clientId);
  _client2channels[clientId].remove(channelId);
  // Delete channel and free some resources, if there are no more participants.
  if (_participants[channelId].isEmpty()) {
    _participants.remove(channelId);
    delete _channels.take(channelId);
  }
  if (_client2channels[clientId].isEmpty()) {
    _client2channels.remove(clientId);
  }
}

void VirtualServer::removeClientFromChannels(int clientId)
{
  // Find all channels of the client.
  QList<int> channelIds;
  if (_client2channels.contains(clientId)) {
    channelIds = _client2channels[clientId].toList();
  }
  // Remove from all channels.
  foreach (auto channelId, channelIds) {
    removeClientFromChannel(clientId, channelId);
  }
}

QList<int> VirtualServer::getSiblingClientIds(int clientId) const
{
  QSet<int> clientIds;
  // From channels (participants).
  foreach (auto channelId, _client2channels.value(clientId).toList()) {
    foreach (auto participantId, _participants.value(channelId)) {
      clientIds.insert(participantId);
    }
  }
  return clientIds.toList();
}

void VirtualServer::bann(const QHostAddress &address)
{
  QDir dir(QCoreApplication::applicationDirPath());
  const auto filePath = dir.filePath("banns.ini");
  QSettings ini(filePath, QSettings::IniFormat);

  ini.beginGroup("ips");
  ini.setValue(address.toString(), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
  ini.endGroup();
}

void VirtualServer::unbann(const QHostAddress &address)
{
  QDir dir(QCoreApplication::applicationDirPath());
  const auto filePath = dir.filePath("banns.ini");
  QSettings ini(filePath, QSettings::IniFormat);

  ini.beginGroup("ips");
  ini.remove(address.toString());
  ini.endGroup();
}

bool VirtualServer::isBanned(const QHostAddress &address)
{
  QDir dir(QCoreApplication::applicationDirPath());
  const auto filePath = dir.filePath("banns.ini");
  QSettings ini(filePath, QSettings::IniFormat);

  return ini.contains("ips/" + address.toString());
}