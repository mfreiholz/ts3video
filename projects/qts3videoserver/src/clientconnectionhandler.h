#ifndef CLIENTCONNECTIONHANDLER_H
#define CLIENTCONNECTIONHANDLER_H

#include <QObject>
#include <QAbstractSocket>
#include <QTime>
#include <QTimer>

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

  void sendMediaAuthSuccessNotify();

private slots:
  void onStateChanged(QAbstractSocket::SocketState state);
  void onNewIncomingRequest(QCorFrameRefPtr frame);
  void updateStatistics();

private:
  TS3VideoServer *_server;

  // Connection data.
  QCorConnection *_connection;
  QTimer _timeoutTimer;

  // Status information.
  ClientEntity *_clientEntity;
  bool _authenticated;

  // Statistics.
  quint64 _bytesRead; ///< Total number of bytes received from client.
  quint64 _bytesWritten; ///< Total number of bytes written to client.

  QTime _bytesReadTime;
  quint64 _bytesReadSince;
  QTime _bytesWrittenTime;
  quint64 _bytesWrittenSince;
};

#endif