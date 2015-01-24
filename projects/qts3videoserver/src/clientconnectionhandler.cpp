#include "clientconnectionhandler.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "qcorconnection.h"

#include "ts3videoserver.h"

///////////////////////////////////////////////////////////////////////

QByteArray createJsonResponseError(int status, const QString &errorMessage)
{
  QJsonObject root;
  root["status"] = status;
  root["error"] = errorMessage;
  return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

QByteArray createJsonResponse(const  QJsonObject &data)
{
  QJsonObject root;
  root["status"] = 0;
  root["data"] = data;
  return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

///////////////////////////////////////////////////////////////////////

ClientConnectionHandler::ClientConnectionHandler(TS3VideoServer *server, QCorConnection *connection, QObject *parent) :
  QObject(parent),
  _server(server),
  _connection(connection),
  _authenticated(false)
{
  connect(_connection, &QCorConnection::newIncomingRequest, this, &ClientConnectionHandler::onNewIncomingRequest);
}

ClientConnectionHandler::~ClientConnectionHandler()
{
  delete _connection;
}

void ClientConnectionHandler::onStateChanged(QAbstractSocket::SocketState state)
{
  switch (state) {
    case QAbstractSocket::UnconnectedState: {
      _server->_connections.removeAll(this);
      // TODO: Notify sibling clients about the disconnect.
      // Delete itself.
      deleteLater();
      break;
    }
  }
}

void ClientConnectionHandler::onNewIncomingRequest(QCorFrameRefPtr frame)
{
  qDebug() << QString("New incoming frame (size=%1; content=%2)").arg(frame->data().size()).arg(QString(frame->data()));

  QJsonParseError err;
  auto doc = QJsonDocument::fromJson(frame->data(), &err);
  if (err.error != QJsonParseError::NoError) {
    QCorFrame res;
    res.initResponse(*frame.data());
    res.setData(createJsonResponseError(1, QString("JSON Parse Error: %1").arg(err.errorString())));
    _connection->sendResponse(res);
    return;
  }
  else if (doc.isEmpty() || !doc.isObject()) {
    QCorFrame res;
    res.initResponse(*frame.data());
    res.setData(createJsonResponseError(2, QString("Empty request not supported: %1")));
    _connection->sendResponse(res);
    return;
  }

  auto root = doc.object();
  auto action = root["action"].toString();
  auto params = root["parameters"].toObject();

  if (action == "auth") {
    auto version = params["version"].toInt();
    auto username = params["username"].toString();
    // Compare client version against server version compatibility.
    if (version != TS3VIDEOSERVER_VERSION) {
      QCorFrame res;
      res.initResponse(*frame.data());
      res.setData(createJsonResponseError(3, QString("Incompatible version. (client=%1; server=%2)").arg(version).arg(TS3VIDEOSERVER_VERSION)));
      _connection->sendResponse(res);
      return;
    }
    // Authenticate.
    if (username.isEmpty()) {
      QCorFrame res;
      res.initResponse(*frame.data());
      res.setData(createJsonResponseError(4, QString("Authentication failed.")));
      _connection->sendResponse(res);
      return;
    }
    _authenticated = true;
    // Send response.
    QCorFrame res;
    res.initResponse(*frame.data());
    res.setData(createJsonResponse(QJsonObject()));
    _connection->sendResponse(res);
    return;
  }

  // The client needs to be authenticated before he can request any other actions.
  // Close connection, if the client tries anything else.
  if (!_authenticated) {
    QCorFrame res;
    res.initResponse(*frame.data());
    res.setData(createJsonResponseError(4, QString("Authentication failed.")));
    _connection->sendResponse(res);
    _connection->disconnectFromHost();
    return;
  }

  if (action == "joinchannel") {
    auto channelId = params["channelid"].toString();
    if (channelId.isEmpty()) {
      // TODO Send error: Invalid channel.
    }
    // TODO Send response.
  }
  else if (action == "leavechannel") {

  }

  QCorFrame res;
  res.initResponse(*frame.data());
  res.setData(createJsonResponseError(4, QString("Unknown action.")));
  _connection->sendResponse(res);
  return;
}