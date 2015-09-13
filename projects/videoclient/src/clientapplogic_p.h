#ifndef CLIENTAPPLOGICPRIVATE_H
#define CLIENTAPPLOGICPRIVATE_H

#include <QSharedPointer>
#include <QCamera>

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

public:
	ClientAppLogic* owner;
	ClientAppLogic::Options opts;
	QSharedPointer<NetworkClient> nc;
	QSharedPointer<QCamera> camera;

	// Direct GUI elements.
	ViewBase* view; ///< Central view to display all video streams.
	ClientCameraVideoWidget* cameraWidget; ///< Local user's camera widget.
};


#endif