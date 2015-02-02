#ifndef MEDIASOCKETHANDLER_H
#define MEDIASOCKETHANDLER_H

#include <QObject>
#include <QUdpSocket>
#include <QVector>

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
  void setRecipients(const MediaRecipients &rec);

signals:
  void tokenAuthentication(const QString &token, const QHostAddress &address, quint16 port);

private slots:
  void onReadyRead();

private:
  QUdpSocket _socket;
  MediaRecipients _recipients;
};

#endif