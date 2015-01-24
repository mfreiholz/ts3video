#ifndef TS3VIDEOSERVER_H
#define TS3VIDEOSERVER_H

#include <QObject>
#include <QList>

#include "qcorserver.h"

class ClientConnectionHandler;

class TS3VideoServer : public QObject
{
  Q_OBJECT

public:
  TS3VideoServer(QObject *parent);
  ~TS3VideoServer();

private:
  // Listens for new client connections.
  QCorServer _corServer;
  QList<ClientConnectionHandler*> _connections;
};

#endif