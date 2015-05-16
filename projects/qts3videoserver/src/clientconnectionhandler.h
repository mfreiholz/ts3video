#ifndef CLIENTCONNECTIONHANDLER_H
#define CLIENTCONNECTIONHANDLER_H

#include <QObject>
#include <QAbstractSocket>
#include <QTime>
#include <QTimer>

#include "qcorframe.h"

#include "networkusageentity.h"

class QCorConnection;
class VirtualServer;
class ClientEntity;

/*
  JSON Example Request
  --------------------
  {
    "action": "auth",
    "parameters": {
      "key1": "value1",
      "key2": [1, 2, 3]
    }
  }

  JSON Example Response
  ---------------------
  {
    "status": 0,  // Required. Status of the response (0 Always means OK!).
    "error": "",  // Optional. Custom error/status message.
    "data": 0     // Optional. Can be everything, defined by action.
  }
*/

class ClientConnectionHandler : public QObject
{
  Q_OBJECT

public:
  ClientConnectionHandler(VirtualServer *server, QCorConnection *connection, QObject *parent);
  ~ClientConnectionHandler();

  void sendMediaAuthSuccessNotify();

private slots:
  void onStateChanged(QAbstractSocket::SocketState state);
  void onNewIncomingRequest(QCorFrameRefPtr frame);

signals:
  void networkUsageUpdated(const NetworkUsageEntity &networkUsage);

public:
  VirtualServer *_server;

  // Connection data.
  QCorConnection *_connection;
  QTimer _connectionTimeoutTimer;

  // Status information.
  ClientEntity *_clientEntity;
  bool _authenticated;

  // Network usage.
  NetworkUsageEntity _networkUsage;
  NetworkUsageEntityHelper _networkUsageHelper;
};

#endif