#ifndef MYCORSERVER_HEADER
#define MYCORSERVER_HEADER

#include <QObject>
#include <QList>
#include <QAbstractSocket>
#include "qcorframe.h"
class QCorServer;
class QCorConnection;


class MyTestObject : public QObject
{
  Q_OBJECT

public:
  MyTestObject(QObject *parent);

public slots:
  // Server based slots.
  void startServer();
  void onNewConnection(QCorConnection *conn);
  void onConnectionStateChanged(QAbstractSocket::SocketState state);
  void onNewFrame(QCorFrameRefPtr frame);
  void printServerStatistics();

  // Client connection based methods.
  void clientConnect(int testRequestInterval = 500);

private:
  // Server
  QCorServer *_server;
  QList<QCorConnection*> _serverConnections;

  // Stats.
  quint64 _receivedFrames;
  quint64 _receivedFrameBytes;

  // Client.
  QList<QCorConnection*> _clientConnections;
};


#endif