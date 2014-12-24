#ifndef MEDIASTREAMINGMASTERCONNECTIONHANDLERPRIVATE_HEADER
#define MEDIASTREAMINGMASTERCONNECTIONHANDLERPRIVATE_HEADER

#include "QAbstractSocket"
#include "mediastreamingmasterconnectionhandler.h"
#include "../tcpprotocol.h"
class QTcpSocket;


class MediaStreamingMasterConnectionHandler::Private :
  public QObject
{
  Q_OBJECT

public:
  Private(MediaStreamingMasterConnectionHandler *owner);
  virtual ~Private();

public slots:
  void onSocketStateChanged(QAbstractSocket::SocketState state);
  void onSocketReadyRead();

protected:
  void processRequest(TCP::Request &request);
  virtual void timerEvent(QTimerEvent *ev);

private slots:
  void sendUpdate();

public:
  MediaStreamingMasterConnectionHandler *_owner;
  MediaStreamingServer *_server;
  QTcpSocket *_socket;
  bool _shutdown;
  MediaStreamingMasterConnectionHandler::StartOptions _opts;
  int _updateTimerId;
  TCP::ProtocolHandler _protocol;
};


#endif