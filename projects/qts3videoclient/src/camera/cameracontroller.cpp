#include "cameracontroller_p.h"
#include "humblelogging/api.h"
#include <QMessageBox>
#include "../cameraframegrabber.h"

HUMBLE_LOGGER(HL, "camera");

CameraController::CameraController(QObject *parent) :
  QObject(parent),
  d(new CameraControllerPrivate(this))
{
  d->camera = nullptr;
}

CameraController::~CameraController()
{
  d->camera->stop();
}

void CameraController::start(const QCameraInfo &info)
{
  auto grabber = new CameraFrameGrabber(this);
  auto camera = new QCamera(info, this);
  camera->setViewfinder(grabber);

  // Grabber events.
  QObject::connect(grabber, &CameraFrameGrabber::newQImage, this, &CameraController::newImage);

  // Camera events.
  QObject::connect(camera, &QCamera::stateChanged, [this, camera](QCamera::State state)
  {
    switch (state) {
    case QCamera::UnloadedState:
      break;
    case QCamera::LoadedState:
      break;
    case QCamera::ActiveState:
      break;
    }
  });

  QObject::connect(camera, static_cast<void(QCamera::*)(QCamera::Error)>(&QCamera::error), [this, camera](QCamera::Error error)
  {
    HL_ERROR(HL, QString("Camera error (error=%1; message=%2)").arg(error).arg(camera->errorString()).toStdString());
  });

  QObject::connect(camera, &QCamera::lockFailed, [this]()
  {
    HL_ERROR(HL, QString("Camera lock failed").toStdString());
  });

  camera->start();
  d->cameraInfo = info;
  d->camera = camera;
}

void CameraController::stop()
{
  if (!d->camera || d->cameraInfo.isNull())
    return;
  d->camera->stop();
}