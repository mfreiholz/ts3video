#ifndef CLIENTAPPLOGICPRIVATE_H
#define CLIENTAPPLOGICPRIVATE_H

#include "clientapplogic.h"
class ClientListModel;

class ClientAppLogicPrivate : public QObject
{
  Q_OBJECT

public:
  ClientAppLogicPrivate(ClientAppLogic *o) :
    QObject(o),
    owner(o),
    opts(),
    clientListModel(nullptr),
    view(nullptr),
    cameraWidget(nullptr),
    progressDialog(nullptr)
  {}
  ~ClientAppLogicPrivate()
  {}

public:
  ClientAppLogic *owner;
  ClientAppLogic::Options opts;
  NetworkClient ts3vc;
  ClientListModel *clientListModel; ///< Always holds the current state of connected clients in the same channel.

  // Direct GUI elements.
  ViewBase *view; ///< Central view to display all video streams.
  ClientCameraVideoWidget *cameraWidget; ///< Local user's camera widget.

  // Dialogs.
  QProgressDialog *progressDialog; ///< Global progress dialog.
};

#endif