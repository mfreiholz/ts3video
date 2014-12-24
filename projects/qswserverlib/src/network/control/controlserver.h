#ifndef CONTROLSERVER_HEADER
#define CONTROLSERVER_HEADER

#include "QObject"
#include "QString"
#include "servercore/api.h"


class SERVERCORE_API ControlServer :
  public QObject
{
  Q_OBJECT
  friend class ControlConnectionHandler;
  class Private;
  Private *d;

public:
  struct Options {
    quint16 clientPort;

    QString masterServerAddress;
    quint16 masterServerPort;

    Options() :
      clientPort(6666)
    {}
  };

  ControlServer(QObject *parent = 0);
  virtual ~ControlServer();
  bool startup(const Options &opts);
  bool shutdown();
};


#endif