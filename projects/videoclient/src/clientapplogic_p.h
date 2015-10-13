#ifndef CLIENTAPPLOGICPRIVATE_H
#define CLIENTAPPLOGICPRIVATE_H

#include <QSharedPointer>
#include <QCamera>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioFormat>

#include "clientapplogic.h"
#include "networkclient/networkclient.h"

class ViewBase;
class ClientCameraVideoWidget;

class ClientAppLogicPrivate : public QObject
{
	Q_OBJECT

public:
	ClientAppLogicPrivate(ClientAppLogic* o);
	~ClientAppLogicPrivate();
	QSharedPointer<QCamera> createCameraFromOptions() const;
	QSharedPointer<QAudioInput> createMicrophoneFromOptions() const;

	QAudioFormat createAudioFormat() const
	{
		QAudioFormat format;
		format.setSampleRate(8000);
		format.setChannelCount(1);
		format.setSampleSize(16);
		format.setCodec("audio/pcm");
		format.setByteOrder(QAudioFormat::LittleEndian);
		format.setSampleType(QAudioFormat::UnSignedInt);
		return format;
	}

public:
	ClientAppLogic* owner;
	ClientAppLogic::Options opts;
	QSharedPointer<NetworkClient> nc;
	QSharedPointer<QCamera> camera;
	QSharedPointer<QAudioInput> audioInput;

	// Direct GUI elements.
	ViewBase* view; ///< Central view to display all video streams.
	ClientCameraVideoWidget* cameraWidget; ///< Local user's camera widget.
};


#endif