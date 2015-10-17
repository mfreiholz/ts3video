#ifndef MEDIASOCKET_H
#define MEDIASOCKET_H

#include <QScopedPointer>
#include <QUdpSocket>
#include "yuvframe.h"
#include "pcmframe.h"
class NetworkUsageEntity;

class MediaSocketPrivate;
class MediaSocket : public QUdpSocket
{
	Q_OBJECT
	friend class MediaSocketPrivate;
	QScopedPointer<MediaSocketPrivate> d;

public:
	MediaSocket(const QString& token, QObject* parent);
	virtual ~MediaSocket();

	bool isAuthenticated() const;
	void setAuthenticated(bool yesno);

	void sendVideoFrame(const QImage& image, int senderId);
	void sendAudioFrame(const PcmFrameRefPtr& f, int senderId);

signals:
	/*! Emits with every new arrived and decoded video frame.
	*/
	void newVideoFrame(YuvFrameRefPtr frame, int senderId);

	/*!	Emits with every new arrived and decoded audio frame.
	*/
	void newAudioFrame(PcmFrameRefPtr frame, int senderId);

	/*! Emits periodically with newest calculated network-usage information.
	*/
	void networkUsageUpdated(const NetworkUsageEntity& networkUsage);

protected:
	void sendKeepAliveDatagram();
	void sendAuthTokenDatagram(const QString& token);
	void sendVideoFrame(const QByteArray& frame, quint64 frameId, quint32 senderId);
	void sendVideoFrameRecoveryDatagram(quint64 frameId, quint32 fromSenderId);
	void sendAudioFrame(const QByteArray& frame, quint64 frameId, quint32 senderId);
	virtual void timerEvent(QTimerEvent* ev);

private slots:
	void onSocketStateChanged(QAbstractSocket::SocketState state);
	void onSocketError(QAbstractSocket::SocketError error);
	void onReadyRead();
};

#endif