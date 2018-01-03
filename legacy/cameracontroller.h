#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QObject>
#include <QScopedPointer>
#include <QCamera>
#include <QCameraInfo>

class CameraControllerPrivate;
class CameraController : public QObject
{
  Q_OBJECT
  QScopedPointer<CameraControllerPrivate> d;

public:
  CameraController(QObject *parent = nullptr);
  virtual ~CameraController();

public slots:
  void start(const QCameraInfo &info);
  void stop();

signals:
  void newImage(const QImage &image);
};

#endif