#ifndef CLIENTAPPLOGICPRIVATE_H
#define CLIENTAPPLOGICPRIVATE_H

#include "clientapplogic.h"

class ClientAppLogicPrivate : public QObject
{
	Q_OBJECT

public:
	ClientAppLogicPrivate(ClientAppLogic* o) :
		QObject(o),
		owner(o),
		opts(),
		view(nullptr),
		cameraWidget(nullptr),
		progressDialog(nullptr)
	{}
	~ClientAppLogicPrivate()
	{}
	QSharedPointer<QCamera> createCameraFromOptions() const;

public:
	ClientAppLogic* owner;
	ClientAppLogic::Options opts;
	QSharedPointer<NetworkClient> nc;
	QSharedPointer<QCamera> camera;

	// Direct GUI elements.
	ViewBase* view; ///< Central view to display all video streams.
	ClientCameraVideoWidget* cameraWidget; ///< Local user's camera widget.

	// Dialogs.
	QProgressDialog* progressDialog; ///< Global progress dialog.
};

/*
	AutoNetworkInitLogicProtocol initializes the app's network connection
	to the server and automatically turns on some stuff (e.g. camera),
	defined by the ClientAppLogic::Options.
*/
class AutoNetworkInitLogicProtocol : public QObject
{
	Q_OBJECT

public:
	AutoNetworkInitLogicProtocol(ClientAppLogic *l);

public:
	ClientAppLogic* _logic;
};

#endif