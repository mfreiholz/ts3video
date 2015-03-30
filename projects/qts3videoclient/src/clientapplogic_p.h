#ifndef CLIENTAPPLOGICPRIVATE_H
#define CLIENTAPPLOGICPRIVATE_H

#include "clientapplogic.h"

class ClientAppLogicPrivate : public QObject
{
  Q_OBJECT

public:
  ClientAppLogicPrivate(ClientAppLogic *o) :
    QObject(o),
    owner(o),
    opts(),
    view(nullptr),
    cameraWidget(nullptr),
    progressDialog(nullptr)
  {}

public:
  ClientAppLogic *owner;
  ClientAppLogic::Options opts;

  // Network connection.
  TS3VideoClient ts3vc;

  // Direct GUI elements.
  ViewBase *view; ///< Central view to display all video streams.
  ClientCameraVideoWidget *cameraWidget; ///< Local user's camera widget.

  // Dialogs.
  QProgressDialog *progressDialog; ///< Global progress dialog.
};

#endif