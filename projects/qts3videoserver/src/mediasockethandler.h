#ifndef MEDIASOCKETHANDLER_H
#define MEDIASOCKETHANDLER_H

#include <QObject>
#include <QUdpSocket>
#include <QVector>
#include <QTime>

#include "networkusageentity.h"

class MediaSenderEntity;
class MediaReceiverEntity;
class MediaRecipients;

/*!
 */
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

/*!
 */
class MediaReceiverEntity
{
public:
  int clientId;
  QHostAddress address;
  quint16 port;
};

/*!
 */
class MediaRecipients
{
public:
  QHash<QString, MediaSenderEntity> id2sender; ///< Maps a sender's adress-id to itself.
  QHash<int, MediaReceiverEntity> clientid2receiver; ///< Maps a receiver's client-id to itself.
};

/*! Receives video-streams and forwards them to all clients in the same channel.
 */
class MediaSocketHandler : public QObject
{
  Q_OBJECT

public:
  MediaSocketHandler(quint16 port, QObject *parent);
  ~MediaSocketHandler();
  bool init();
  void setRecipients(const MediaRecipients &rec);

signals:
  /*! Emits for every incoming authentication from a client.
   */
  void tokenAuthentication(const QString &token, const QHostAddress &address, quint16 port);

  /*! Emits whenever the transfer rates have been recalculated and updated.
      It gets calculated every X seconds by an internal timer.
   */
  void networkUsageUpdated(const NetworkUsageEntity &);

private slots:
  void onReadyRead();

private:
  quint16 _port;
  QUdpSocket _socket;
  MediaRecipients _recipients;

  // Network usage.
  NetworkUsageEntity _networkUsage;
  NetworkUsageEntityHelper _networkUsageHelper;
};

#endif