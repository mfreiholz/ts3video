#ifndef CAMERACONTROLLERPRIVATE_H
#define CAMERACONTROLLERPRIVATE_H

#include "cameracontroller.h"

class CameraControllerPrivate : public QObject
{
  Q_OBJECT

public:
  CameraControllerPrivate(CameraController *o) : owner(o) {}

public slots:
  void onCameraStatusChanged(QCamera::Status status)
  {
    switch (status) {
    case QCamera::StartingStatus:
    break;
    case QCamera::StoppingStatus:
    break;
    case QCamera::ActiveStatus:
    break;
    }
  }

public:
  CameraController *owner;
  QCameraInfo cameraInfo;
  QCamera *camera;
};

#endif