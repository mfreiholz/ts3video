#ifndef MYCORSERVER_HEADER
#define MYCORSERVER_HEADER

#include <QObject>
#include <QList>
#include <QAbstractSocket>
class QCorServer;
class QCorConnection;
class QCorFrame;

class MyTestObject : public QObject
{
  Q_OBJECT

public:
  MyTestObject(QObject *parent);

public slots:
  // Server based slots.
  void onNewConnection(QCorConnection *conn);
  void onConnectionStateChanged(QAbstractSocket::SocketState state);
  void onNewFrame(QCorFrame *frame);

  // Client connection based methods.
  void clientConnect();

  // Common
  void printStatistics();

private:
  // Server
  QCorServer *_server;
  QList<QCorConnection*> _serverConnections;
};


#endif