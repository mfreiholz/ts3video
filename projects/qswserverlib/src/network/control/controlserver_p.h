#ifndef CONTROLSERVERPRIVATE_HEADER
#define CONTROLSERVERPRIVATE_HEADER

#include <QObject>
#include <QList>
#include <QHash>
#include <QTcpServer>
#include "controlserver.h"
#include "controltomasterconnectionhandler.h"
class ControlConnectionHandler;
class ControlModule;


class ControlServer::Private :
  public QObject
{
  Q_OBJECT

public:
  Private(ControlServer *owner);
  virtual ~Private();

public slots:
  void onNewClientConnection();
  void onClientDisconnected();
  void testBroadcast();

public:
  ControlServer *_owner;
  ControlServer::Options _opts;

  // Client connections.
  QTcpServer _clientListener;
  QList<ControlConnectionHandler*> _clientConnections;

  // Master connection.
  ControlToMasterConnectionHandler _masterConnection;

  // Modules.
  QList<ControlModule*> _modules;
  QHash<QString, ControlModule*> _id2module;
};


#endif