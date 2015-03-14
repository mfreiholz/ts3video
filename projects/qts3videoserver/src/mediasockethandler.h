#ifndef MEDIASOCKETHANDLER_H
#define MEDIASOCKETHANDLER_H

#include <QObject>
#include <QUdpSocket>
#include <QVector>
#include <QTime>

class MediaSenderEntity;
class MediaReceiverEntity;
class MediaRecipients;

class MediaSenderEntity
{
public:
  static inline QString createID(const QHostAddress &address, quint16 port)
  {
    return address.toString() + QString(":") + QString::number(port);
  }

  QString id;
  int clientId;
  QHostAddress address;
  quint16 port;
  QVector<MediaReceiverEntity> receivers;
};

class MediaReceiverEntity
{
public:
  int clientId;
  QHostAddress address;
  quint16 port;
};

class MediaRecipients
{
public:
  QHash<QString, MediaSenderEntity> id2sender; ///< Maps a sender's adress-id to itself.
  QHash<int, MediaReceiverEntity> clientid2receiver; ///< Maps a receiver's client-id to itself.
};

class MediaSocketHandler : public QObject
{
  Q_OBJECT

public:
  MediaSocketHandler(quint16 port, QObject *parent);
  ~MediaSocketHandler();
  bool init();
  void setRecipients(const MediaRecipients &rec);

signals:
  /**
   * Emits for every incoming authentication from a client.
   */
  void tokenAuthentication(const QString &token, const QHostAddress &address, quint16 port);

  /**
   * Emits whenever the transfer rates have been recalculated and updated.
   * It gets calculated every X seconds by an internal timer.
   */
  void transferRatesChanged(double bytesReadRate, double bytesWriteRate);

private slots:
  void onReadyRead();

private:
  quint16 _port;
  QUdpSocket _socket;
  MediaRecipients _recipients;

  // Bandwidth stats.
  quint64 _bytesRead; ///< Number of bytes read.
  quint64 _bytesWritten; ///< Number of bytes written.

  double _transferRateRead;
  double _transferRateWrite;
  
  QTime _bandwidthCalcTime;
  quint64 _bandwidthBytesReadTemp;
  quint64 _bandwidthBytesWrittenTemp;
};

#endif