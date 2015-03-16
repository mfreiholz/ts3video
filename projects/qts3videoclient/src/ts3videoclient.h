#ifndef TS3VIDEOCLIENT_H
#define TS3VIDEOCLIENT_H

#include <QObject>
#include <QScopedPointer>
#include <QAbstractSocket>

#include "qcorframe.h"
#include "qcorreply.h"
#include "qcorconnection.h"

#include "yuvframe.h"

class QHostAddress;
class ClientEntity;
class ChannelEntity;
class NetworkUsageEntity;

class TS3VideoClientPrivate;
class TS3VideoClient : public QObject
{
  Q_OBJECT
  friend class TS3VideoClientPrivate;
  QScopedPointer<TS3VideoClientPrivate> d;

public:
  TS3VideoClient(QObject *parent = nullptr);
  TS3VideoClient(const TS3VideoClient &other);
  ~TS3VideoClient();

  void setMediaEnabled(bool yesno); ///< TODO Remove me... DEV only.

  const QAbstractSocket* socket() const;
  const ClientEntity& clientEntity() const;
  bool isReadyForStreaming() const;

  /*!
    Connects to the remote TS3-VideoServer.
    Emits the "connected()" signal when the connection is established.
  */
  void connectToHost(const QHostAddress &address, qint16 port);

  /*!
    Authenticates with the server.
    This action has to be performed as first step, as soon as the connection is established.
    If the client doesn't authenticate within X seconds, the server will drop the connection.
    \see connected()
    \return QCorReply* Ownership goes over to caller who needs to delete it with "deleteLater()".
  */
  QCorReply* auth(const QString &name);

  /*!
    Joins a channel/room/conference.
    Requires an authenticated connection.
    \see auth()
    \return QCorReply* Ownership goes over to caller who needs to delete it with "deleteLater()".
  */
  QCorReply* joinChannel(int id = 0);

  /*!
    Sends a single frame to the server, which will then broadcast it to other clients.
    Internally encodes the image with VPX codec.
    \thread-safe
    \param image A single frame of the video.
  */
  void sendVideoFrame(const QImage &image);

signals:
  // Connection based signals.
  void connected();
  void disconnected();
  void error(QAbstractSocket::SocketError socketError);

  // Protocol based signals.
  void serverError(int errorCode, const QString &errorMessage);
  void clientJoinedChannel(const ClientEntity &client, const ChannelEntity &channel);
  void clientLeftChannel(const ClientEntity &client, const ChannelEntity &channel);
  void clientDisconnected(const ClientEntity &client);
  void newVideoFrame(YuvFrameRefPtr frame, int senderId);

  // Periodically updated information to display.
  void networkUsageUpdated(const NetworkUsageEntity &networkUsage);

private slots:
  void onStateChanged(QAbstractSocket::SocketState state);
  void onNewIncomingRequest(QCorFrameRefPtr frame);
};

#endif // TS3VIDEOCLIENT_H
