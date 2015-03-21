#ifndef MASTERSERVER_HEADER
#define MASTERSERVER_HEADER

#include "QObject"
#include "QString"
#include "servercore/api.h"


/*!
  Manages a single or a cluster of media streaming NODES.

  Everytime a users wants to start a audio-/video- conference, the master provides an available media streaming server
  to which the clients needs to connect.

  \reentrant
*/
class SERVERCORE_API MasterServer :
  public QObject
{
  Q_OBJECT
  friend class MasterMediaNodeConnectionHandler;
  friend class MasterClientNodeConnectionHandler;
  friend class MasterCtrlWsConnectionHandler;
  class Private;
  Private *d;

public:
  struct Options {
    quint16 mediaNodesPort;
    quint16 clientNodesPort;
    quint16 ctrlWsPort;

    Options() :
      mediaNodesPort(6660),
      clientNodesPort(6661),
      ctrlWsPort(6662)
    {}
  };

  struct ConnectionInfo {
    QString serverAddress;
    quint16 serverPort;
  };

  MasterServer(QObject *parent = 0);
  virtual ~MasterServer();

  bool startup(const Options &opts);
  bool shutdown();
  ConnectionInfo getBestMediaServer() const;
};


#endif