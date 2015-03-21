#ifndef MEDIASTREAMINGSOCKETHANDLERPRIVATE_HEADER
#define MEDIASTREAMINGSOCKETHANDLERPRIVATE_HEADER

#include "QObject"
#include "QAbstractSocket"
#include "mediastreamingsockethandler.h"
class QUdpSocket;


class MediaStreamingSocketHandler::Private :
  public QObject
{
  Q_OBJECT

public:
  Private(MediaStreamingSocketHandler *owner);
  ~Private();

public slots:
  void onStateChanged(QAbstractSocket::SocketState);
  void onReadyReadDatagrams();

public:
  bool processDatagram(QByteArray *data, const QHostAddress &sender, quint16 senderPort);
  bool processTokenDatagram(QDataStream &in, QByteArray *data, const QHostAddress &sender, quint16 sender_port);
  bool processVideoFrameDatagramV1(QDataStream &in, QByteArray *data, const QHostAddress &sender, quint16 sender_port);
  bool processVideoFrameRecoveryDatagramV1(QDataStream &in, QByteArray *data, const QHostAddress &senderAddress, quint16 senderPort);
  bool processAudioFrameDatagramV1(QDataStream &in, QByteArray *data, const QHostAddress &sender, quint16 sender_port);

public:
  MediaStreamingSocketHandler *_owner;
  QUdpSocket *_socket;
  quint64 _totalBytesRead;
  quint64 _totalBytesWritten;
  UdpDataReceiverMap _receiverMap;
};

#endif