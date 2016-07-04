#ifndef MEDIASOCKET_H
#define MEDIASOCKET_H

#include <QScopedPointer>
#include <QUdpSocket>
#include <QTimer>

#include "baselib/defines.h"

#include "videolib/yuvframe.h"
#include "videolib/pcmframe.h"

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

	void initVideoEncoder(int width, int height, int bitrate, int fps);
	void resetVideoEncoder();
	void sendVideoFrame(const QImage& image, ocs::clientid_t senderId);
	
	void resetVideoDecoderOfClient(ocs::clientid_t senderId);

#if defined(OCS_INCLUDE_AUDIO)
	void sendAudioFrame(const PcmFrameRefPtr& f, ocs::clientid_t senderId);
#endif

signals:
	/*! Emits with every new arrived and decoded video frame.
	*/
	void newVideoFrame(YuvFrameRefPtr frame, ocs::clientid_t senderId);

#if defined(OCS_INCLUDE_AUDIO)
	/*!	Emits with every new arrived and decoded audio frame.
	*/
	void newAudioFrame(PcmFrameRefPtr frame, ocs::clientid_t senderId);
#endif

	/*! Emits periodically with newest calculated network-usage information.
	*/
	void networkUsageUpdated(const NetworkUsageEntity& networkUsage);

protected:
	void sendKeepAliveDatagram();
	void sendAuthTokenDatagram(const QString& token);
	void sendVideoFrame(const QByteArray& frame, quint64 frameId, ocs::clientid_t senderId);
	void sendVideoFrameRecoveryDatagram(quint64 frameId, ocs::clientid_t fromSenderId);

#if defined(OCS_INCLUDE_AUDIO)
	void sendAudioFrame(const QByteArray& frame, quint64 frameId, ocs::clientid_t senderId);
#endif

	virtual void timerEvent(QTimerEvent* ev);

private slots:
	void onSocketStateChanged(QAbstractSocket::SocketState state);
	void onSocketError(QAbstractSocket::SocketError error);
	void onReadyRead();

	void onVideoFrameEncoded(QByteArray frame, ocs::clientid_t senderId);
	void onVideoFrameDecoded(YuvFrameRefPtr frame, ocs::clientid_t senderId);

private:
	QTimer _bandwidthTimer;
};

#endif