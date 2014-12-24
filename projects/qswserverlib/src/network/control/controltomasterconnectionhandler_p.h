#ifndef CONTROLMASTERCONNECTIONHANDLERPRIVATE_HEADER
#define CONTROLMASTERCONNECTIONHANDLERPRIVATE_HEADER

#include <QObject>
#include <QAbstractSocket>
#include "controltomasterconnectionhandler.h"
#include "qcorconnection.h"


class ControlToMasterConnectionHandler::Private :
  public QObject
{
  Q_OBJECT

public:
  Private(ControlToMasterConnectionHandler *owner);
  virtual ~Private();

public slots:
  void onSocketStateChanged(QAbstractSocket::SocketState);
  void onNewIncomingRequest(QCorFrameRefPtr frame);

public:
  ControlToMasterConnectionHandler *_owner;
  ControlToMasterConnectionHandler::StartupOptions _opts;
  ControlServer *_server;
  QCorConnection *_conn;
  bool _shutdown;
};


#endif