#ifndef CLIENTCONNECTIONHANDLER_H
#define CLIENTCONNECTIONHANDLER_H

#include <QObject>
class QCorConnection;
class TS3VideoServer;

class ClientConnectionHandler : public QObject
{
  Q_OBJECT

public:
  ClientConnectionHandler(TS3VideoServer *server, QCorConnection *connection, QObject *parent);
  ~ClientConnectionHandler();

private:
  TS3VideoServer *_server;
  QCorConnection *_connection;
};

#endif