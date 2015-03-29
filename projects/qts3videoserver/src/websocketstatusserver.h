#ifndef WEBSOCKETSTATUSSERVER_H
#define WEBSOCKETSTATUSSERVER_H

#include <QObject>
#include <QList>
#include <QTime>
#include <QHostAddress>

#include "ts3video.h"

class QWebSocketServer;
class QWebSocket;
class TS3VideoServer;

class WebSocketStatusServer : public QObject
{
  Q_OBJECT

public:
  class Options
  {
  public:
    // Address and port to listen for new connections.
    QHostAddress address = QHostAddress::LocalHost;
    quint16 port = IFVS_SERVER_WSSTATUS_PORT;
  };

public:
  WebSocketStatusServer(const WebSocketStatusServer::Options &opts, TS3VideoServer *server);
  virtual ~WebSocketStatusServer();
  bool init();

public slots:
  void broadcastAllInfo();

signals:
  void closed();

private slots:
  void onNewConnection();
  void onTextMessage(const QString &message);
  void onDisconnected();
  
private:
  QJsonValue getAllInfo() const;
  QJsonValue getAppInfo() const;
  QJsonValue getMemoryUsageInfo() const;
  QJsonValue getBandwidthInfo() const;
  QJsonValue getClientsInfo() const;
  QJsonValue getChannelsInfo() const;
  QJsonValue getWebSocketsInfo() const;

private:
  Options _opts;
  TS3VideoServer *_server;
  QWebSocketServer *_wsServer;
  QList<QWebSocket*> _sockets;
  
  // Restricts the  socket to send updates with a maxium rate.
  QTime _lastUpdateTime;
  unsigned int _maxUpdateRate;
};

#endif