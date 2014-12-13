#ifndef MYCORSERVER_HEADER
#define MYCORSERVER_HEADER

#include <QObject>
class QCorServer;
class QCorConnection;

class MyTestObject : public QObject
{
  Q_OBJECT

public:
  MyTestObject(QObject *parent);

public slots:
  void onNewConnection(QCorConnection *conn);

  // Client connection based methods.
  void clientConnect();
  void clientSendFrame();

private:
  // Server
  QCorServer *_server;

  // Client
  QCorConnection *_clientConn;
};


#endif