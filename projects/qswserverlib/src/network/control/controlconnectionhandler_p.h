#ifndef CONTROLCONNECTIONHANDLERPRIVATE_HEADER
#define CONTROLCONNECTIONHANDLERPRIVATE_HEADER

#include "QObject"
#include "QAbstractSocket"
#include "controlconnectionhandler.h"
#include "qcorconnection.h"


class ControlConnectionHandler::Private :
  public QObject
{
  Q_OBJECT

public:
  Private(ControlConnectionHandler *owner);
  virtual ~Private();

public slots:
  void onSocketStateChanged(QAbstractSocket::SocketState);
  void onNewIncomingRequest(QCorFrameRefPtr frame);

public:
  ControlConnectionHandler *_owner;
  ControlServer *_server;
  QCorConnection *_conn;
};


#endif