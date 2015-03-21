#ifndef MASTERCTRLWSCONNECTIONHANDLERPRIVATE_HEADER
#define MASTERCTRLWSCONNECTIONHANDLERPRIVATE_HEADER

#include "QObject"
#include "QAbstractSocket"
#include "masterctrlwsconnectionhandler.h"
class MasterServer;

class MasterCtrlWsConnectionHandler::Private :
  public QObject
{
  Q_OBJECT

public:
  Private(MasterCtrlWsConnectionHandler *owner);
  virtual ~Private();

public slots:
  void onSocketStateChanged(QAbstractSocket::SocketState state);
  void onTextMessageReceived(const QString &message);
  void sendStatus();

public:
  MasterCtrlWsConnectionHandler *_owner;
  MasterServer *_master;
  QWebSocket *_socket;
};

#endif