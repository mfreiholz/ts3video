#ifndef CLIENTCONNECTIONHANDLER_H
#define CLIENTCONNECTIONHANDLER_H

#include <QObject>
#include <QAbstractSocket>

#include "qcorframe.h"

class QCorConnection;
class TS3VideoServer;
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
  ClientConnectionHandler(TS3VideoServer *server, QCorConnection *connection, QObject *parent);
  ~ClientConnectionHandler();

private slots:
  void onStateChanged(QAbstractSocket::SocketState state);
  void onNewIncomingRequest(QCorFrameRefPtr frame);

private:
  TS3VideoServer *_server;
  QCorConnection *_connection;
  ClientEntity *_clientEntity;

  // Status information.
  bool _authenticated;
};

#endif