#ifndef CONTROLMASTERCONNECTIONHANDLER_HEADER
#define CONTROLMASTERCONNECTIONHANDLER_HEADER

#include "QObject"
#include "QString"
class ControlServer;


class ControlToMasterConnectionHandler :
  public QObject
{
  Q_OBJECT
  class Private;
  Private *d;

public:
  struct StartupOptions {
    QString address;
    quint16 port;
  };

  ControlToMasterConnectionHandler(ControlServer *server, QObject *parent = 0);
  virtual ~ControlToMasterConnectionHandler();
  void startup(const StartupOptions &opts);
  void shutdown();

signals:
  void disconnected();
};


#endif