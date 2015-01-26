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
  static QString createID(const QHostAddress &address, quint16 port)
  {
    return address.toString() + QString::number(port);
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
  QHash<QString, MediaSenderEntity> id2sender;
};

class MediaSocketHandler : public QObject
{
  Q_OBJECT

public:
  MediaSocketHandler(QObject *parent);
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