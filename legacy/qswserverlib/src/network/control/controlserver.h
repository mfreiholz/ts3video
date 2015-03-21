#ifndef CONTROLSERVER_HEADER
#define CONTROLSERVER_HEADER

#include <QObject>
#include <QHostAddress>
#include "qswserverlib/api.h"

class SERVERCORE_API ControlServer :
  public QObject
{
  Q_OBJECT

public:
  class Options
  {
  public:
    Options() :
      address(QHostAddress::Any),
      port(6666),
      masterServerAddress(),
      masterServerPort(0)
    {}
    QHostAddress address; ///< The address on which to listen for new client connections.
    quint16 port; ///< The port on which to listen for new client connections.
    QHostAddress masterServerAddress; ///< The address of the remote MASTER server.
    quint16 masterServerPort; ///< The port of the remote MASTER server.
  };

  ControlServer(QObject *parent = 0);
  virtual ~ControlServer();
  bool startup(const Options &opts);
  bool shutdown();

private:
  class Private;
  Private *d;

  friend class ControlConnectionHandler;
};


#endif